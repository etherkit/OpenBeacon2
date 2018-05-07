#include <Scheduler.h>
#include <JTEncode.h>
#include <ArduinoJson.h>
#include <ArduinoSTL.h>
#include <Menu.h>
#include <RTCZero.h>
#include <Morse.h>
#include <morsechar.h>
#include <U8g2lib.h>
#include <si5351.h>
#include <Wire.h>

#include <cstdlib>
#include <map>
#include <string>
#include <time.h>

#include "bands.h"
#include "modes.h"

// Enumerations
enum class DisplayMode {Main, Menu, Setting};
enum class SettingType {Uint, Int, Str, Time};
enum class TxState {Idle, MFSK, Preamble};

// Hardware constexprs
constexpr uint8_t BTN_DSP_1 = 0;
constexpr uint8_t BTN_DSP_2 = 1;
constexpr uint8_t BTN_UP = 2;
constexpr uint8_t BTN_DOWN = 3;
constexpr uint8_t BTN_LEFT = 4;
constexpr uint8_t BTN_RIGHT = 5;
constexpr uint8_t BTN_BACK = 6;
constexpr uint8_t CLK_INPUT = 9;
constexpr uint8_t TX_KEY = 13;
constexpr uint8_t SYNC_LED = 26;

constexpr uint8_t ADC_BAND_ID = 0;

constexpr uint8_t MCP4725A1_BUS_BASE_ADDR = 0x62;
constexpr uint16_t MCP4725A1_VREF = 5000UL;
constexpr uint16_t ANALOG_REF = 3600UL; // TODO
constexpr uint16_t PA_BIAS_FULL = 1850UL;

//constexpr uint16_t TIMER_PRESCALER_DIV = 1024;
//constexpr uint32_t TIMER_BASE_CLOCK = 32768;
constexpr uint32_t TIMER_BASE_CLOCK = 4000000;
constexpr uint16_t TIMER_PRESCALER_DIV = 1;
constexpr uint16_t TIMER_FREQUENCY = 1000;

constexpr uint8_t TIME_REQUEST = '\a';
constexpr uint8_t TIME_HEADER = 'T';
constexpr uint32_t TIME_EXPIRE = 86400;
constexpr uint32_t TIME_SYNC_INTERVAL = 43200;
constexpr uint32_t TIME_SYNC_RETRY_RATE = 60;

constexpr static unsigned char lock_bits[] = {
   0x18, 0x24, 0x24, 0x7e, 0x81, 0x81, 0x81, 0x7e };

// Character 0 of the value field denotes setting type:
// S == string
// U == uint
// I == int
// T == time
constexpr char* config_table[][2] =
{
  {"PA Bias", "U1800"},
  {"Callsign", "SNT7S"},
  {"Grid", "SCN85"},
  {"Power", "I25"},
  {"TX Intv", "U6"},
  {"WPM", "U22"}
};

// Defaults
constexpr uint32_t DEFAULT_FREQUENCY = 0UL;
constexpr uint32_t DEFAULT_LOWER_FREQ_LIMIT = 0UL;
constexpr uint32_t DEFAULT_UPPER_FREQ_LIMIT = 0UL;
constexpr uint8_t DEFAULT_BAND_INDEX = 0;
constexpr MetaMode DEFAULT_METAMODE = MetaMode::MFSK;
constexpr Mode DEFAULT_MODE = Mode::WSPR;
constexpr bool DEFAULT_TX_LOCK = false;
constexpr DisplayMode DEFAULT_DISPLAY_MODE = DisplayMode::Main;
constexpr uint16_t DEFAULT_TX_DELAY = 1; // in minutes
constexpr TxState DEFAULT_STATE = TxState::Idle;
constexpr uint8_t DEFAULT_TX_INTERVAL = 6;

struct tm DEFAULT_TIME = {0, 1, 18, 19, 3, 2018, 1, 0, 1};

// Limits
constexpr uint8_t SETTING_FONT_WIDTH = 6;


// Class constructors
//U8G2_SSD1306_128X32_UNIVISION_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Si5351 si5351;
Morse morse(TX_KEY, 15);
RTCZero rtc;
Menu menu;
JTEncode jtencode;

// ISR global variables
volatile uint32_t base_frequency = DEFAULT_FREQUENCY; // 30m WSPR
volatile uint32_t frequency = base_frequency;
volatile uint32_t last_reported_pos;   // change management
volatile uint32_t lower_freq_limit = DEFAULT_LOWER_FREQ_LIMIT;
volatile uint32_t upper_freq_limit = DEFAULT_UPPER_FREQ_LIMIT;
volatile uint8_t band_index = DEFAULT_BAND_INDEX;
volatile MetaMode meta_mode = DEFAULT_METAMODE;
volatile Mode mode = DEFAULT_MODE;
volatile bool tx_lock = DEFAULT_TX_LOCK;
volatile DisplayMode display_mode = DEFAULT_DISPLAY_MODE;
volatile uint16_t tx_delay = DEFAULT_TX_DELAY;
volatile TxState cur_state = DEFAULT_STATE;
volatile TxState prev_state = DEFAULT_STATE;
volatile uint32_t cur_timer, next_event;
volatile uint8_t clk_temp = 0;
volatile uint8_t cur_symbol = 0;
volatile uint16_t cur_tone_spacing = 0;
volatile bool change_freq = false;

// Global variables
uint8_t tune_step = 0;
uint32_t band_id = 0;
std::map<std::string, std::string> cfg;
std::string cur_setting = "";
uint64_t cur_setting_uint = 0;
//uint64_t temp_setting_uint = 0;
int64_t cur_setting_int = 0;
//int64_t temp_setting_int = 0;
std::string cur_setting_str = "";
//std::string temp_setting_str = "";
SettingType cur_setting_type = SettingType::Uint;
uint8_t cur_setting_selected = 0;
std::string settings_str_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-+/.";
uint8_t cur_setting_char = 0;
uint32_t next_tx;
uint32_t time_sync_expire, next_time_sync;
uint32_t initial_time_sync = 0;
//bool time_sync_request = false;
uint8_t tx_interval = DEFAULT_TX_INTERVAL;
uint8_t tx_buffer[255];
uint8_t cur_symbol_count;
uint16_t cur_symbol_time;

// Timer code derived from:
// https://github.com/nebs/arduino-zero-timer-demo

void setTimerFrequency(uint16_t frequencyHz)
{
  //int compareValue = (VARIANT_MCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  //uint16_t compareValue = (TIMER_BASE_CLOCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  uint32_t compareValue = ((TIMER_BASE_CLOCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1);
  //TcCount16* TC = (TcCount16*) TC5;
  Tcc* TC = (Tcc*) TCC0;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  //while (TC->STATUS.bit.SYNCBUSY == 1);
  TC->CC[0].reg = compareValue;
  while (TC->SYNCBUSY.bit.CC0 == 1);
  //while (TC->STATUS.bit.SYNCBUSY == 1);
}

/*
This is a slightly modified version of the timer setup found at:
https://github.com/maxbader/arduino_tools
 */
void startTimer(int frequencyHz)
{
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC0_TCC1));
  //GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
  //GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
  //REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TCC2_TC3));
  while (GCLK->STATUS.bit.SYNCBUSY == 1);

  Tcc* TC = (Tcc*) TCC0;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;

  // Use the 16-bit timer
  //TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  //while (TC->STATUS.bit.SYNCBUSY == 1);

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->WAVE.reg |= TCC_WAVE_WAVEGEN_MFRQ;
  //while (TC->STATUS.bit.SYNCBUSY == 1);
  while (TC->SYNCBUSY.bit.WAVE == 1);

  // Set prescaler to 4 to get 1 MHz clock
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1;
  //while (TC->STATUS.bit.SYNCBUSY == 1);

  // Have counter wrap around on prescaled clock
//  TC->CTRLA.reg |= TC_CTRLA_PRESCSYNC(TC_CTRLA_PRESCSYNC_PRESC);
//  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequency(frequencyHz);

  NVIC_DisableIRQ(TCC0_IRQn);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

//  NVIC_ClearPendingIRQ(TCC0_IRQn);
//  NVIC_SetPriority(TCC0_IRQn, 0);
  NVIC_EnableIRQ(TCC0_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->SYNCBUSY.bit.ENABLE == 1);
  
//  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
//  //GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
//  //GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
//  //REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TCC2_TC3));
//  while (GCLK->STATUS.bit.SYNCBUSY == 1);
//
//  TcCount16* TC = (TcCount16*) TC5;
//
//  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
//
//  // Use the 16-bit timer
//  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
//  while (TC->STATUS.bit.SYNCBUSY == 1);
//
//  // Use match mode so that the timer counter resets when the count matches the compare register
//  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
//  while (TC->STATUS.bit.SYNCBUSY == 1);
//
//  // Set prescaler to 4 to get 1 MHz clock
//  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV4;
//  while (TC->STATUS.bit.SYNCBUSY == 1);
//
//  // Have counter wrap around on prescaled clock
////  TC->CTRLA.reg |= TC_CTRLA_PRESCSYNC(TC_CTRLA_PRESCSYNC_PRESC);
////  while (TC->STATUS.bit.SYNCBUSY == 1);
//
//  setTimerFrequency(frequencyHz);
//
//  NVIC_DisableIRQ(TC5_IRQn);
//
//  // Enable the compare interrupt
//  TC->INTENSET.reg = 0;
//  TC->INTENSET.bit.MC0 = 1;
//
//  NVIC_ClearPendingIRQ(TC5_IRQn);
//  NVIC_SetPriority(TC5_IRQn, 0);
//  NVIC_EnableIRQ(TC5_IRQn);
//
//  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
//  while (TC->STATUS.bit.SYNCBUSY == 1);
}

constexpr unsigned long power_10(unsigned long exponent)
{
  // bounds checking pls
  return (exponent == 0) ? 1 : 10 * power_10(exponent - 1);
}

// Voltage specified in millivolts
void setPABias(uint16_t voltage)
{
  uint32_t reg;
  uint8_t reg1, reg2;
  
  // Bounds checking
  if(voltage > MCP4725A1_VREF)
  {
    voltage = MCP4725A1_VREF;
  }
  
  // Convert millivolts to the correct register value
  reg = ((uint32_t)voltage * 4096UL) / MCP4725A1_VREF;
  reg1 = (uint8_t)((reg >> 8) & 0xFF);
  reg2 = (uint8_t)(reg & 0xFF); 
  
  // Write the register to the MCP4725A1
  noInterrupts();
  Wire.beginTransmission(MCP4725A1_BUS_BASE_ADDR);
  Wire.write(reg1);
  Wire.write(reg2);
  Wire.endTransmission();
  interrupts();
}

void initMenu()
{
  menu.addChild("Mode");
    menu.selectChild(0);
    for(auto i : mode_table)
    {
      uint8_t index = static_cast<uint8_t>(i.index);
      menu.addChild(i.mode_name, selectMode, index);
    }
    menu.selectParent();
  menu.addChild("Buffers");
    menu.selectChild(1);
    menu.addChild("1");
    menu.selectParent();
  menu.addChild("Settings");
    menu.selectChild(2);
//    for(uint8_t i = 0; i < cfg.size(); ++i)
//    {
//      
//    }
    for(auto& c : cfg)
    {
      const char* key = c.first.c_str();
      menu.addChild(key, setConfig, key);
    }
//    menu.addChild("PA Bias", setConfig, "PA Bias");
//    menu.addChild("Callsign", setConfig, "Callsign");
    menu.selectParent();
  //menu.addChild("Menu");  
  menu.selectRoot();
}

// ===== ISRs =====
void TCC0_Handler()
{
  //TcCount16* TC = (TcCount16*) TC5;
  Tcc* TC = (Tcc*) TCC0;

  if (TC->INTFLAG.bit.MC0 == 1)
  {
    ++cur_timer;
    //morse.update();
    TC->INTFLAG.bit.MC0 = 1;
  }
}

//void handleClkInput()
//{
//  ++clk_temp;
//  if(clk_temp > 10)
//  {
//    ++cur_timer;
//    clk_temp = 0;
//  }
//}

// ===== Callbacks =====
void selectMode(uint8_t sel)
{
  switch(static_cast<Mode>(sel))
  {
  case Mode::DFCW3:
    mode = Mode::DFCW3;
    break;
  case Mode::DFCW6:
    mode = Mode::DFCW6;
    break;
  case Mode::DFCW10:
    mode = Mode::DFCW10;
    break;
  case Mode::DFCW120:
    mode = Mode::DFCW120;
    break;
  case Mode::QRSS3:
    mode = Mode::QRSS3;
    break;
  case Mode::QRSS6:
    mode = Mode::QRSS6;
    break;
  case Mode::QRSS10:
    mode = Mode::QRSS10;
    break;
  case Mode::QRSS120:
    mode = Mode::QRSS120;
    break;
  case Mode::CW:
    mode = Mode::CW;
    break;
  case Mode::HELL:
    mode = Mode::HELL;
    break;
  case Mode::WSPR:
    mode = Mode::WSPR;
    //memset(tx_buffer, 0, 255);
    //jtencode.wspr_encode(cfg["Callsign"].c_str(), cfg["Grid"].c_str(), atoi(cfg["Power"].c_str()), tx_buffer);
    cur_tone_spacing = mode_table[static_cast<uint8_t>(mode)].tone_spacing;
    cur_symbol_count = mode_table[static_cast<uint8_t>(mode)].symbol_count;
    cur_symbol_time = mode_table[static_cast<uint8_t>(mode)].symbol_time;
    break;
  case Mode::JT65:
    mode = Mode::JT65;
    break;
  case Mode::JT9:
    mode = Mode::JT9;
    break;
  case Mode::JT4:
    mode = Mode::JT4;
    break;
  }
}

void setConfig(const char * key)
{
  //cfg[key] = "1850";
  display_mode = DisplayMode::Setting;
  cur_setting = key;
  std::string val = cfg[key];
  switch(val[0])
  {
  case 'U':
    cur_setting_uint = atoll(cfg[key].substr(1).c_str());
    cur_setting_type = SettingType::Uint;
    break;
  case 'I':
    cur_setting_int = atoll(cfg[key].substr(1).c_str());
    cur_setting_type = SettingType::Int;
    break;
  case 'S':
    cur_setting_str = cfg[key].substr(1).c_str();
    cur_setting_type = SettingType::Str;
    break;
  }
}

// ===== Setup =====
//void setup()
//{
//  // Serial port init
//  SerialUSB.begin(57600);
//  while(!SerialUSB);
//
//  // Set up scheduler
//  Scheduler.startLoop(processTxTrigger);
//  Scheduler.startLoop(drawOLED);
//  Scheduler.startLoop(pollButtons);
//  Scheduler.startLoop(selectBand);
//  Scheduler.startLoop(processTimeSync);
//
//  // Load config map
//  for(auto const& c : config_table)
//  {
//    cfg[c[0]] = c[1];
//  }
//  
//  // Start u8g2
//  u8g2.begin();
//
//  // I/O init
//  pinMode(BTN_DSP_1, INPUT_PULLUP);
//  pinMode(BTN_DSP_2, INPUT_PULLUP);
//  pinMode(BTN_UP, INPUT_PULLUP);
//  pinMode(BTN_DOWN, INPUT_PULLUP);
//  pinMode(BTN_LEFT, INPUT_PULLUP);
//  pinMode(BTN_RIGHT, INPUT_PULLUP);
//  pinMode(BTN_BACK, INPUT_PULLUP);
//  pinMode(TX_KEY, OUTPUT);
//  pinMode(SYNC_LED, OUTPUT);
//
//  //attachInterrupt(digitalPinToInterrupt(BTN_BACK), handleMenuBack, FALLING);
//
//
//  // ADC resolution
//  analogReadResolution(12);
//  analogReference(AR_DEFAULT);
//
//  // Si5351
//  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);
//  si5351.set_freq(base_frequency * SI5351_FREQ_MULT, SI5351_CLK0);
//  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
//  Wire.setClock(400000UL);
//
//  // Set PA bias
//  setPABias(PA_BIAS_FULL);
//
//  // RTC setup
//  // Can't use the RTC alarm interrupt, far too much trigger time variance
//  rtc.begin();
//  rtc.setTime(DEFAULT_TIME.tm_hour, DEFAULT_TIME.tm_min, DEFAULT_TIME.tm_sec);
//  rtc.setDate(DEFAULT_TIME.tm_mday, DEFAULT_TIME.tm_mon, DEFAULT_TIME.tm_year);
//  time_sync_expire = rtc.getEpoch();
//  next_time_sync = rtc.getEpoch();
//
//  // 
//  setNextTx(0);
//
//  // Init menu
//  initMenu();
//
//  // Start Timer
//  startTimer(TIMER_FREQUENCY); // 1 ms ISR
//
//  cur_tone_spacing = mode_table[static_cast<uint8_t>(mode)].tone_spacing;
//
//  // Clear TX buffer
//  memset(tx_buffer, 0, 255);
//  jtencode.wspr_encode(cfg["Callsign"].substr(1).c_str(), cfg["Grid"].substr(1).c_str(), 
//    atoi(cfg["Power"].substr(1).c_str()), tx_buffer);
//  //jtencode.wspr_encode("NT7S", "CN85", 23, tx_buffer);
//  setTxState(TxState::Idle);
//  frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
//  change_freq = true;
//
//  //morse.send("DE NT7S");
//}

void drawOLED()
{
  static char temp_str[8];
  static char freq_str[10];
  static char freq_str_1[4];
  static char freq_str_2[4];
  static char freq_str_3[4];
  static char temp_chr[2];
  uint32_t freq;
  uint8_t zero_pad = 0;
  uint8_t underline;
  char menu_1[16];
  char menu_2[16];
  uint8_t menu_1_x, menu_2_x;

  // u8g2 draw loop
  // --------------
  u8g2.clearBuffer();          // clear the internal memory
//  u8g2.firstPage();
//do{
  if(display_mode != DisplayMode::Setting)
  {
    u8g2.setFont(u8g2_font_logisoso16_tn);
    //u8g2.setFont(u8g2_font_6x10_mr);
    u8g2.setDrawColor(1);
    //u8g2.setFont(u8g2_font_inb19_mn);
  
    // MHz
    yield();
    freq = base_frequency;
    //sprintf(freq_str, "%9lu", freq);
//    sprintf(freq_str_1, "%3lu", freq / 1000000UL);
//    freq %= 1000000UL;
//    sprintf(freq_str_2, "%3lu", freq / 1000UL);
//    freq %= 1000UL;
//    sprintf(freq_str_3, "%3lu", freq);

    if(base_frequency / 1000000UL > 0)
    {
      sprintf(temp_str, "%3lu", freq / 1000000UL);
//      for(uint8_t i = 0; i < 3; ++i)
//      {
//        sprintf(temp_chr, "%c", temp_str[i]);
//        u8g2.drawGlyph(i * 9, 17, static_cast<uint16_t>(freq_str[i]));
//      }
//      u8g2.drawStr(0, 17, temp_str);
      zero_pad = 1;
    }
    else
    {
      sprintf(temp_str, "   ");
      //u8g2.drawStr(0, 17, temp_str);
    }
    // We do this because the desired font isn't quite monospaced :-/
    for(uint8_t i = 0; i < 3; ++i)
    {
      //memmove(temp_chr, temp_str + i, 1);
      sprintf(temp_chr, "%c", temp_str[i]);
      u8g2.drawStr(i * 9, 17, temp_chr);
    }
    freq %= 1000000UL;
    
    // kHz
    yield();
    if(zero_pad == 1)
    {
      sprintf(temp_str, "%03lu", freq / 1000UL);
    }
    else if(freq / 1000UL > 0)
    {
      sprintf(temp_str, "%3lu", freq / 1000UL);
      zero_pad = 1;
    }
    else
    {
      sprintf(temp_str, "   ");
    }
    //u8g2.drawStr(38, 20, temp_str);
    //u8g2.drawStr(29, 17, temp_str);
    for(uint8_t i = 0; i < 3; ++i)
    {
      //memmove(temp_chr, temp_str + i, 1);
      sprintf(temp_chr, "%c", temp_str[i]);
      u8g2.drawStr(i * 9 + 29, 17, temp_chr);
    }
    freq %= 1000UL;
    
    // Hz
    yield();
    if(zero_pad == 1)
    {
      sprintf(temp_str, "%03lu", freq);
    }
    else
    {
      sprintf(temp_str, "%3lu", freq);
    }
    //u8g2.drawStr(78, 20, temp_str);
    for(uint8_t i = 0; i < 3; ++i)
    {
      //memmove(temp_chr, temp_str + i, 1);
      sprintf(temp_chr, "%c", temp_str[i]);
      u8g2.drawStr(i * 9 + 58, 17, temp_chr);
    }
    
    // Indicate step size
    yield();
    switch(tune_step)
    {
    case 5:
      underline = 29;
      break;
    case 4:
      underline = 38;
      break;
    case 3:
      underline = 47;
      break;
    case 2:
      underline = 59;
      break;
    case 1:
      underline = 68;
      break;
    case 0:
      underline = 77;
      break;
    }

    yield();
  
    // Draw step size indicator
    u8g2.drawBox(underline, 18, 9, 2);
  
    // Draw Hz
    //u8g2.setFont(u8g2_font_5x8_mr);
  //  u8g2.drawStr(120, 20, "Hz");
  
    // Draw ADC
    //u8g2.setFont(u8g2_font_5x8_mr);
  //  sprintf(temp_str, "%u %u", band_id, band_index);
  //  u8g2.drawStr(90, 10, temp_str);
  
    // Draw clock
    yield();
    //u8g2.setFont(u8g2_font_6x10_mr);
    u8g2.setFont(u8g2_font_5x7_tn);
    sprintf(temp_str, "%02u:%02u:%02u", rtc.getHours(),
      rtc.getMinutes(), rtc.getSeconds());
    u8g2.drawStr(88, 6, temp_str);
  
    // Draw mode
    yield();
    u8g2.setFont(u8g2_font_6x10_mr);
    sprintf(temp_str, "%s", mode_table[static_cast<uint8_t>(mode)].mode_name);
    u8g2.drawStr(87, 15, temp_str);
    //yield();
  
    // Draw index
    //sprintf(temp_str, "%s", cfg["Callsign"].c_str());
//    sprintf(temp_str, "%s", cur_setting.c_str());
//    u8g2.drawStr(87, 15, temp_str);

    // Draw key
//    sprintf(temp_str, "%s", cfg["PA Bias"].first.c_str());
//    u8g2.drawStr(87, 15, temp_str);

    // Draw timer and next event
//    sprintf(temp_str, "%lu", cur_timer);
//    u8g2.drawStr(0, 30, temp_str);
//    sprintf(temp_str, "%lu", next_event);
//    u8g2.drawStr(50, 30, temp_str);
//    sprintf(temp_str, "%i", RTC->MODE2.FREQCORR.reg);
//    u8g2.drawStr(0, 30, temp_str);
//    sprintf(temp_str, "%u", tx_buffer[cur_symbol]);
//    u8g2.drawStr(100, 30, temp_str);

//    sprintf(temp_str, "%u", cur_symbol);
//    u8g2.drawStr(111, 15, temp_str);
    
//      for(uint8_t i = 0; i < 18; ++i)
//      {
//        sprintf(temp_str, "%u", tx_buffer[i]);
//        u8g2.drawStr(i * 6, 30, temp_str);
//      }
//    sprintf(temp_str, "%lu", rtc.getEpoch());
//    u8g2.drawStr(0, 30, temp_str);
//    sprintf(temp_str, "%lu", next_time_sync);
//    u8g2.drawStr(66, 30, temp_str);


    // Draw band ID
//    sprintf(temp_str, "%lu", band_id);
//    u8g2.drawStr(0, 30, temp_str);

    // Draw callsign
    //sprintf(temp_str, "%s", cfg["Callsign"].substr(1).c_str());
    //sprintf(temp_str, "%s", cfg["Grid"].substr(1).c_str());
//    sprintf(temp_str, "%s", cfg["Power"].substr(1).c_str());
//    u8g2.drawStr(26, 30, temp_str);
    
  
    // Draw TX lock
    //u8g2.drawXBM(120, 8, 8, 8, lock_bits);
  }
  else // Draw settings menu
  {
    // Draw setting name
    u8g2.setFont(u8g2_font_6x10_mr);
    sprintf(temp_str, "%s", cur_setting.c_str());
    u8g2.drawStr(64 - u8g2.getStrWidth(temp_str) / 2, 10, temp_str);

    const char* setting_val = cur_setting_str.c_str();

    switch(cur_setting_type)
    {
    case SettingType::Uint:
      sprintf(temp_str, "%lu", cur_setting_uint);
      break;
    case SettingType::Int:
      sprintf(temp_str, "%l", cur_setting_int);
      break;
    case SettingType::Str:
      if(cur_setting_selected > 10)
      {
        setting_val += cur_setting_selected - 10;
      }
      //sprintf(temp_str, "%s", cur_setting_str.c_str());
      sprintf(temp_str, "%s", setting_val);
      break;
    }
    uint8_t str_x = (cur_setting_selected * SETTING_FONT_WIDTH > 60 ? 0 : 60 - cur_setting_selected * SETTING_FONT_WIDTH);
    //u8g2.setDrawColor(0);
    u8g2.drawStr(str_x, 20, temp_str);
    //u8g2.setDrawColor(1);

    // Find char in allowable list
    std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
    if(pos != std::string::npos)
    {
      cur_setting_char = pos;
    }
    
    sprintf(temp_str, "%d", cur_setting_selected);
    u8g2.drawStr(0, 10, temp_str);

    // Underline the current setting selection
    u8g2.drawLine(60, 21, 66, 21);
  }

  //yield();

  // Draw buffer or menu items
  
  // Arrows
  constexpr uint8_t triangle_top = 21;
  constexpr uint8_t triangle_bottom = 30;
  constexpr uint8_t triangle_center = 26;
  constexpr uint8_t left_triangle_left = 0;
  constexpr uint8_t left_triangle_right = 4;
  constexpr uint8_t right_triangle_left = 110;
  constexpr uint8_t right_triangle_right = 115;
    
  if(display_mode == DisplayMode::Menu)
  {
    // Menu items
    sprintf(menu_1, "%s", menu.getActiveChildLabel());
    sprintf(menu_2, "%s", menu.getActiveChildLabel(1));
    menu_1_x = 6;
    menu_2_x = 58;
    u8g2.setFont(u8g2_font_6x10_mf);
    //u8g2.setDrawColor(0);
    u8g2.drawStr(menu_1_x, 30, menu_1);
    u8g2.drawStr(menu_2_x, 30, menu_2);
    
    if(menu.countChildren() > 2)
    {
      u8g2.drawTriangle(left_triangle_right, triangle_top,
        left_triangle_right, triangle_bottom, 
        left_triangle_left, triangle_center);
      u8g2.drawTriangle(right_triangle_left, triangle_top,
        right_triangle_left, triangle_bottom, 
        right_triangle_right, triangle_center);
    }
    
    // Back icon
    u8g2.setFont(u8g2_font_m2icon_9_tf);
    u8g2.drawGlyph(118, 31, 0x0061);
  }
  else if(display_mode == DisplayMode::Setting)
  {
    sprintf(menu_1, "%s", "Ins/Del");
    sprintf(menu_2, "%s", "OK");
    menu_1_x = 6;
    menu_2_x = 80;
    u8g2.setFont(u8g2_font_6x10_mf);
    u8g2.drawStr(menu_1_x, 30, menu_1);
    u8g2.drawStr(menu_2_x, 30, menu_2);

    // Back icon
    u8g2.setFont(u8g2_font_m2icon_9_tf);
    u8g2.drawGlyph(118, 31, 0x0061);
  }
  else // Show the current buffer
  {
    //yield();
    if(cur_state != TxState::Idle)
    {
      //sprintf(menu_2, "%s", "End TX");
    }
    else
    {
      //sprintf(menu_2, "%s", "");
    }
    //sprintf(menu_1, "%s", "Ins/Del");
    menu_1_x = 6;
    menu_2_x = 80;
    u8g2.setFont(u8g2_font_6x10_mf);
    //u8g2.drawStr(menu_1_x, 30, menu_1);
    //u8g2.drawStr(menu_2_x, 30, menu_2);
    
    // Menu icon
    //yield();
    if(cur_state == TxState::Idle)
    {
      u8g2.setFont(u8g2_font_m2icon_9_tf);
      u8g2.drawGlyph(121, 31, 0x0042);
    }
    else
    {
      u8g2.setFont(u8g2_font_m2icon_9_tf);
      u8g2.drawGlyph(121, 31, 0x0043);
    }
  }
  //yield();
//  sprintf(menu_1, "%s", "DFCW");
//  sprintf(menu_2, "%s", "TX Enb");
//  menu_1_x = 32 - ((u8g2.getStrWidth(menu_1) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_1) / 2);
//  menu_2_x = 96 - ((u8g2.getStrWidth(menu_2) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_2) / 2);
//  
//  u8g2.setFont(u8g2_font_6x10_mr);
//  //u8g2.setDrawColor(0);
//  u8g2.drawStr(menu_1_x, 31, menu_1);
//  u8g2.drawStr(menu_2_x, 31, menu_2);
//}while(u8g2.nextPage());  
  yield();
  u8g2.sendBuffer();          // transfer internal memory to the display
  //yield();
}

void pollButtons()
{
  // Read buttons
  // ------------
  yield();
  // Handle up button
  if(digitalRead(BTN_UP) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_UP) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        
      }
      else if(display_mode == DisplayMode::Setting)
      {
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          ++cur_setting_uint;
          break;
        case SettingType::Int:
          ++cur_setting_int;
          break;
        case SettingType::Str:
          if(cur_setting_char == 0)
          {
            cur_setting_char = settings_str_chars.size() - 1;
          }
          else
          {
            cur_setting_char--;
          }
          cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_char];
          break;
        } 
      }
      else
      {
        if(cur_state == TxState::Idle)
        {
          if(base_frequency + power_10(tune_step) > upper_freq_limit)
          {
            base_frequency = upper_freq_limit;
          }
          else
          {
            base_frequency += power_10(tune_step);
          }
        }
      }
      
      delay(50); //delay to avoid many steps at one;
      yield();
    }
  }

  // Handle down button
  if(digitalRead(BTN_DOWN) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_DOWN) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        
      }
      else if(display_mode == DisplayMode::Setting)
      {
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          --cur_setting_uint;
          break;
        case SettingType::Int:
          --cur_setting_int;
          break;
        case SettingType::Str:
          if(cur_setting_char > settings_str_chars.size())
          {
            cur_setting_char = 0;
          }
          else
          {
            cur_setting_char++;
          }
          cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_char];
          break;
        }
      }
      else
      {
        if(cur_state == TxState::Idle)
        {
          if(base_frequency - power_10(tune_step) < lower_freq_limit)
          {
            base_frequency = lower_freq_limit;
          }
          else
          {
            base_frequency -= power_10(tune_step);
          }
        }
      }
      
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }

  // Handle left button
  if(digitalRead(BTN_LEFT) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_LEFT) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        if(menu.countChildren() > 2)
        {
          if(menu.active_child == 0 && menu.countChildren() % 2 == 1)
          {
            menu--;
          }
          else
          {
            menu--;
            menu--;
          }
        }
      }
      else if(display_mode == DisplayMode::Setting)
      {
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          --cur_setting_uint;
          break;
        case SettingType::Int:
          --cur_setting_int;
          break;
        case SettingType::Str:
          if(cur_setting_selected == 0)
          {
            //
          }
          else
          {
            cur_setting_selected--;
          }
          break;
        }
      }
      else
      {
        if(tune_step < 5)
        {
          tune_step++;
        }
        else
        {
          tune_step = 0;
        }
      }
      
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }

  // Handle right button
  if(digitalRead(BTN_RIGHT) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_RIGHT) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        if(menu.countChildren() > 2)
        {
          if(menu.active_child == menu.countChildren() - 1)
          {
            menu++;
          }
          else
          {
            menu++;
            menu++;
          }
        }
      }
      else if(display_mode == DisplayMode::Setting)
      {
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          --cur_setting_uint;
          break;
        case SettingType::Int:
          --cur_setting_int;
          break;
        case SettingType::Str:
          if(cur_setting_selected < cur_setting_str.size())
          {
            cur_setting_selected++;
          }
          break;
        }
      }
      else
      {
        if(tune_step == 0)
        {
          tune_step = 5;
        }
        else
        {
          tune_step--;
        }
      }
      
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }

  // Handle display 1 button
  if(digitalRead(BTN_DSP_1) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        MenuType type = menu.selectChild(menu.active_child);
        if(type == MenuType::Action)
        {
          display_mode = DisplayMode::Main;
          menu.selectRoot();
        }
      }
      else if(display_mode == DisplayMode::Setting)
      {
        // OK button
        char temp_str[42];
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          sprintf(temp_str, "U%lu", cur_setting_uint);
          cfg[cur_setting] = temp_str;
          break;
        case SettingType::Int:
          sprintf(temp_str, "I%l", cur_setting_int);
          cfg[cur_setting] = temp_str;
          break;
        case SettingType::Str:
          sprintf(temp_str, "S%s", cur_setting_str.c_str());
          cfg[cur_setting] = temp_str;
          break;
        }

        cur_setting_selected = 0;
        display_mode = DisplayMode::Main;
        menu.selectRoot();
      }
      else
      {
        //
      }
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }

  // Handle display 2 button
  if(digitalRead(BTN_DSP_2) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        MenuType type = menu.selectChild(menu.active_child + 1);
        if(type == MenuType::Action)
        {
          display_mode = DisplayMode::Main;
          menu.selectRoot();
        }
      }
      else if(display_mode == DisplayMode::Setting)
      {
        // OK button
        char temp_str[42];
        switch(cur_setting_type)
        {
        case SettingType::Uint:
          sprintf(temp_str, "U%lu", cur_setting_uint);
          cfg[cur_setting] = temp_str;
          break;
        case SettingType::Int:
          sprintf(temp_str, "I%l", cur_setting_int);
          cfg[cur_setting] = temp_str;
          break;
        case SettingType::Str:
          sprintf(temp_str, "S%s", cur_setting_str.c_str());
          cfg[cur_setting] = temp_str;
          break;
        }
        
        cur_setting_selected = 0;
        display_mode = DisplayMode::Main;
        menu.selectRoot();
      }
      else
      {
//        if(cur_state != TxState::Idle)
//        {
//          setTxState(TxState::Idle);
//          setNextTx(0);
//        }
      }
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }

  // Handle menu button
  if(digitalRead(BTN_BACK) == LOW)
  {
    delay(50);   // delay to debounce
    yield();
    if (digitalRead(BTN_BACK) == LOW)
    {
      if(display_mode == DisplayMode::Menu)
      {
        if(menu.selectParent()) // if we are at root menu, exit
        {
          display_mode = DisplayMode::Main;
        }
      }
      else
      {
        if(cur_state == TxState::Idle)
        {
          cur_setting_selected = 0;
          display_mode = DisplayMode::Menu;
        }
        else
        {
          setTxState(TxState::Idle);
          setNextTx(0);
        }
      }
      //display_menu ^= true;
      
      delay(50); //delay to avoid many steps at one
      yield();
    }
  }
}

void selectBand()
{
  // TODO: handle out of bounds
  band_id = analogRead(A0);
  yield();
  band_id = (band_id * ANALOG_REF) / 4096UL;
  yield();
  
  for(auto band : band_table)
  {
    if(band_id < band.upper_v && band_id > band.lower_v)
    {
      if(band.index != band_index)
      {
        band_index = band.index;
        lower_freq_limit = band.lower_limit;
        upper_freq_limit = band.upper_limit;
        if(base_frequency > upper_freq_limit || base_frequency < lower_freq_limit)
        {
          base_frequency = band.wspr_freq;
        }
      }
    }
  }
  yield();
}

void setTxState(TxState state)
{
  switch(state)
  {
  case TxState::Idle:
    digitalWrite(TX_KEY, LOW);
//    si5351.output_enable(SI5351_CLK0, 0);
//    setPABias(0);
    prev_state = cur_state;
    cur_state = state;
    break;
  case TxState::MFSK:
    SerialUSB.write('\f');
    //yield();
    cur_symbol = 0;
    frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
    change_freq = true;
    digitalWrite(TX_KEY, HIGH);
//    si5351.output_enable(SI5351_CLK0, 1);
//    setPABias(PA_BIAS_FULL);
    prev_state = cur_state;
    cur_state = state;
    next_event = cur_timer + cur_symbol_time;
    //cur_tone_spacing = mode_table[static_cast<uint8_t>(mode)].tone_spacing;
//    switch(mode)
//    {
//    case Mode::WSPR:
//      //jtencode.wspr_encode(cfg["Callsign"].c_str(), cfg["Grid"].c_str(), atoi(cfg["Power"].c_str()), tx_buffer);
//      break;
//    };
    break;
  case TxState::Preamble:
    break;
  default:
    break;
  }
  yield();
}

void setNextTx(uint8_t minutes)
{
//  struct tm cur_time = {rtc.getSeconds(), rtc.getMinutes(), rtc.getHours(),
//    rtc.getDay(), rtc.getMonth(), rtc.getYear(), 1, 0, 1};
//  time_t t = mktime(&cur_time);
//  time_t t = rtc.getEpoch();
  uint32_t t = rtc.getEpoch();
  uint16_t sec_to_add = (60 - rtc.getSeconds()) + (rtc.getMinutes() % 2 ? 0 : 60) + (minutes * 60);
  t += sec_to_add;
  next_tx = t;
  //rtc.setAlarmEpoch(t);
//  struct tm * n_tx = gmtime(&t);
//
//  rtc.setAlarmTime(n_tx->tm_hour, n_tx->tm_min, n_tx->tm_sec);
//  rtc.setAlarmDate(n_tx->tm_mday, n_tx->tm_mon, n_tx->tm_year);
}

void processSyncMessage()
{
  uint32_t pctime;
  constexpr uint32_t DEFAULT_TIME = 946684800; // 1 Jan 2000

  yield();

  if(SerialUSB.find(TIME_HEADER))
  {
    // check the integer is a valid time (greater than 1 Jan 2000)
    pctime = SerialUSB.parseInt();
    //yield();
    if(pctime >= DEFAULT_TIME)
    {
      rtc.setEpoch(pctime); // Sync RTC to the time received on the serial port
      time_sync_expire = pctime + TIME_EXPIRE;
      next_time_sync = pctime + TIME_SYNC_INTERVAL;
      if(initial_time_sync == 0)
      {
        initial_time_sync = pctime;
        setNextTx(0);
      }
    }
  }
  yield();
}

bool isTimeValid()
{
  if(time_sync_expire < rtc.getEpoch())
  {
    return true;
  }
  else
  {
    return false;
  }
}

void processTimeSync()
{
  static bool time_sync_request = false;
  
  // Check to see if we need to sync
  if(rtc.getEpoch() > next_time_sync)
  {
    SerialUSB.write(TIME_REQUEST);
    time_sync_request = true;
    next_time_sync = rtc.getEpoch() + TIME_SYNC_RETRY_RATE;
  }

  // Process time sync message if data is available on the serial port
  if(time_sync_request)
  {
    if(SerialUSB.available())
    {
      processSyncMessage();
      time_sync_request = false;
    }
  }

  // Indicate time sync status
  if(isTimeValid())
  {
    digitalWrite(SYNC_LED, HIGH);
  }
  else
  {
    digitalWrite(SYNC_LED, LOW);
  }
  yield();
}

void processTxTrigger()
{
  if(rtc.getEpoch() >= next_tx)
  {
    setTxState(TxState::MFSK);
    next_tx = UINT32_MAX;
  }
  yield();
}


void updateTimer(void)
{
  yield();
  
  // Latch the current time
  // MUST disable interrupts during this read or there will be an occasional corruption of cur_timer
  noInterrupts();
  cur_timer = millis();
  interrupts();
}

void txStateMachine()
{
switch(meta_mode)
  {
  case MetaMode::MORSE:
    break;
  case MetaMode::MFSK:
    switch(cur_state)
    {
    case TxState::Idle:
      break;
    case TxState::MFSK:
      if(cur_timer >= next_event)
      {
        ++cur_symbol;
        if(cur_symbol >= cur_symbol_count) //reset everything and switch to idle
        {
          SerialUSB.write('\b');
          setTxState(TxState::Idle);
          //frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
          frequency = (base_frequency * 100);
          change_freq = true;
          setNextTx(0);
        }
        else // next symbol
        {
          next_event = cur_timer + cur_symbol_time;
          frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
          change_freq = true;
        }
      }
      break;
    case TxState::Preamble:
      break;
    }
    break;
  }
  yield();
}

// ===== Setup =====
void setup()
{
  // Serial port init
  SerialUSB.begin(57600);
  while(!SerialUSB);



  // Load config map
  for(auto const& c : config_table)
  {
    cfg[c[0]] = c[1];
  }
  
  // Start u8g2
  u8g2.begin();

  // I/O init
  pinMode(BTN_DSP_1, INPUT_PULLUP);
  pinMode(BTN_DSP_2, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(CLK_INPUT, INPUT);
  pinMode(TX_KEY, OUTPUT);
  pinMode(SYNC_LED, OUTPUT);

  //attachInterrupt(digitalPinToInterrupt(BTN_BACK), handleMenuBack, FALLING);
  


  // ADC resolution
  analogReadResolution(12);
  analogReference(AR_DEFAULT);

  // Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);
  si5351.set_freq(base_frequency * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.set_freq(1000000UL, SI5351_CLK2);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  Wire.setClock(400000UL);

  // Set PA bias
  setPABias(PA_BIAS_FULL);

  // RTC setup
  // Can't use the RTC alarm interrupt, far too much trigger time variance
  rtc.begin();
  rtc.setTime(DEFAULT_TIME.tm_hour, DEFAULT_TIME.tm_min, DEFAULT_TIME.tm_sec);
  rtc.setDate(DEFAULT_TIME.tm_mday, DEFAULT_TIME.tm_mon, DEFAULT_TIME.tm_year);
  time_sync_expire = rtc.getEpoch();
  next_time_sync = rtc.getEpoch();

  // 
  setNextTx(0);

  // Init menu
  initMenu();

  // Set up scheduler
  Scheduler.startLoop(txStateMachine);
  //Scheduler.startLoop(updateTimer);
  //Scheduler.startLoop(processTxTrigger);
  Scheduler.startLoop(drawOLED, 10240);
  Scheduler.startLoop(pollButtons);
  Scheduler.startLoop(selectBand);
  //Scheduler.startLoop(processTimeSync);

  // Start Timer
  startTimer(TIMER_FREQUENCY); // 1 ms ISR

  cur_tone_spacing = mode_table[static_cast<uint8_t>(mode)].tone_spacing;
  cur_symbol_count = mode_table[static_cast<uint8_t>(mode)].symbol_count;
  cur_symbol_time = mode_table[static_cast<uint8_t>(mode)].symbol_time;

  // Clear TX buffer
  memset(tx_buffer, 0, 255);
  jtencode.wspr_encode(cfg["Callsign"].substr(1).c_str(), cfg["Grid"].substr(1).c_str(), 
    atoi(cfg["Power"].substr(1).c_str()), tx_buffer);
  //jtencode.wspr_encode("NT7S", "CN85", 23, tx_buffer);
  setTxState(TxState::Idle);
  frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
  change_freq = true;

  //morse.send("DE NT7S");
  
  //attachInterrupt(digitalPinToInterrupt(CLK_INPUT), handleClkInput, FALLING);
}

void loop()
{
//  noInterrupts();
//  cur_timer = millis();
//  interrupts();
//  yield();

  if(change_freq)
  {
    //noInterrupts();
    si5351.set_freq(frequency, SI5351_CLK0);
    change_freq = false;
    //interrupts();
  }

  yield();
  
//  switch(meta_mode)
//  {
//  case MetaMode::MORSE:
//    break;
//  case MetaMode::MFSK:
//    switch(cur_state)
//    {
//    case TxState::Idle:
//      break;
//    case TxState::MFSK:
//      if(cur_timer >= next_event)
//      {
//        ++cur_symbol;
//        if(cur_symbol >= cur_symbol_count) //reset everything and switch to idle
//        {
//          SerialUSB.write('\b');
//          setTxState(TxState::Idle);
//          //frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
//          frequency = (base_frequency * 100);
//          change_freq = true;
//          setNextTx(0);
//        }
//        else // next symbol
//        {
//          next_event = cur_timer + cur_symbol_time;
//          frequency = (base_frequency * 100) + (tx_buffer[cur_symbol] * cur_tone_spacing);
//          change_freq = true;
//        }
//      }
//      break;
//    case TxState::Preamble:
//      break;
//    }
//    break;
//  }
//  yield();

  processTxTrigger();
  //drawOLED();
//  pollButtons();
//  selectBand();
  processTimeSync();
}
