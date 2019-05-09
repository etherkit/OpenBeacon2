// OpenBeacon Mini
// Etherkit
//
// Rev 3 May 2019
//
// Hardware Requirements
// ---------------------
// This firmware must be run on an Etherkit Empyrean Alpha or Arduino Zero capable microcontroller
//
// Required Libraries
// ------------------
// Flash Storage (Library Manager)
// TinyGPS++ (http://arduiniana.org/libraries/tinygpsplus/)
// ArduinoSTL (Library Manager)
// u8g2lib (Library Manager)
// Etherkit Menu (https://github.com/etherkit/MenuArduino)
// Etherkit Morse (Library Manager)
// Etherkit Si5351 (Library Manager)
// Etherkit JTEncode (Library Manager)
// Scheduler (Library Manager)
// ArduinoJson (Library Manager)
// External EEPROM (extEEPROM) (Library Manager)
// Wire (Arduino Standard Library)
// RTCZero (Library Manager)

//#define REV_A
#define REV_B

#ifdef REV_B
#define EXT_EEPROM // Microchip 24AA64T
#endif

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
#ifdef EXT_EEPROM
#include <extEEPROM.h>
#else
#include <FlashStorage.h>
#endif

#include <cstdlib>
#include <map>
#include <string>
#include <time.h>

#include "bands.h"
#include "modes.h"

// Enumerations
enum class DisplayMode {Main, Menu, Setting, Buffer, Modal};
enum class SettingType {Uint, Int, Str, Float, Time, Bool};
enum class TxState {Idle, MFSK, CW, DFCW, CWID, IDDelay, Preamble};

// Hardware constexprs
#ifdef REV_A
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
#endif

#ifdef REV_B
constexpr uint8_t BTN_DSP_1 = 8;
constexpr uint8_t BTN_DSP_2 = 9;
constexpr uint8_t BTN_UP = 2;
constexpr uint8_t BTN_DOWN = 3;
constexpr uint8_t BTN_LEFT = 7;
constexpr uint8_t BTN_RIGHT = 5;
constexpr uint8_t BTN_BACK = 6;
constexpr uint8_t BAND_SW = 11;
constexpr uint8_t TX_KEY = 12;
constexpr uint8_t TX_LED = 13;
constexpr uint8_t SYNC_LED = 26;
constexpr uint8_t ADC_BAND_ID_1 = 0;
constexpr uint8_t ADC_BAND_ID_2 = 1;
#endif

constexpr uint8_t MCP4725A1_BUS_BASE_ADDR = 0x62;
constexpr uint16_t MCP4725A1_VREF = 5000UL;
constexpr uint16_t ANALOG_REF = 3400UL; // TODO
constexpr uint16_t PA_BIAS_FULL = 1850UL;

constexpr uint8_t EEP_24AA64T_BUS_BASE_ADDR = 0x50;
constexpr uint16_t EEP_24AA64T_BLOCK_SIZE = 32;  // bytes
constexpr uint16_t EEP_24AA64T_CAPACITY = 8192;  // bytes

constexpr uint32_t TIMER_BASE_CLOCK = 4000000;
constexpr uint16_t TIMER_PRESCALER_DIV = 1;
constexpr uint16_t TIMER_FREQUENCY = 1000;

constexpr uint8_t TIME_REQUEST = '\a';
constexpr uint8_t TIME_HEADER = 'T';
constexpr uint32_t TIME_EXPIRE = 28800;
constexpr uint32_t TIME_SYNC_INTERVAL = 14400;
constexpr uint32_t TIME_SYNC_RETRY_RATE = 60;

constexpr uint8_t MSG_BUFFER_SIZE = 41;

constexpr uint8_t CONFIG_SCHEMA_VERSION = 1;

constexpr char PACKET_ID = '\a'; // ASCII BEL
constexpr char PACKET_TERM = '\n'; // ASCII LF
//constexpr uint8_t PACKET_MSG_TYPE_0 = 0;
constexpr uint16_t JSON_MAX_SIZE = 900;

constexpr static unsigned char lock_bits[] = {
  0x18, 0x24, 0x24, 0x7e, 0x81, 0x81, 0x81, 0x7e
};

const std::string settings_str_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-+/. ";

// Configuration schema version 1
// 8 Oct 2018
struct Config
{
  boolean valid;
  uint8_t version;
  Mode mode;
  uint8_t band;
  uint64_t base_freq;
  uint16_t wpm;
  uint8_t tx_intv;
  uint8_t dfcw_offset;
  uint8_t buffer;
  char callsign[20];
  char grid[10];
  uint8_t power;
  uint16_t pa_bias;
  boolean cwid;
  char msg_buffer_1[MSG_BUFFER_SIZE];
  char msg_buffer_2[MSG_BUFFER_SIZE];
  char msg_buffer_3[MSG_BUFFER_SIZE];
  char msg_buffer_4[MSG_BUFFER_SIZE];
  int32_t si5351_int_corr; 
};

// Instantiate flash storage
#ifndef EXT_EEPROM
FlashStorage(flash_store, Config);
Config flash_config;
#endif

// Defaults
constexpr uint32_t DEFAULT_FREQUENCY = 0UL;
constexpr uint32_t DEFAULT_LOWER_FREQ_LIMIT = 0UL;
constexpr uint32_t DEFAULT_UPPER_FREQ_LIMIT = 0UL;
constexpr uint8_t DEFAULT_BAND_MODULE_INDEX = 0;
constexpr uint8_t DEFAULT_BAND_INDEX = 0;
constexpr MetaMode DEFAULT_METAMODE = MetaMode::MFSK;
constexpr Mode DEFAULT_MODE = Mode::WSPR;
constexpr bool DEFAULT_TX_LOCK = false;
constexpr bool DEFAULT_TX_ENABLE = false;
constexpr DisplayMode DEFAULT_DISPLAY_MODE = DisplayMode::Main;
constexpr TxState DEFAULT_STATE = TxState::Idle;
constexpr uint16_t DEFAULT_TX_INTERVAL = 4;
constexpr uint8_t DEFAULT_CUR_BUFFER = 1;
constexpr uint8_t DEFAULT_DFCW_OFFSET = 5;
constexpr char DEFAULT_CALLSIGN[20] = "NT7S";
constexpr char DEFAULT_GRID[10] = "AA00";
constexpr uint8_t DEFAULT_POWER = 23;
#ifdef REV_A
constexpr uint16_t DEFAULT_PA_BIAS = 1800;
#endif
#ifdef REV_B
constexpr uint16_t DEFAULT_PA_BIAS = 2000;
#endif
constexpr boolean DEFAULT_CWID = true;
constexpr char DEFAULT_MSG_1[41] = "BUFFER1";
constexpr char DEFAULT_MSG_2[41] = "BUFFER2";
constexpr char DEFAULT_MSG_3[41] = "BUFFER3";
constexpr char DEFAULT_MSG_4[41] = "BUFFER4";
constexpr uint64_t DEFAULT_SI5351_INT_CORR = 0ULL;

struct tm DEFAULT_TIME = {0, 1, 18, 19, 3, 2018, 1, 0, 1};

// Character 0 of the value field denotes setting type:
// S == string
// U == uint
// I == int
// F == float
// T == time
// B == boolean
const std::string settings_table[][2] =
{
  {"pa_bias", "PA Bias"},
  {"callsign", "Callsign"},
  {"grid", "Grid"},
  {"power", "Power"},
  {"tx_intv", "TX Intv"},
  {"wpm", "CW WPM"},
  {"cwid", "CW ID"}
};
//const char* settings_table[][3] =
//{
//  {"pa_bias", "PA Bias", "U1801"},
//  {"callsign", "Callsign", "SNT7S"},
//  {"grid", "Grid", "SCN85"},
//  {"power", "Power", "U23"},
//  {"tx_intv", "TX Intv", "U0"},
//  {"wpm", "CW WPM", "U22"}
//};

const char* default_config = 
  "{\"valid\":\"true\", \"version\":1, \"mode\":\"MODE::WSPR\", \"band\":0, \"wpm\":25, \"tx_intv\":6, \"dfcw_offset\":5, \"buffer\":0, \"callsign\":\"N0CALL\", \"grid\":\"AA00\", \"power\":23, \"pa_bias\":1800, \"cwid\":true, \"msg_buffer_1\":\"\", \"msg_buffer_2\":\"\", \"msg_buffer_3\":\"\", \"msg_buffer_4\":\"\",\"si5351_int_corr\":0}";

// Limits
constexpr uint8_t SETTING_FONT_WIDTH = 6;

// Typedef
typedef std::pair<std::string, std::string> settings_value_type;
typedef std::map<std::string, settings_value_type> settings_type; //<key, <display_name, value>>

// Class constructors
//U8G2_SSD1306_128X32_UNIVISION_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Si5351 si5351;
Morse morse(TX_KEY, 15);
RTCZero rtc;
Menu menu;
JTEncode jtencode;
#ifdef EXT_EEPROM
extEEPROM eeprom(kbits_64, 1, EEP_24AA64T_BLOCK_SIZE, EEP_24AA64T_BUS_BASE_ADDR);
#endif

// ISR global variables
//volatile uint64_t base_frequency = DEFAULT_FREQUENCY; // 30m WSPR
volatile uint64_t frequency = DEFAULT_FREQUENCY;
volatile uint32_t last_reported_pos;   // change management
volatile uint32_t lower_freq_limit = DEFAULT_LOWER_FREQ_LIMIT;
volatile uint32_t upper_freq_limit = DEFAULT_UPPER_FREQ_LIMIT;
volatile MetaMode meta_mode = DEFAULT_METAMODE;
//volatile Mode mode = DEFAULT_MODE;
volatile DisplayMode display_mode = DEFAULT_DISPLAY_MODE;
//volatile uint16_t tx_interval = DEFAULT_TX_INTERVAL;
volatile TxState cur_state = DEFAULT_STATE;
volatile TxState prev_state = DEFAULT_STATE;
volatile TxState next_state = DEFAULT_STATE;
volatile uint32_t cur_timer, next_event;
volatile uint8_t clk_temp = 0;
volatile uint8_t cur_symbol = 0;
volatile uint16_t cur_tone_spacing = 0;
volatile bool change_freq = false;

// Global variables
#ifdef REV_B
uint32_t band_id_1 = 0;
uint32_t band_id_2 = 0;
uint8_t band_module_index_1 = DEFAULT_BAND_MODULE_INDEX;
uint8_t band_module_index_2 = DEFAULT_BAND_MODULE_INDEX;
#endif
uint8_t tune_step = 0;
uint32_t band_id = 0;
//uint8_t band_index = 0;
settings_type settings;
//std::string cur_setting = "";
uint64_t cur_setting_uint = 0;
//uint64_t temp_setting_uint = 0;
int64_t cur_setting_int = 0;
//int64_t temp_setting_int = 0;
std::string cur_setting_label = "";
std::string cur_setting_key = "";
std::string cur_setting_str = "";
std::string temp_setting_str = "";
float cur_setting_float;
boolean cur_setting_bool;
SettingType cur_setting_type = SettingType::Str;
uint8_t cur_setting_selected = 0;
uint8_t cur_setting_len = 0;
//std::string settings_str_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-+/.";
uint8_t cur_setting_char = 0;
uint8_t cur_setting_index = 0;
char cur_callsign[21];
char cur_grid[5];
uint8_t cur_power;
uint32_t next_tx = UINT32_MAX;
char next_tx_time[14];
uint32_t time_sync_expire = 0;
uint32_t next_time_sync = 0;
uint32_t initial_time_sync = 0;
//bool time_sync_request = false;
uint8_t mfsk_buffer[255];
char wspr_buffer[41];
char msg_buffer[MSG_BUFFER_SIZE];
//char msg_buffer_1[MSG_BUFFER_SIZE];
//char msg_buffer_2[MSG_BUFFER_SIZE] = "TESTING";
//char msg_buffer_3[MSG_BUFFER_SIZE];
//char msg_buffer_4[MSG_BUFFER_SIZE];
//uint8_t cur_buffer = DEFAULT_CUR_BUFFER;
uint8_t cur_symbol_count;
uint16_t cur_symbol_time;
uint16_t cur_tx_interval_mult;
uint16_t cur_tx_interval = DEFAULT_TX_INTERVAL;
bool tx_lock = DEFAULT_TX_LOCK;
bool tx_enable = DEFAULT_TX_ENABLE;
//float wpm = DEFAULT_WPM;
uint8_t cur_setting_digit = 0;
uint8_t cur_edit_buffer;
bool ins_del_mode = false;
uint16_t cur_pa_bias = DEFAULT_PA_BIAS;
uint16_t cur_dfcw_offset = DEFAULT_DFCW_OFFSET;
uint64_t cur_si5351_int_corr = DEFAULT_SI5351_INT_CORR;
uint32_t cwid_delay = 1000;
uint32_t cwid_start;
uint8_t cwid_wpm = 30;
Config cur_config; 
boolean disable_display_loop = false;
uint8_t cur_band_module = 0;
std::string band_name;
bool time_sync_request = false;
bool select_band_reset = false;

StaticJsonDocument<JSON_MAX_SIZE> json_rx_doc;
StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;

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
}

constexpr unsigned long power_10(unsigned long exponent)
{
  // bounds checking pls
  return (exponent == 0) ? 1 : 10 * power_10(exponent - 1);
}

uint8_t num_digits(uint64_t number)
{
  uint8_t result = 0;

  do
  {
    number /= 10;
    result++;
  }while(number > 0);

  return result;
}

// Voltage specified in millivolts
void setPABias(uint16_t voltage)
{
  uint32_t reg;
  uint8_t reg1, reg2;

  // Bounds checking
  if (voltage > MCP4725A1_VREF)
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
  for (auto i : mode_table)
  {
    uint8_t index = static_cast<uint8_t>(i.index);
    menu.addChild(i.mode_name, selectMode, index);
  }
  menu.selectParent();
  menu.addChild("Buf Sel");
  menu.selectChild(1);
  menu.addChild("1", selectBuffer, 1);
  menu.addChild("2", selectBuffer, 2);
  menu.addChild("3", selectBuffer, 3);
  menu.addChild("4", selectBuffer, 4);
  menu.selectParent();
  menu.addChild("Buf Edit");
  menu.selectChild(2);
  menu.addChild("1", setBuffer, "1");
  menu.addChild("2", setBuffer, "2");
  menu.addChild("3", setBuffer, "3");
  menu.addChild("4", setBuffer, "4");
  menu.selectParent();
  menu.addChild("Settings");
  menu.selectChild(3);
  for (auto c : settings_table)
  {
//    const char* key = c[0].c_str();
//    const char* label = c[1].c_str();
//    char key[20];
//    char label[20];
//    strcpy(key, c[0]);
//    strcpy(label, c[1]);
//    menu.addChild(label, setConfig, key);
    menu.addChild(c[1].c_str(), setConfig, c[0].c_str());
  }
  menu.selectParent();
  menu.addChild("Reset", resetConfig, 0);
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
    // TODO: need to look into some weirdness with the keying pin flipping during this ISR
    if (meta_mode == MetaMode::CW or meta_mode == MetaMode::DFCW or cur_state == TxState::CWID)
    {
      morse.update();
    }
    TC->INTFLAG.bit.MC0 = 1;
  }
}

// ===== Callbacks =====
void selectMode(uint8_t sel)
{
  switch (static_cast<Mode>(sel))
  {
    case Mode::DFCW3:
//      mode = Mode::DFCW3;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW3;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      settings["TX Intv"] = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW6:
//      mode = Mode::DFCW6;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW6;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW10:
//      mode = Mode::DFCW10;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW10;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::DFCW120:
//      mode = Mode::DFCW120;
//      cur_config.mode = mode;
      cur_config.mode = Mode::DFCW120;
      meta_mode = MetaMode::DFCW;
      setTxState(TxState::Idle);
      next_state = TxState::DFCW;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS3:
//      mode = Mode::QRSS3;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS3;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS6:
//      mode = Mode::QRSS6;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS6;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS10:
//      mode = Mode::QRSS10;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS10;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::QRSS120:
//      mode = Mode::QRSS120;
//      cur_config.mode = mode;
      cur_config.mode = Mode::QRSS120;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_HIGH);
      digitalWrite(TX_KEY, HIGH);
      #endif
      break;
    case Mode::CW:
//      mode = Mode::CW;
//      cur_config.mode = mode;
      cur_config.mode = Mode::CW;
      meta_mode = MetaMode::CW;
      setTxState(TxState::Idle);
      next_state = TxState::CW;
//      composeBuffer();
      cur_config.wpm = mode_table[static_cast<uint8_t>(cur_config.mode)].WPM;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::HELL:
//      mode = Mode::HELL;
//      cur_config.mode = mode;
      cur_config.mode = Mode::HELL;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      composeBuffer();
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].cw_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::WSPR:
//      mode = Mode::WSPR;
//      cur_config.mode = mode;
      cur_config.mode = Mode::WSPR;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      selectBuffer(cur_buffer);
//      composeMFSKMessage();
//      composeBuffer(1);
//      memset(mfsk_buffer, 0, 255);
//      jtencode.wspr_encode(cur_callsign, cur_grid, cur_power, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].wspr_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT65:
//      mode = Mode::JT65;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT65;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt65_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt65_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT9:
//      mode = Mode::JT9;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT9;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt9_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::JT4:
//      mode = Mode::JT4;
//      cur_config.mode = mode;
      cur_config.mode = Mode::JT4;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
      selectBuffer(cur_config.buffer);
//      composeMFSKMessage();
//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.jt4_encode(msg_buffer, mfsk_buffer);
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
    case Mode::FT8:
//      mode = Mode::FT8;
//      cur_config.mode = mode;
      cur_config.mode = Mode::FT8;
      meta_mode = MetaMode::MFSK;
      setTxState(TxState::Idle);
      next_state = TxState::MFSK;
//      selectBuffer(cur_buffer);
      composeMFSKMessage();

//      composeBuffer();
//      memset(mfsk_buffer, 0, 255);
//      jtencode.ft8_encode(msg_buffer, mfsk_buffer);
//      SerialUSB.write('\v');
//      for(uint8_t i = 0; i < FT8_SYMBOL_COUNT; ++i)
//      {
//        SerialUSB.print(mfsk_buffer[i]);
//      }
      cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
//      SerialUSB.write('\v');
//      SerialUSB.print(cur_tone_spacing, DEC);
      cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
      cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//      cur_config.tx_intv = mode_table[static_cast<uint8_t>(cur_config.mode)].tx_interval_mult;
      cur_config.base_freq = band_table[cur_config.band].ft8_freq;
      #ifdef REV_B
//      si5351.set_clock_disable(SI5351_CLK0, SI5351_CLK_DISABLE_LOW);
      digitalWrite(TX_KEY, LOW);
      #endif
      break;
  }
  serializeConfig();
  if(next_tx != UINT32_MAX)
  {
    setNextTx(cur_config.tx_intv);
  }
  yield();
}

void setBuffer(const char * b, const char * value)
{
  std::size_t pos;
  uint8_t buf;
  buf = atoi(b);

  if (buf >= 1 && buf <= 4)
  {
    switch (buf)
    {
      case 1:
        cur_setting_str = cur_config.msg_buffer_1;
        break;
      case 2:
        cur_setting_str = cur_config.msg_buffer_2;
        break;
      case 3:
        cur_setting_str = cur_config.msg_buffer_3;
        break;
      case 4:
        cur_setting_str = cur_config.msg_buffer_4;
        break;
    }
    cur_edit_buffer = buf;
    cur_setting_type = SettingType::Str;
//    display_mode = DisplayMode::Setting;
    display_mode = DisplayMode::Buffer;
    cur_setting_selected = 0;
    pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
    if (pos != std::string::npos)
    {
      cur_setting_index = (uint8_t)pos;
    }
  }
}

void selectBuffer(const uint8_t buf)
{
  switch(buf)
  {
  case 1:
    strcpy(msg_buffer, cur_config.msg_buffer_1);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 2:
    strcpy(msg_buffer, cur_config.msg_buffer_2);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 3:
    strcpy(msg_buffer, cur_config.msg_buffer_3);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  case 4:
    strcpy(msg_buffer, cur_config.msg_buffer_4);
//    cur_buffer = buf;
    cur_config.buffer = buf;
    break;
  }
//  cur_config.buffer = buf;
  composeMFSKMessage();
//  SerialUSB.write('\v');
//  for(uint8_t i = 0; i < FT8_SYMBOL_COUNT; ++i)
//  {
//    SerialUSB.print(mfsk_buffer[i]);
//  }
}

void setConfig(const char * key, const char * label)
{
  display_mode = DisplayMode::Setting;
  cur_setting_key = std::string(key);
//  cur_setting_label = settings[key].first;
  cur_setting_label = std::string(label);
  std::string val = settings[key].second;
  char temp_str[41];
  char type = val[0];
  std::size_t pos;

  switch (type)
  {
    case 'U':
      cur_setting_uint = atoll(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Uint;
      sprintf(temp_str, "%lu", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'I':
      cur_setting_int = atoll(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Int;
      sprintf(temp_str, "%l", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'S':
      cur_setting_str = settings[key].second.substr(1);
      cur_setting_type = SettingType::Str;
      sprintf(temp_str, "%s", cur_setting_str.c_str());
      //    cur_setting_selected = strlen(temp_str) - 1;
      cur_setting_selected = 0;
      pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
      if (pos != std::string::npos)
      {
        cur_setting_index = (uint8_t)pos;
      }
      break;
    case 'F':
      cur_setting_float = atof(settings[key].second.substr(1).c_str());
      cur_setting_type = SettingType::Float;
      sprintf(temp_str, "%f", cur_setting_uint);
      cur_setting_selected = 0;
      break;
    case 'B':
      if(val[1] == '1')
      {
        cur_setting_bool = true;
      }
      else
      {
        cur_setting_bool = false;
      }
      cur_setting_type = SettingType::Bool;
      break;
  }
}

void resetConfig(const uint8_t x)
{
  disable_display_loop = true;
  
  // TODO: get confirmation
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_6x10_mr);
  u8g2.drawStr(64 - u8g2.getStrWidth("Reset Configuration?") / 2, 10, "Reset Configuration?");
  u8g2.drawStr(6, 29, "OK");
  u8g2.drawStr(80, 29, "Cancel");
  u8g2.sendBuffer();
  delay(500);
  while (digitalRead(BTN_DSP_1) == LOW || digitalRead(BTN_DSP_2) == LOW)
  {
  }
  delay(50);
  
  while (digitalRead(BTN_DSP_1) == HIGH && digitalRead(BTN_DSP_2) == HIGH)
  {
    delay(50);
  }
  if (digitalRead(BTN_DSP_2) == LOW)
  {
    delay(50);
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      disable_display_loop = false;
      return;
    }
  }
  else if (digitalRead(BTN_DSP_1) == LOW)
  {
    delay(50);
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      // Display wait message
      u8g2.clearBuffer();
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_6x10_mr);
      u8g2.drawStr(64 - u8g2.getStrWidth("Resetting.") / 2, 10, "Resetting.");
      u8g2.drawStr(64 - u8g2.getStrWidth("Please wait...") / 2, 25, "Please wait...");
      u8g2.sendBuffer();

      #ifdef EXT_EEPROM
      // Erase EEPROM
      eraseEEPROM();
      #endif
      
      // Load default values into config
      char temp_str[81];
      cur_config.valid = true;
      cur_config.version = CONFIG_SCHEMA_VERSION;
      cur_config.mode = DEFAULT_MODE;
      cur_config.band = DEFAULT_BAND_INDEX;
      cur_config.base_freq = DEFAULT_FREQUENCY;
      cur_config.wpm = DEFAULT_WPM;
      cur_config.tx_intv = DEFAULT_TX_INTERVAL;
      cur_config.dfcw_offset = DEFAULT_DFCW_OFFSET;
      cur_config.buffer = DEFAULT_CUR_BUFFER;
      strcpy(cur_config.callsign, DEFAULT_CALLSIGN);
      strcpy(cur_config.grid, DEFAULT_GRID);
      cur_config.power = DEFAULT_POWER;
      cur_config.pa_bias = DEFAULT_PA_BIAS;
      cur_config.cwid = DEFAULT_CWID;
      strcpy(cur_config.msg_buffer_1, DEFAULT_MSG_1);
      strcpy(cur_config.msg_buffer_2, DEFAULT_MSG_2);
      strcpy(cur_config.msg_buffer_3, DEFAULT_MSG_3);
      strcpy(cur_config.msg_buffer_4, DEFAULT_MSG_4);
      cur_config.si5351_int_corr = DEFAULT_SI5351_INT_CORR;
//      mode = cur_config.mode;
//      strcpy(msg_buffer_1, cur_config.msg_buffer_1);
//      strcpy(msg_buffer_2, cur_config.msg_buffer_2);
//      strcpy(msg_buffer_3, cur_config.msg_buffer_3);
//      strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//      cur_buffer = cur_config.buffer;
      selectBuffer(cur_config.buffer);
      cur_tone_spacing = cur_config.dfcw_offset;
      sprintf(temp_str, "U%lu", cur_config.pa_bias);
      settings["pa_bias"].second = std::string(temp_str);
      sprintf(temp_str, "S%s", cur_config.callsign);
      settings["callsign"].second = std::string(temp_str);
      sprintf(temp_str, "S%s", cur_config.grid);
      settings["grid"].second = std::string(temp_str);
      sprintf(temp_str, "U%lu", cur_config.power);
      settings["power"].second = std::string(temp_str);
      sprintf(temp_str, "U%lu", cur_config.tx_intv);
      settings["tx_intv"].second = std::string(temp_str);
      sprintf(temp_str, "U%lu", cur_config.wpm);
      settings["wpm"].second = std::string(temp_str);
      if(cur_config.cwid)
      {
        settings["cwid"].second = "B1";
      }
      else
      {
        settings["cwid"].second = "B0";
      }
    
      // Save config to EEPROM
      serializeConfig();

      select_band_reset = true;
    }
  }

  disable_display_loop = false;
}

// ===== Main routines =====

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

  yield();

  if(!disable_display_loop)
  {
    // u8g2 draw loop
    // --------------
    u8g2.clearBuffer();          // clear the internal memory
  
    if(display_mode == DisplayMode::Main || display_mode == DisplayMode::Menu)
    {
      u8g2.setFont(u8g2_font_logisoso16_tn);
      //yield();
      u8g2.setDrawColor(1);
      //u8g2.setFont(u8g2_font_inb19_mn);
    
      // MHz
      yield(); // you need an odd number of yields or you get a strange bimodal distribution
               // of transmit times, with half being too long
      freq = cur_config.base_freq;
  
      if(cur_config.base_freq / 1000000UL > 0)
      {
        sprintf(temp_str, "%3lu", freq / 1000000UL);
        zero_pad = 1;
      }
      else
      {
        sprintf(temp_str, "   ");
        //u8g2.drawStr(0, 17, temp_str);
      }
      yield();
      // We do this because the desired font isn't quite monospaced :-/
      for(uint8_t i = 0; i < 3; ++i)
      {
        //memmove(temp_chr, temp_str + i, 1);
        sprintf(temp_chr, "%c", temp_str[i]);
        yield();
        u8g2.drawStr(i * 9, 17, temp_chr);
        yield();
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
      yield();
      for(uint8_t i = 0; i < 3; ++i)
      {
        //memmove(temp_chr, temp_str + i, 1);
        sprintf(temp_chr, "%c", temp_str[i]);
        yield();
        u8g2.drawStr(i * 9 + 29, 17, temp_chr);
        yield();
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
      yield();
      for(uint8_t i = 0; i < 3; ++i)
      {
        //memmove(temp_chr, temp_str + i, 1);
        sprintf(temp_chr, "%c", temp_str[i]);
        yield();
        u8g2.drawStr(i * 9 + 58, 17, temp_chr);
        yield();
      }
      
      // Indicate step size
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
  
      //yield();
    
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
//      yield();
      //u8g2.setFont(u8g2_font_6x10_mr);
      u8g2.setFont(u8g2_font_5x7_tn);
      sprintf(temp_str, "%02u:%02u:%02u", rtc.getHours(),
        rtc.getMinutes(), rtc.getSeconds());
      yield();
      u8g2.drawStr(88, 6, temp_str);
    
      // Draw mode
      yield();
      u8g2.setFont(u8g2_font_6x10_mr);
      sprintf(temp_str, "%s", mode_table[static_cast<uint8_t>(cur_config.mode)].mode_name);
      u8g2.drawStr(87, 15, temp_str);
      yield();
    
      // Draw index
      //sprintf(temp_str, "%s", settings["Callsign"].c_str());
  //    sprintf(temp_str, "%s", cur_setting.c_str());
  //    u8g2.drawStr(87, 15, temp_str);
  
      // Draw key
  //    sprintf(temp_str, "%s", settings["PA Bias"].first.c_str());
  //    u8g2.drawStr(87, 15, temp_str);
  
      // Draw timer and next event
  //    sprintf(temp_str, "%lu", cur_timer);
  //    u8g2.drawStr(0, 30, temp_str);
  //    sprintf(temp_str, "%lu", next_event);
  //    u8g2.drawStr(50, 30, temp_str);
  //    sprintf(temp_str, "%i", RTC->MODE2.FREQCORR.reg);
  //    u8g2.drawStr(0, 30, temp_str);
//      sprintf(temp_str, "%u", mfsk_buffer[cur_symbol]);
//      u8g2.drawStr(100, 30, temp_str);
  
//      sprintf(temp_str, "%u", cur_state);
//      u8g2.drawStr(0, 8, temp_str);
//      sprintf(temp_str, "%u", cur_symbol);
//      u8g2.drawStr(111, 15, temp_str);
  //    sprintf(temp_str, "%u", morse.tx);
  //    u8g2.drawStr(122, 15, temp_str);
      
  //      for(uint8_t i = 0; i < 18; ++i)
  //      {
  //        sprintf(temp_str, "%u", mfsk_buffer[i]);
  //        u8g2.drawStr(i * 6, 30, temp_str);
  //      }
//      sprintf(temp_str, "%lu", rtc.getEpoch());
//      u8g2.drawStr(39, 30, temp_str);
  //    sprintf(temp_str, "%lu", next_time_sync);
  //    u8g2.drawStr(66, 30, temp_str);
//      sprintf(temp_str, "%lu", time_sync_expire);
//      u8g2.drawStr(39, 30, temp_str);
//      sprintf(temp_str, "%ld", time_sync_expire - rtc.getEpoch());
//      u8g2.drawStr(39, 30, temp_str);
  
  
      // Draw band ID
//      sprintf(temp_str, "%lu", band_id_1);
//      u8g2.drawStr(50, 29, temp_str);
//      sprintf(temp_str, "%lu", band_id_2);
//      u8g2.drawStr(88, 29, temp_str);

//      sprintf(temp_str, "%u", cur_band_module);
//      u8g2.drawStr(88, 29, temp_str);


      // Draw callsign
      //sprintf(temp_str, "%s", settings["Callsign"].substr(1).c_str());
      //sprintf(temp_str, "%s", settings["Grid"].substr(1).c_str());
  //    sprintf(temp_str, "%s", settings["Power"].substr(1).c_str());
  //    u8g2.drawStr(26, 30, temp_str);
      
    
      // Draw TX lock
  //    if(tx_lock)
  //    {
  //      u8g2.drawXBM(120, 8, 8, 8, lock_bits);
  //    }
    }
    else if (display_mode == DisplayMode::Setting) // Draw settings menu
    {
      // Draw setting name
      u8g2.setFont(u8g2_font_6x10_mr);
      sprintf(temp_str, "%s", cur_setting_label.c_str());
      u8g2.drawStr(64 - u8g2.getStrWidth(temp_str) / 2, 10, temp_str);
  
      //const char* setting_val = cur_setting_str.c_str();
  
  //    switch(cur_setting_type)
  //    {
  //    case SettingType::Uint:
  //      sprintf(temp_str, "%lu", cur_setting_uint);
  //      break;
  //    case SettingType::Int:
  //      sprintf(temp_str, "%l", cur_setting_int);
  //      break;
  //    case SettingType::Str:
  ////      if(cur_setting_selected > 10)
  ////      {
  ////        setting_val += cur_setting_selected - 10;
  ////      }
  //      //sprintf(temp_str, "%s", cur_setting_str.c_str());
  //      sprintf(temp_str, "%s", settings[cur_setting].c_str());
  ////      SerialUSB.print("\v");
  ////      SerialUSB.println(temp_str);
  //      //sprintf(temp_str, "%s", setting_val);
  //      break;
  //    }
  
      uint8_t str_x;
  
      switch(settings[cur_setting_key].second[0])
      {
      case 'U':
        sprintf(temp_str, "%lu", cur_setting_uint);
        str_x = 61 - (num_digits(cur_setting_uint) * SETTING_FONT_WIDTH) + ((cur_setting_selected + 1) * SETTING_FONT_WIDTH);
        break;
      case 'I':
        sprintf(temp_str, "%l", cur_setting_int);
        str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
        break;
      case 'B':
        if(cur_setting_bool)
        {
          sprintf(temp_str, "On");
          str_x = 61;
        }
        else
        {
          sprintf(temp_str, "Off");
          str_x = 61;
        }
        break;
      case 'S':
  //      if(cur_setting_selected > 10)
  //      {
  //        setting_val += cur_setting_selected - 10;
  //      }
        sprintf(temp_str, "%s<", cur_setting_str.c_str());
        str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
        //sprintf(temp_str, "%s", settings[cur_setting].c_str());
  //      SerialUSB.print("\v");
  //      SerialUSB.println(temp_str);
        //sprintf(temp_str, "%s", setting_val);
        break;
      }
  
      cur_setting_len = strlen(temp_str) - 1;
  
      // Put active digit/char at X pos 61
  //    uint8_t str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
  //    uint8_t str_x = 61 - ((cur_setting_selected - 1 - cur_setting_len) * SETTING_FONT_WIDTH);
      
      //uint8_t str_x = (cur_setting_selected * SETTING_FONT_WIDTH > 60 ? 0 : 60 - cur_setting_selected * SETTING_FONT_WIDTH);
      //u8g2.setDrawColor(0);
      u8g2.drawStr(str_x, 20, temp_str);
      //u8g2.setDrawColor(1);
  
      cur_setting_char = cur_setting_str[cur_setting_selected];
  
      // Find char in allowable list
  //    std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
  //    if(pos != std::string::npos)
  //    {
  //      cur_setting_index = pos;
  //    }
  
      // debugging stuff
//      sprintf(temp_str, "%d", cur_setting_selected);
//      u8g2.drawStr(0, 10, temp_str);
  //    sprintf(temp_str, "%c", cur_setting_char);
  //    u8g2.drawStr(20, 10, temp_str);
  //    sprintf(temp_str, "%d", cur_setting_index);
  //    u8g2.drawStr(90, 10, temp_str);
  //    sprintf(temp_str, "%d", cur_setting_type);
  //    u8g2.drawStr(110, 10, temp_str);
      
  
      // Underline the current setting selection
      u8g2.drawLine(60, 21, 66, 21);
    }
    else // Draw buffer menu
    {
      uint8_t str_x;
  
      // Draw buffer name
      u8g2.setFont(u8g2_font_6x10_mr);
      sprintf(temp_str, "Buffer %d", cur_edit_buffer);
      u8g2.drawStr(64 - u8g2.getStrWidth(temp_str) / 2, 10, temp_str);
      
      sprintf(temp_str, "%s<", cur_setting_str.c_str());
      str_x = 61 - ((cur_setting_selected) * SETTING_FONT_WIDTH);
      cur_setting_len = strlen(temp_str) - 1;
      u8g2.drawStr(str_x, 20, temp_str);
  
      // Underline the current setting selection
      u8g2.drawLine(60, 21, 66, 21);
  
      // debugging stuff
  //    sprintf(temp_str, "%d", cur_edit_buffer);
  //    u8g2.drawStr(0, 10, temp_str);
      
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
    else if(display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
    {
      if(ins_del_mode)
      {
        sprintf(menu_1, "%s", "Ins");
        sprintf(menu_2, "%s", "Del");
      }
      else
      {
        if (cur_setting_type == SettingType::Str)
        {
          sprintf(menu_1, "%s", "Ins/Del");
        }
        else
        {
          sprintf(menu_1, "%s", "");
        }
        sprintf(menu_2, "%s", "OK");
      }
      menu_1_x = 6;
      menu_2_x = 80;
      u8g2.setFont(u8g2_font_6x10_mf);
      u8g2.drawStr(menu_1_x, 29, menu_1);
      u8g2.drawStr(menu_2_x, 29, menu_2);
  
      // Back icon
      u8g2.setFont(u8g2_font_m2icon_9_tf);
      u8g2.drawGlyph(118, 30, 0x0061);
    }
    else // Show the current buffer
    {
      // Draw buffer contents if transmitting
      if(cur_state != TxState::Idle)
      {
        char buffer_str[81];
        //std::string wspr_buffer;
//        yield();
        
        switch(cur_config.mode)
        {
        case Mode::DFCW3:
        case Mode::DFCW6:
        case Mode::DFCW10:
        case Mode::DFCW120:
        case Mode::QRSS3:
        case Mode::QRSS6:
        case Mode::QRSS10:
        case Mode::QRSS120:
        case Mode::CW:
        case Mode::HELL:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%d:%s<", cur_config.buffer, msg_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 11;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          break;
          
        case Mode::WSPR:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%s<", wspr_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Draw TX progress bar
            yield();
            uint8_t tx_progress = (cur_symbol * 128 / WSPR_SYMBOL_COUNT);
            u8g2.drawLine(0, 31, tx_progress, 31);
          }
          break;
          
        case Mode::JT65:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%d:%s<", cur_config.buffer, msg_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Draw TX progress bar
            yield();
            uint8_t tx_progress = (cur_symbol * 128 / JT65_SYMBOL_COUNT);
            u8g2.drawLine(0, 31, tx_progress, 31);
          }
          break;
          
        case Mode::JT9:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%d:%s<", cur_config.buffer, msg_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Draw TX progress bar
            yield();
            uint8_t tx_progress = (cur_symbol * 128 / JT9_SYMBOL_COUNT);
            u8g2.drawLine(0, 31, tx_progress, 31);
          }
          break;
          
        case Mode::JT4:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%d:%s<", cur_config.buffer, msg_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Draw TX progress bar
            yield();
            uint8_t tx_progress = (cur_symbol * 128 / JT4_SYMBOL_COUNT);
            u8g2.drawLine(0, 31, tx_progress, 31);
          }
          break;
          
        case Mode::FT8:
          if(cur_state == TxState::CWID or cur_state == TxState::IDDelay)
          {
            sprintf(buffer_str, "CWID:%s<", cur_config.callsign);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Underline current TX char
            uint8_t line_x = (morse.cur_char * 6) + 29;
            u8g2.drawLine(line_x, 31, line_x + 6, 31);
          }
          else
          {
            sprintf(buffer_str, "%d:%s<", cur_config.buffer, msg_buffer);
            yield();
            u8g2.drawStr(0, 30, buffer_str);

            // Draw TX progress bar
            yield();
            uint8_t tx_progress = (cur_symbol * 128 / FT8_SYMBOL_COUNT);
            u8g2.drawLine(0, 31, tx_progress, 31);
            yield();
          }
          break;
        }
  
//        yield();
//        u8g2.drawStr(0, 30, buffer_str);
      }
      else // otherwise show TX enb/dis and current band module choice
      {
        if(tx_enable)
        {
          u8g2.setDrawColor(0);
          u8g2.drawStr(0, 29, "TX Dis");
          u8g2.setDrawColor(1); 
          sprintf(temp_str, "%1u: %s", cur_config.buffer, next_tx_time);
          u8g2.drawStr(45, 29, temp_str);
        }
        else
        {
          u8g2.setDrawColor(0);
          u8g2.drawStr(0, 29, "TX Enb");
          u8g2.setDrawColor(1);

          u8g2.setDrawColor(0);
          sprintf(temp_str, "%s", band_name.c_str());
          u8g2.drawStr(60, 29, temp_str);
          u8g2.setDrawColor(1);

//          if(cur_band_module == 0)
//          {
//            u8g2.setDrawColor(0);
//            u8g2.drawStr(60, 29, "Mod 1");
//            u8g2.setDrawColor(1);
//          }
//          else if(cur_band_module == 1)
//          {
//            u8g2.setDrawColor(0);
//            u8g2.drawStr(60, 29, "Mod 2");
//            u8g2.setDrawColor(1);
//          }
        }
      }
      yield();
      
      // Menu icon
      //yield();
      if(cur_state == TxState::Idle)
      {
        u8g2.setFont(u8g2_font_m2icon_9_tf);
        u8g2.drawGlyph(121, 30, 0x0042);
      }
      else
      {
        u8g2.setFont(u8g2_font_m2icon_9_tf);
        u8g2.drawGlyph(121, 30, 0x0043);
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
  
//    yield();
    if(disable_display_loop)
    {
      u8g2.clearBuffer();
    }
    else
    {
      u8g2.sendBuffer();          // transfer internal memory to the display
    }
    //yield();
  }
  yield();
}

void pollButtons()
{
  // Read buttons
  // ------------
  yield();
  if(!disable_display_loop)
  {
    // Handle up button
    if (digitalRead(BTN_UP) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_UP) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
  
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              if (cur_setting_uint + power_10(cur_setting_selected) < UINT64_MAX)
              {
                cur_setting_uint += power_10(cur_setting_selected);
              }
              break;
            case SettingType::Int:
              ++cur_setting_int;
              break;
            case SettingType::Bool:
              cur_setting_bool = !cur_setting_bool;
              break;
            case SettingType::Str:
              if (cur_setting_index >= settings_str_chars.size() - 1)
              {
                cur_setting_index = 0;
              }
              else
              {
                cur_setting_index++;
              }
  
              cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
              break;
          }
        }
        else
        {
          //if(cur_state == TxState::Idle)
          if (!tx_lock)
          {
            if (cur_config.base_freq + power_10(tune_step) > upper_freq_limit)
            {
              cur_config.base_freq = upper_freq_limit;
            }
            else
            {
              cur_config.base_freq += power_10(tune_step);
            }
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one;
      }
    }
  
    // Handle down button
    if (digitalRead(BTN_DOWN) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_DOWN) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
  
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              if(cur_setting_uint >0)
              {
                if ((cur_setting_uint - power_10(cur_setting_selected)) >= 0)
                {
                  cur_setting_uint -= power_10(cur_setting_selected);
                }
                if (num_digits(cur_setting_uint) - 1 < cur_setting_selected)
                {
                  --cur_setting_selected;
                }
              }
              break;
            case SettingType::Int:
              --cur_setting_int;
              break;
            case SettingType::Bool:
              cur_setting_bool = !cur_setting_bool;
              break;
            case SettingType::Str:
              if (cur_setting_index == 0)
              {
                cur_setting_index = settings_str_chars.size() - 1;
              }
              else
              {
                cur_setting_index--;
              }
              cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
              break;
          }
        }
        else
        {
          //if(cur_state == TxState::Idle)
          if (!tx_lock)
          {
            if (cur_config.base_freq - power_10(tune_step) < lower_freq_limit)
            {
              cur_config.base_freq = lower_freq_limit;
            }
            else
            {
              cur_config.base_freq -= power_10(tune_step);
            }
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle left button
    if (digitalRead(BTN_LEFT) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_LEFT) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
          if (menu.countChildren() > 2)
          {
            if (menu.active_child == 0 && menu.countChildren() % 2 == 1)
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
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
            case SettingType::Int:
              if (cur_setting_selected < num_digits(cur_setting_uint) - 1)
              {
                ++cur_setting_selected;
              }
              break;
            case SettingType::Str:
              if (cur_setting_selected > 0)
              {
                --cur_setting_selected;
  
                // Find char in allowable list
                std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
                if (pos != std::string::npos)
                {
                  cur_setting_index = (uint8_t)pos;
                  //cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
                }
              }
              break;
          }
        }
        else
        {
          if (tune_step < 5)
          {
            tune_step++;
          }
          else
          {
            tune_step = 0;
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle right button
    if (digitalRead(BTN_RIGHT) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_RIGHT) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
          if (menu.countChildren() > 2)
          {
            if (menu.active_child == menu.countChildren() - 1)
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
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          switch (cur_setting_type)
          {
            case SettingType::Uint:
            case SettingType::Int:
              if (cur_setting_selected > 0)
              {
                --cur_setting_selected;
              }
              break;
            case SettingType::Str:
              if (cur_setting_selected < cur_setting_len - 1)
              {
                ++cur_setting_selected;
                // Find char in allowable list
                std::size_t pos = settings_str_chars.find(cur_setting_str[cur_setting_selected]);
                if (pos != std::string::npos)
                {
                  cur_setting_index = (uint8_t)pos;
                  //cur_setting_str[cur_setting_selected] = settings_str_chars[cur_setting_index];
                }
              }
              break;
              //        case SettingType::Int:
              //          --cur_setting_int;
              //          break;
              //        case SettingType::Str:
              //          if(cur_setting_selected < cur_setting_str.size())
              //          {
              //            cur_setting_selected++;
              //          }
              //          break;
          }
        }
        else
        {
          if (tune_step == 0)
          {
            tune_step = 5;
          }
          else
          {
            tune_step--;
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle display 1 button
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_DSP_1) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
          MenuType type = menu.selectChild(menu.active_child);
          if (type == MenuType::Action)
          {
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          // INS/DEL button
          if (ins_del_mode)
          {
            // Insert
            cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
            cur_setting_index = settings_str_chars.length() - 1;
          }
          else
          {
            ins_del_mode = true;
          }
          //        switch(cur_setting_type)
          //        {
          //        case SettingType::Str:
          //          // Loop here to see if a long or short press
          //          // Short press == INS, long press == DEL
          //          uint32_t btn_timer_end = cur_timer + 1000;
          //          bool short_press = false;
          //
          //          while(cur_timer < btn_timer_end)
          //          {
          //            if(digitalRead(BTN_DSP_1) == HIGH)
          //            {
          //              // Insert a space
          ////              cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
          ////              cur_setting_index = settings_str_chars.length() - 1;
          //              short_press = true;
          //              break;
          //            }
          //            yield();
          //          }
          //          // If we made it this far, the button was held down for 1 s,
          //          // so do an DEL
          ////          cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--));
          //          if(short_press)
          //          {
          //            cur_setting_str.insert(static_cast<std::size_t>(++cur_setting_selected), " ");
          //            cur_setting_index = settings_str_chars.length() - 1;
          //          }
          //          else
          //          {
          //            cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--));
          //            while(digitalRead(BTN_DSP_1) == LOW)
          //            {
          //              yield();
          //            }
          //          }
          //          break;
          //        }
        }
        else
        {
          if (tx_enable)
          {
            tx_enable = false;
            next_tx = UINT32_MAX;
          }
          else
          {
            tx_enable = true;
            setTxState(TxState::Idle);
            // Re-compose the buffers to reflect changes
            selectBuffer(cur_config.buffer);
            composeWSPRBuffer();
            setNextTx(0);
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle display 2 button
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_DSP_2) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
          MenuType type = menu.selectChild(menu.active_child + 1);
          if (type == MenuType::Action)
          {
            selectBuffer(cur_config.buffer);
//            composeMFSKMessage();  // logic so executed only on Sel Buf?
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Setting)
        {
          // OK button
          char temp_str[81];
          switch (cur_setting_type)
          {
            case SettingType::Uint:
              sprintf(temp_str, "U%lu", cur_setting_uint);
              settings[cur_setting_key].second = temp_str;
              break;
            case SettingType::Int:
              sprintf(temp_str, "I%l", cur_setting_int);
              settings[cur_setting_key].second = temp_str;
              break;
            case SettingType::Bool:
              if(cur_setting_bool)
              {
                settings[cur_setting_key].second = "B1";
              }
              else
              {
                settings[cur_setting_key].second = "B0";
              }
              break;
            case SettingType::Str:
              if (ins_del_mode)
              {
                // Delete
                cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--), 1);
              }
              else
              {
                sprintf(temp_str, "S%s", cur_setting_str.c_str());
                settings[cur_setting_key].second = temp_str;
              }
  
              break;
          }
  
          // Yeah, inelegant. Hopefully we can get C++17 in Arduino soon for better data structures
          if (cur_setting_key == "pa_bias")
          {
            cur_config.pa_bias = cur_setting_uint;
            setPABias(cur_setting_uint);
          }
          else if (cur_setting_key == "callsign")
          {
            strcpy(cur_config.callsign, cur_setting_str.c_str());
            composeWSPRBuffer();
            composeMFSKMessage();
            // TODO: 
          }
          else if (cur_setting_key == "grid")
          {
            strcpy(cur_config.grid, cur_setting_str.c_str());
            composeWSPRBuffer();
            composeMFSKMessage();
          }
          else if (cur_setting_key == "power")
          {
            cur_config.power = cur_setting_uint;
            composeWSPRBuffer();
            composeMFSKMessage();
          }
          else if (cur_setting_key == "tx_intv")
          {
            cur_config.tx_intv = cur_setting_uint;
          }
          else if (cur_setting_key == "wpm")
          {
            cur_config.wpm = cur_setting_uint;
          }
          else if (cur_setting_key == "cwid")
          {
            cur_config.cwid = cur_setting_bool;
          }
  
          // If we need to make any immediate setting changes to hardware
  //        if (cur_setting_key == "pa_bias")
  //        {
  //          setPABias(cur_setting_uint);
  //        }
          
  
          // Re-compose the buffers to reflect changes
//          composeWSPRBuffer();
//          composeJTBuffer(1);
//          composeMorseBuffer(1);
          selectBuffer(cur_config.buffer);
//          composeMFSKMessage();
//          selectMode(static_cast<uint8_t>(mode));
  
          // Save the config to NVM
          serializeConfig();
  
          if (!ins_del_mode)
          {
            cur_setting_selected = 0;
            display_mode = DisplayMode::Main;
            menu.selectRoot();
          }
        }
        else if (display_mode == DisplayMode::Buffer)
        {
          if (ins_del_mode)
          {
            // Delete
            cur_setting_str.erase(static_cast<std::size_t>(cur_setting_selected--), 1);
          }
          else
          {
            switch (cur_edit_buffer)
            {
              case 1:
                sprintf(cur_config.msg_buffer_1, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_1, msg_buffer_1);
                break;
              case 2:
                sprintf(cur_config.msg_buffer_2, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_2, msg_buffer_2);
                break;
              case 3:
                sprintf(cur_config.msg_buffer_3, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_3, msg_buffer_3);
                break;
              case 4:
                sprintf(cur_config.msg_buffer_4, "%s", cur_setting_str.c_str());
//                strcpy(cur_config.msg_buffer_4, msg_buffer_4);
                break;
            }
  
            cur_setting_selected = 0;
            display_mode = DisplayMode::Main;
            menu.selectRoot();
            selectBuffer(cur_config.buffer);
            serializeConfig();
//            composeMFSKMessage();
//            selectMode(static_cast<uint8_t>(mode));
          }
        }
        else
        {
          if (!tx_enable)
          {
            uint32_t new_freq, new_cw_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
            uint32_t new_lower_freq_limit, new_upper_freq_limit;
            std::string new_band_name;
            bool retune = false;
  
            // Switch bands

            // First, check if there is another higher band in this band module
            if(band_table[cur_config.band].module_index == band_table[cur_config.band + 1].module_index)
            {
              ++cur_config.band;
              new_lower_freq_limit = band_table[cur_config.band].lower_limit;
              new_upper_freq_limit = band_table[cur_config.band].upper_limit;
              new_cw_freq = band_table[cur_config.band].cw_freq;
              new_wspr_freq = band_table[cur_config.band].wspr_freq;
              new_jt65_freq = band_table[cur_config.band].jt65_freq;
              new_jt9_freq = band_table[cur_config.band].jt9_freq;
              new_band_name = band_table[cur_config.band].name;
              retune = true;
            }
            // Then, check if there is another band module to switch to
            else
            {
              if(cur_band_module == 0)
              {
                if(band_id_2 > 150)
                {
                  cur_band_module = 1;
                  digitalWrite(BAND_SW, HIGH);

                  // Set band to first in band module list
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_2)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
                      new_lower_freq_limit = band.lower_limit;
                      new_upper_freq_limit = band.upper_limit;
                      new_cw_freq = band.cw_freq;
                      new_wspr_freq = band.wspr_freq;
                      new_jt65_freq = band.jt65_freq;
                      new_jt9_freq = band.jt9_freq;
                      new_band_name = band.name;
                      retune = true;
                      break;
                    }
                  }
                }
                else
                {
                  // If not, go back to the first band on this module
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_1)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
                      new_lower_freq_limit = band.lower_limit;
                      new_upper_freq_limit = band.upper_limit;
                      new_cw_freq = band.cw_freq;
                      new_wspr_freq = band.wspr_freq;
                      new_jt65_freq = band.jt65_freq;
                      new_jt9_freq = band.jt9_freq;
                      new_band_name = band.name;
                      retune = true;
                      break;
                    }
                  }
                }
              }
              else
              {
                if(band_id_1 > 150)
                {
                  cur_band_module = 0;
                  digitalWrite(BAND_SW, LOW);

                  // Set band to first in band module list
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_1)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
                      new_lower_freq_limit = band.lower_limit;
                      new_upper_freq_limit = band.upper_limit;
                      new_cw_freq = band.cw_freq;
                      new_wspr_freq = band.wspr_freq;
                      new_jt65_freq = band.jt65_freq;
                      new_jt9_freq = band.jt9_freq;
                      new_band_name = band.name;
                      retune = true;
                      break;
                    }
                  }
                }
                else
                {
                  for(auto band : band_table)
                  {
                    if(band.module_index == band_module_index_2)
                    {
                      cur_config.band = band.index;
          
                      // Set default frequencies for band
                      new_lower_freq_limit = band.lower_limit;
                      new_upper_freq_limit = band.upper_limit;
                      new_cw_freq = band.cw_freq;
                      new_wspr_freq = band.wspr_freq;
                      new_jt65_freq = band.jt65_freq;
                      new_jt9_freq = band.jt9_freq;
                      new_band_name = band.name;
                      retune = true;
                      break;
                    }
                  }
                }
              }
            }

            if(retune)
            {
              lower_freq_limit = new_lower_freq_limit;
              upper_freq_limit = new_upper_freq_limit;
              band_name = new_band_name;

              switch (cur_config.mode)
              {
                case Mode::DFCW3:
                case Mode::DFCW6:
                case Mode::DFCW10:
                case Mode::DFCW120:
                case Mode::QRSS3:
                case Mode::QRSS6:
                case Mode::QRSS10:
                case Mode::QRSS120:
                case Mode::CW:
                case Mode::HELL:
                  cur_config.base_freq = new_cw_freq;
                  break;
      
                case Mode::WSPR:
                  cur_config.base_freq = new_wspr_freq;
                  break;
                case Mode::JT65:
                  cur_config.base_freq = new_jt65_freq;
                  break;
                case Mode::JT9:
                case Mode::JT4:
                case Mode::FT8: // TODO
                  cur_config.base_freq = new_jt9_freq;
                  break;
              }
            }
            
//            if(cur_band_module == 0)
//            {
//              cur_band_module = 1;
//              digitalWrite(BAND_SW, HIGH);
//            }
//            else
//            {
//              cur_band_module = 0;
//              digitalWrite(BAND_SW, LOW);
//            }
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  
    // Handle menu button
    if (digitalRead(BTN_BACK) == LOW)
    {
      delay(50);   // delay to debounce
      yield();
      if (digitalRead(BTN_BACK) == LOW)
      {
        if (display_mode == DisplayMode::Menu)
        {
          if (menu.selectParent()) // if we are at root menu, exit
          {
            display_mode = DisplayMode::Main;
          }
        }
        else if (display_mode == DisplayMode::Setting || display_mode == DisplayMode::Buffer)
        {
          if (ins_del_mode)
          {
            ins_del_mode = false;
          }
          else
          {
            display_mode = DisplayMode::Menu;
          }
        }
        else
        {
          if (cur_state == TxState::Idle)
          {
            cur_setting_selected = 0;
            display_mode = DisplayMode::Menu;
          }
          else
          {
            setTxState(TxState::Idle);
            morse.reset();
            setNextTx(0);

            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_MAX_SIZE + 6];
            json_tx_doc["level"] = 0;
            json_tx_doc["text"] = "TX End";
            json_tx_doc["data"] = cur_config.base_freq;
            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0xFE, send_json);
          }
        }
        yield();
        delay(50); //delay to avoid many steps at one
      }
    }
  }
}

void selectBand()
{
  #ifdef REV_A
  static uint8_t prev_band_module_index[3] = {0, 0, 0};
  static uint32_t new_freq, new_cw_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
  static uint32_t new_lower_freq_limit, new_upper_freq_limit;

  //  constexpr uint8_t ADC0_RING_BUF_SIZE = 4;
  //  static uint16_t adc0_ring_buf[ADC0_RING_BUF_SIZE];
  //  static uint8_t adc0_ring_buf_pos = 0;
  //  uint32_t adc0_ring_buf_total = 0;

  // TODO: handle out of bounds
  //  uint16_t adc0 = analogRead(A0);
  //  yield();
  //  adc0_ring_buf[adc0_ring_buf_pos++] = adc0;
  //  if(adc0_ring_buf_pos >= ADC0_RING_BUF_SIZE)
  //  {
  //    adc0_ring_buf_pos = 0;
  //  }
  //  for(uint8_t i = 0; i <= ADC0_RING_BUF_SIZE; ++i)
  //  {
  //    if(adc0_ring_buf[i] == 0)
  //    {
  //      adc0_ring_buf_total += adc0;
  //    }
  //    else
  //    {
  //      adc0_ring_buf_total += adc0_ring_buf[i];
  //    }
  //    yield();
  //  }
  //  band_id = adc0_ring_buf_total / ADC0_RING_BUF_SIZE;

  prev_band_module_index[2] = prev_band_module_index[1];
  prev_band_module_index[1] = prev_band_module_index[0];
  prev_band_module_index[0] = cur_config.band;

  band_id = analogRead(ADC_BAND_ID);

  yield();
  band_id = (band_id * ANALOG_REF) / 4096UL;
  yield();

  for (auto band : band_table)
  {
    if (band_id < band.upper_v && band_id > band.lower_v)
    {
      if (band.index != cur_config.band)
      {
        prev_band_module_index[0] = band.index;
        //band_index = band.index;
        new_lower_freq_limit = band.lower_limit;
        new_upper_freq_limit = band.upper_limit;
        new_cw_freq = band.cw_freq;
        new_wspr_freq = band.wspr_freq;
        new_jt65_freq = band.jt65_freq;
        new_jt9_freq = band.jt9_freq;
        //        if(cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
        //        {
        //          switch(mode)
        //          {
        //          case Mode::DFCW3:
        //          case Mode::DFCW6:
        //          case Mode::DFCW10:
        //          case Mode::DFCW120:
        //          case Mode::QRSS3:
        //          case Mode::QRSS6:
        //          case Mode::QRSS10:
        //          case Mode::QRSS120:
        //          case Mode::CW:
        //          case Mode::HELL:
        //            new_freq = band.cw_freq;
        //            break;
        //
        //          case Mode::WSPR:
        //            new_freq = band.wspr_freq;
        //            break;
        //          case Mode::JT65:
        //            new_freq = band.jt65_freq;
        //            break;
        //          case Mode::JT9:
        //          case Mode::JT4:
        //            new_freq = band.jt9_freq;
        //            break;
        //          }
        //        }
      }
    }
  }
  yield();

  // Guard against ADC glitches by not changing bands until
  // three consequtive reads of the same band
  if (prev_band_module_index[0] == prev_band_module_index[1] && prev_band_module_index[1] == prev_band_module_index[2])
  {
    // If the band index is changed, change bands only when not transmitting,
    // unless band index is 0, which indicates removal of the band module
    //if(band_index != prev_band_module_index[0] && (cur_state != TxState::Idle || band_index == 0))
    //if(band_index != prev_band_module_index[0] && (!tx_lock || prev_band_module_index[0] == 0))
    if (cur_config.band != prev_band_module_index[0])
    {
      lower_freq_limit = new_lower_freq_limit;
      upper_freq_limit = new_upper_freq_limit;

      if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
      {
        switch (cur_config.mode)
        {
          case Mode::DFCW3:
          case Mode::DFCW6:
          case Mode::DFCW10:
          case Mode::DFCW120:
          case Mode::QRSS3:
          case Mode::QRSS6:
          case Mode::QRSS10:
          case Mode::QRSS120:
          case Mode::CW:
          case Mode::HELL:
            new_freq = new_cw_freq;
            break;

          case Mode::WSPR:
            new_freq = new_wspr_freq;
            break;
          case Mode::JT65:
            new_freq = new_jt65_freq;
            break;
          case Mode::JT9:
          case Mode::JT4:
          case Mode::FT8: // TODO
            new_freq = new_jt9_freq;
            break;
        }
      }

      //if(cur_state != TxState::Idle)
      if (tx_lock)
      {
        // Terminate the transmission if module is removed while transmitting
        if (prev_band_module_index[0] == 0)
        {
          setTxState(TxState::Idle);
          tx_enable = false;
          return;
        }
      }
      else
      {
        cur_config.base_freq = new_freq;
      }
    }
    // Then change band index
    cur_config.band = prev_band_module_index[0];

    //    if(band_index == 0)
    //    {
    //      tx_lock = true;
    //      cur_config.base_freq = 0;
    //    }

    //    if(!tx_lock)
    //    {
    //      cur_config.base_freq = new_freq;
    //    }
  }
  else if (prev_band_module_index[2] == 0 && prev_band_module_index[1] != 0 && prev_band_module_index[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }

  //  if(band_index == 0 && prev_band_module_index[1] == 0 && prev_band_module_index[0] == 0)
  //  {
  //    tx_lock = true;
  //    cur_config.base_freq = 0;
  //  }
  //  else
  //  {
  //    if(tx_lock == true && cur_state == TxState::Idle)
  //    {
  //      //tx_lock = false;
  //    }
  //  }


  //  if(prev_band_module_index[1] == 0 && prev_band_module_index[0] != 0 && band_index != 0)
  //  {
  //    tx_lock = false;
  //  }
  //  else if(prev_band_module_index[0] != prev_band_module_index[1] && prev_band_module_index[0] == band_index)
  //  {
  //    return;
  //  }

  //  if(!tx_lock)
  //  {
  //    cur_config.base_freq = new_freq;
  //  }
  //  else
  //  {
  //    if(band_index == 0)
  //    {
  //      cur_config.base_freq = new_freq;
  //    }
  //  }
  yield();
  #endif
  #ifdef REV_B
  static uint8_t prev_band_module_index_1[3] = {0, 0, 0};
  static uint8_t prev_band_module_index_2[3] = {0, 0, 0};
  static uint32_t new_freq, new_cw_freq, new_wspr_freq, new_jt65_freq, new_jt9_freq;
  static uint32_t new_lower_freq_limit, new_upper_freq_limit;
  static std::string new_band_name;

  if(select_band_reset)
  {
    prev_band_module_index_1[2] = 0;
    prev_band_module_index_1[1] = 0;
    prev_band_module_index_1[0] = 0;
  
    prev_band_module_index_2[2] = 0;
    prev_band_module_index_2[1] = 0;
    prev_band_module_index_2[0] = 0;

    select_band_reset = false;
  }
  else
  {
    prev_band_module_index_1[2] = prev_band_module_index_1[1];
    prev_band_module_index_1[1] = prev_band_module_index_1[0];
    prev_band_module_index_1[0] = band_module_index_1;
  
    prev_band_module_index_2[2] = prev_band_module_index_2[1];
    prev_band_module_index_2[1] = prev_band_module_index_2[0];
    prev_band_module_index_2[0] = band_module_index_2;
  }

  band_id_1 = analogRead(ADC_BAND_ID_1);
  yield();
  band_id_2 = analogRead(ADC_BAND_ID_2);
  yield();
  
//  if(cur_band_module == 0)
//  {
//    band_id = analogRead(ADC_BAND_ID_1);
//  }
//  else if(cur_band_module == 1)
//  {
//    band_id = analogRead(ADC_BAND_ID_2);
//  }

  band_id_1 = (band_id_1 * ANALOG_REF) / 4096UL;
  band_id_2 = (band_id_2 * ANALOG_REF) / 4096UL;
  yield();

//  for (auto band_module : band_module_table)
//  {
//    yield();
//    band_id = cur_band_module == 0 ? band_id_1 : band_id_2;
//    if (band_id < band_module.upper_v && band_id > band_module.lower_v)
//    {
//      // Module change
//      if (band_module.index != (cur_band_module == 0 ? band_module_index_1 : band_module_index_2))
//      {
//        if(cur_band_module == 0)
//        {
//          prev_band_module_index_1[0] = band_module.index;
//        }
//        else
//        {
//          prev_band_module_index_2[0] = band_module.index;
//        }
//        
//        // Set band to first in band module list
//        for(auto band : band_table)
//        {
//          if(band.module_index == band_module.index)
//          {
//            band_index = band.index;
//
//            // Set default frequencies for band
//            new_lower_freq_limit = band.lower_limit;
//            new_upper_freq_limit = band.upper_limit;
//            new_cw_freq = band.cw_freq;
//            new_wspr_freq = band.wspr_freq;
//            new_jt65_freq = band.jt65_freq;
//            new_jt9_freq = band.jt9_freq;
//            new_band_name = band.name;
//            break;
//          }
//        }
//      }
//    }
//  }
  for (auto band_module : band_module_table)
  {
    yield();
    if (band_id_1 < band_module.upper_v && band_id_1 > band_module.lower_v)
    {
      // Module change
      if (band_module.index != band_module_index_1)
      {
        prev_band_module_index_1[0] = band_module.index;
        
        // Set band to first in band module list
        if(cur_band_module == 0)
        {
          for(auto band : band_table)
          {
            if(band.module_index == band_module.index)
            {
              cur_config.band = band.index;
  
              // Set default frequencies for band
              new_lower_freq_limit = band.lower_limit;
              new_upper_freq_limit = band.upper_limit;
              new_cw_freq = band.cw_freq;
              new_wspr_freq = band.wspr_freq;
              new_jt65_freq = band.jt65_freq;
              new_jt9_freq = band.jt9_freq;
              new_band_name = band.name;
              break;
            }
          }
        }
      }
    }
  }
  yield();
  for (auto band_module : band_module_table)
  {
    yield();
    if (band_id_2 < band_module.upper_v && band_id_2 > band_module.lower_v)
    {
      prev_band_module_index_2[0] = band_module.index;
      
      // Module change
      if (band_module.index != band_module_index_2)
      {
//        prev_band_module_index_2[0] = band_module.index;
        
        // Set band to first in band module list
        if(cur_band_module == 1)
        {
          for(auto band : band_table)
          {
            if(band.module_index == band_module.index)
            {
              cur_config.band = band.index;
  
              // Set default frequencies for band
              new_lower_freq_limit = band.lower_limit;
              new_upper_freq_limit = band.upper_limit;
              new_cw_freq = band.cw_freq;
              new_wspr_freq = band.wspr_freq;
              new_jt65_freq = band.jt65_freq;
              new_jt9_freq = band.jt9_freq;
              new_band_name = band.name;
              break;
            }
          }
        }
      }
    }
  }
  yield();

  // Guard against ADC glitches by not changing bands until
  // three consequtive reads of the same band
  if (prev_band_module_index_1[0] == prev_band_module_index_1[1] && prev_band_module_index_1[1] == prev_band_module_index_1[2])
  {
    if(cur_band_module == 0)
    {
      // If the band index is changed, change bands only when not transmitting,
      // unless band index is 0, which indicates removal of the band module
      if (band_module_index_1 != prev_band_module_index_1[0])
      {
        lower_freq_limit = new_lower_freq_limit;
        upper_freq_limit = new_upper_freq_limit;
        band_name = new_band_name;
  
        if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
        {
          switch (cur_config.mode)
          {
            case Mode::DFCW3:
            case Mode::DFCW6:
            case Mode::DFCW10:
            case Mode::DFCW120:
            case Mode::QRSS3:
            case Mode::QRSS6:
            case Mode::QRSS10:
            case Mode::QRSS120:
            case Mode::CW:
            case Mode::HELL:
              new_freq = new_cw_freq;
              break;
  
            case Mode::WSPR:
              new_freq = new_wspr_freq;
              break;
            case Mode::JT65:
              new_freq = new_jt65_freq;
              break;
            case Mode::JT9:
            case Mode::JT4:
            case Mode::FT8: // TODO
              new_freq = new_jt9_freq;
              break;
          }
        }
  
        //if(cur_state != TxState::Idle)
        if (tx_lock)
        {
          // Terminate the transmission if module is removed while transmitting
          if (prev_band_module_index_1[0] == 0)
          {
            setTxState(TxState::Idle);
            tx_enable = false;
            return;
          }
        }
        else
        {
          cur_config.base_freq = new_freq;
        }
      }
    }
    // Then change band index
    band_module_index_1 = prev_band_module_index_1[0];
  }
  else if (prev_band_module_index_1[2] == 0 && prev_band_module_index_1[1] != 0 && prev_band_module_index_1[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }
  
  if (prev_band_module_index_2[0] == prev_band_module_index_2[1] && prev_band_module_index_2[1] == prev_band_module_index_2[2])
  {
    if(cur_band_module == 1)
    {
      // If the band index is changed, change bands only when not transmitting,
      // unless band index is 0, which indicates removal of the band module
      if (band_module_index_2 != prev_band_module_index_2[0])
      {
        lower_freq_limit = new_lower_freq_limit;
        upper_freq_limit = new_upper_freq_limit;
        band_name = new_band_name;
  
        if (cur_config.base_freq > upper_freq_limit || cur_config.base_freq < lower_freq_limit)
        {
          switch (cur_config.mode)
          {
            case Mode::DFCW3:
            case Mode::DFCW6:
            case Mode::DFCW10:
            case Mode::DFCW120:
            case Mode::QRSS3:
            case Mode::QRSS6:
            case Mode::QRSS10:
            case Mode::QRSS120:
            case Mode::CW:
            case Mode::HELL:
              new_freq = new_cw_freq;
              break;
  
            case Mode::WSPR:
              new_freq = new_wspr_freq;
              break;
            case Mode::JT65:
              new_freq = new_jt65_freq;
              break;
            case Mode::JT9:
            case Mode::JT4:
            case Mode::FT8: // TODO
              new_freq = new_jt9_freq;
              break;
          }
        }
  
        //if(cur_state != TxState::Idle)
        if (tx_lock)
        {
          // Terminate the transmission if module is removed while transmitting
          if (prev_band_module_index_2[0] == 0)
          {
            setTxState(TxState::Idle);
            tx_enable = false;
            return;
          }
        }
        else
        {
          cur_config.base_freq = new_freq;
        }
      }
    }
    // Then change band index
    band_module_index_2 = prev_band_module_index_2[0];
  }
  else if (prev_band_module_index_2[2] == 0 && prev_band_module_index_2[1] != 0 && prev_band_module_index_2[0] != 0)
  {
    tx_lock = false;
  }
  else
  {
    return;
  }

  #endif
}

void setTxState(TxState state)
{
  uint8_t key_val;
  switch (state)
  {
    case TxState::Idle:
      tx_lock = false;
      morse.reset();
      morse.tx_enable = false;
      #ifdef REV_A
      digitalWrite(TX_KEY, LOW);
      #endif
      #ifdef REV_B
      if(cur_config.mode == Mode::DFCW3 || cur_config.mode == Mode::DFCW6 || cur_config.mode == Mode::DFCW10 ||
         cur_config.mode == Mode::DFCW120 || cur_config.mode == Mode::QRSS3 || cur_config.mode == Mode::QRSS6 || 
         cur_config.mode == Mode::QRSS10 || cur_config.mode == Mode::QRSS120)
      {
        digitalWrite(TX_KEY, HIGH);
        setPABias(cur_config.pa_bias + 800);
      }
      else
      {
        digitalWrite(TX_KEY, LOW);
      }
      digitalWrite(TX_LED, LOW);
      #endif
      yield();
      si5351.output_enable(SI5351_CLK0, 0);
//      frequency = (cur_config.base_freq * 100ULL);
//      change_freq = true;
//      next_state = prev_state;
      prev_state = cur_state;
      cur_state = state;
//      key_val = digitalRead(TX_KEY);
//      SerialUSB.write('\v');
//      SerialUSB.println(key_val);
      break;
    case TxState::MFSK:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      tx_lock = true;
      cur_symbol = 0;
      frequency = (cur_config.base_freq * 100ULL) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
      change_freq = true;
      digitalWrite(TX_KEY, HIGH);
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, HIGH);
      #endif
      yield();
      si5351.output_enable(SI5351_CLK0, 1);
      //    setPABias(PA_BIAS_FULL);
      next_state = TxState::MFSK;
      prev_state = cur_state;
      cur_state = state;
      next_event = cur_timer + cur_symbol_time;
      break;
    case TxState::CW:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      #endif
//      morse.output_pin = TX_KEY;
      morse.dfcw_mode = false;
      morse.setWPM(cur_config.wpm);
      next_state = TxState::CW;
      prev_state = cur_state;
      cur_state = state;
      selectBuffer(cur_config.buffer);
      morse.send(msg_buffer);
      break;
    case TxState::IDDelay:
//      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      digitalWrite(TX_KEY, LOW);
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, LOW);
      #endif
      si5351.output_enable(SI5351_CLK0, 0);
//      tx_lock = true;
//      frequency = (cur_config.base_freq * 100ULL);
//      change_freq = true;
//      morse.output_pin = TX_KEY;
//      morse.setWPM(wpm);
//      next_state = prev_state;
//      prev_state = cur_state;
      cur_state = state;
      cwid_start = cur_timer + cwid_delay;
      break;
    case TxState::CWID:
//      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      #endif
//      morse.output_pin = TX_KEY;
      morse.dfcw_mode = false;
//      morse.setWPM(wpm);
      morse.setWPM(cwid_wpm);
//      next_state = prev_state;
//      prev_state = cur_state;
      cur_state = state;
      strcpy(cur_callsign, cur_config.callsign);
      morse.send(cur_callsign);
      break;
    case TxState::DFCW:
      sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"TX Start\"}");;
      si5351.output_enable(SI5351_CLK0, 1);
      tx_lock = true;
      digitalWrite(TX_KEY, HIGH);
      morse.reset();
      morse.tx_enable = true;
      #ifdef REV_B
      setPABias(cur_config.pa_bias);
      digitalWrite(TX_LED, HIGH);
      #endif
      frequency = (cur_config.base_freq * 100ULL);
      change_freq = true;
//      morse.output_pin = 0;
      morse.dfcw_mode = true;
      morse.setWPM(cur_config.wpm);
      next_state = TxState::DFCW;
      prev_state = cur_state;
      cur_state = state;
      morse.preamble_enable = true;
      selectBuffer(cur_config.buffer);
      morse.send(msg_buffer);
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
  uint16_t sec_to_add;
//  uint32_t t = rtc.getEpoch();
  uint8_t ten_min_delay, one_min_delay;

  switch (cur_config.mode)
  {
    case Mode::DFCW3:
    case Mode::DFCW6:
    case Mode::DFCW10:
    case Mode::DFCW120:
    case Mode::QRSS3:
    case Mode::QRSS6:
    case Mode::QRSS10:
    case Mode::QRSS120:
    case Mode::HELL:
      one_min_delay = (60 - rtc.getSeconds());
      //    if(one_min_delay == )
      //    {
//      ten_min_delay = 9 - ((rtc.getMinutes() % 10) == 0 ? 0 : (rtc.getMinutes() % 10));
      ten_min_delay = 9 - (rtc.getMinutes() % 10);
      //    }
      //    else
      //    {
      //      ten_min_delay = 9 - ((rtc.getMinutes() % 10) ? (rtc.getMinutes() % 10) : 10);
      //    }
      sec_to_add = (one_min_delay) + (ten_min_delay * 60) + ((minutes / 10) * 60);
      //    sec_to_add = (one_min_delay) + (ten_min_delay * 60);
      break;

    case Mode::CW:
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;

    case Mode::WSPR:
      sec_to_add = (60 - rtc.getSeconds()) + (rtc.getMinutes() % 2 ? 0 : 60) 
        + (minutes % 2 ? ((minutes * 60) +   60) : (minutes * 60));
      break;

    case Mode::JT65:
    case Mode::JT9:
    case Mode::JT4:
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;

    case Mode::FT8:
//      sec_to_add = (60 - rtc.getSeconds() - 1) + (minutes * 60);
      sec_to_add = (60 - rtc.getSeconds()) + (minutes * 60);
      break;
  }

  //uint16_t sec_to_add = (60 - rtc.getSeconds()) + (rtc.getMinutes() % 2 ? 0 : 60) + (minutes * 60);
//  uint32_t t = rtc.getEpoch();
  next_tx = rtc.getEpoch() + sec_to_add;
  yield();

  // Build next TX time string
  const time_t ntx = static_cast<time_t>(next_tx);
  struct tm * n_tx = gmtime(&ntx);
  sprintf(next_tx_time, "%02u:%02u:%02u", n_tx->tm_hour, n_tx->tm_min, n_tx->tm_sec);
}

void processSyncMessage()
{
  uint32_t pctime;
  constexpr uint32_t DEFAULT_TIME = 946684800; // 1 Jan 2000

//  yield();

  if (SerialUSB.find(TIME_HEADER))
  {
    // check the integer is a valid time (greater than 1 Jan 2000)
    pctime = SerialUSB.parseInt();
    //yield();
    if (pctime >= DEFAULT_TIME)
    {
      time_sync_expire = pctime + TIME_EXPIRE;
      next_time_sync = pctime + TIME_SYNC_INTERVAL;
      rtc.setEpoch(pctime); // Sync RTC to the time received on the serial port
      if (initial_time_sync == 0)
      {
        initial_time_sync = pctime;
      }
    }
  }
  yield();
}

bool isTimeValid()
{
  if (rtc.getEpoch() < time_sync_expire)
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
//  static bool time_sync_request = false;

  // Check to see if we need to sync
  if (rtc.getEpoch() > next_time_sync)
  {
    sendSerialPacket(0, "");
//    SerialUSB.write(TIME_REQUEST);
    yield();
    time_sync_request = true;
    next_time_sync = rtc.getEpoch() + TIME_SYNC_RETRY_RATE;
  }

  // Process time sync message if data is available on the serial port
//  if (time_sync_request)
//  {
//    if (SerialUSB.available())
//    {
////      processSyncMessage();
//      time_sync_request = false;
//    }
//  }
  yield();

  // Indicate time sync status
  // This LED has inverted logic
  if (isTimeValid())
  {
    digitalWrite(SYNC_LED, LOW);
  }
  else
  {
    digitalWrite(SYNC_LED, HIGH);
  }
  yield();
}

void processSerialIn()
{
  constexpr uint32_t DEFAULT_TIME = 946684800; // 1 Jan 2000

  uint32_t pctime;
//  char serial_packet[JSON_MAX_SIZE + 6];
  char payload[JSON_MAX_SIZE];
//  StaticJsonDocument<JSON_MAX_SIZE> json_rx_doc;
//  StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
  char send_json[JSON_MAX_SIZE + 6];

  if(SerialUSB)
  {
    if (SerialUSB.available())
    {
//      char serial_packet[JSON_MAX_SIZE + 6];
//      char payload[JSON_MAX_SIZE];
//      StaticJsonDocument<JSON_MAX_SIZE> json_rx_doc;
//      StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
//      char send_json[JSON_MAX_SIZE + 6];
  
      if (SerialUSB.read() == PACKET_ID)
      {
  //      digitalWrite(LED_BUILTIN, HIGH);
        
        // Get message type
        uint8_t message_type = SerialUSB.read();
    
        // Determine payload length
        uint16_t payload_len = SerialUSB.read();
        payload_len <<= 8;
        payload_len += SerialUSB.read();
    
        // Get the payload
        if(payload_len > 0)
        {
          SerialUSB.readBytes(payload, payload_len);
        }
    
        // Make sure the packet is terminated correctly
        if(SerialUSB.read() == '\n')
        {
        }
        else return;
    
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(json_rx_doc, payload);
    
        if(!error)
        {
        }
        else return;
    
        // Handle message
        switch(message_type)
        {
        case 0x01:
          pctime = json_rx_doc["timestamp"];
//          sendSerialPacket(0xFE, "{\"text\":\"hello\"}");
          if (pctime >= DEFAULT_TIME)
          {
            time_sync_expire = pctime + TIME_EXPIRE;
            next_time_sync = pctime + TIME_SYNC_INTERVAL;
            rtc.setEpoch(pctime); // Sync RTC to the time received on the serial port
            if (initial_time_sync == 0)
            {
              initial_time_sync = pctime;
            }
            time_sync_request = false;
          }
          break;
        case 0x02:
          if(json_rx_doc["set"] == true)
          {
            // Only make changes if not transmitting
            if (cur_state == TxState::Idle)
            {
              if(json_rx_doc["config"] == "base_freq")
              {
                uint64_t temp_freq = json_rx_doc["value"];
                if(temp_freq >= lower_freq_limit && temp_freq <= upper_freq_limit)
                {
                  cur_config.base_freq = temp_freq;
                }
              }
              else if(json_rx_doc["config"] == "mode")
              {
                // TODO: bounds checking
                uint8_t temp_mode = json_rx_doc["value"];
                selectMode(temp_mode);
//                serializeConfig();
              }
              else if(json_rx_doc["config"] == "band")
              {
                uint8_t temp_band_index = json_rx_doc["value"];
                
                // Make sure the requested band is available
                if(band_table[temp_band_index].module_index == band_module_index_1 ||
                  band_table[temp_band_index].module_index == band_module_index_2)
                {
                  if(band_table[temp_band_index].module_index == band_module_index_1)
                  {
                    cur_band_module = 0;
                    digitalWrite(BAND_SW, LOW);
                  }
                  else
                  {
                    cur_band_module = 1;
                    digitalWrite(BAND_SW, HIGH);
                  }
                  cur_config.band = temp_band_index;
//                  band_index = cur_config.band;
                  lower_freq_limit = band_table[cur_config.band].lower_limit;
                  upper_freq_limit = band_table[cur_config.band].upper_limit;
                  band_name = band_table[cur_config.band].name;
    
                  switch (cur_config.mode)
                  {
                    case Mode::DFCW3:
                    case Mode::DFCW6:
                    case Mode::DFCW10:
                    case Mode::DFCW120:
                    case Mode::QRSS3:
                    case Mode::QRSS6:
                    case Mode::QRSS10:
                    case Mode::QRSS120:
                    case Mode::CW:
                    case Mode::HELL:
                      cur_config.base_freq = band_table[cur_config.band].cw_freq;
                      break;
          
                    case Mode::WSPR:
                      cur_config.base_freq = band_table[cur_config.band].wspr_freq;
                      break;
                    case Mode::JT65:
                      cur_config.base_freq = band_table[cur_config.band].jt65_freq;
                      break;
                    case Mode::JT9:
                    case Mode::JT4:
                    case Mode::FT8: // TODO
                      cur_config.base_freq = band_table[cur_config.band].jt9_freq;
                      break;
                  }
                }
//                serializeConfig();
              }
              else if(json_rx_doc["config"] == "wpm")
              {
                char temp_str[40];
                uint8_t temp_wpm = json_rx_doc["value"];
                cur_config.wpm = temp_wpm;
                sprintf(temp_str, "U%lu", cur_config.wpm);
                settings["wpm"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "tx_intv")
              {
                char temp_str[40];
                uint8_t temp_tx_intv = json_rx_doc["value"];
                cur_config.tx_intv = temp_tx_intv;
                sprintf(temp_str, "U%lu", cur_config.tx_intv);
                settings["tx_intv"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "dfcw_offset")
              {
                char temp_str[40];
                uint8_t temp_dfcw_offset = json_rx_doc["value"];
                cur_config.dfcw_offset = temp_dfcw_offset;
                sprintf(temp_str, "U%lu", cur_config.dfcw_offset);
                settings["dfcw_offset"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "buffer")
              {
                char temp_str[40];
                uint8_t temp_buffer = json_rx_doc["value"];
                selectBuffer(temp_buffer);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "callsign")
              {
                char temp_str[40];
                strcpy(cur_config.callsign, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.callsign);
                settings["callsign"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "grid")
              {
                char temp_str[40];
                strcpy(cur_config.grid, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.grid);
                settings["grid"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "power")
              {
                char temp_str[40];
                uint8_t temp_power = json_rx_doc["value"];
                cur_config.power = temp_power;
                sprintf(temp_str, "U%lu", cur_config.power);
                settings["power"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "pa_bias")
              {
                char temp_str[40];
                uint16_t temp_pa_bias = json_rx_doc["value"];
                cur_config.pa_bias = temp_pa_bias;
                sprintf(temp_str, "U%lu", cur_config.pa_bias);
                settings["pa_bias"].second = std::string(temp_str);
                setPABias(cur_config.pa_bias);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "cwid")
              {
                char temp_str[40];
                bool temp_cwid = json_rx_doc["value"];
                cur_config.cwid = temp_cwid;
                if(temp_cwid)
                {
                  sprintf(temp_str, "B1");
                }
                else
                {
                  sprintf(temp_str, "B0");
                }
                settings["cwid"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_1")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_1, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_1);
                settings["msg_buffer_1"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_2")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_2, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_2);
                settings["msg_buffer_2"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_3")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_3, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_3);
                settings["msg_buffer_3"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "msg_buffer_4")
              {
                char temp_str[42];
                strcpy(cur_config.msg_buffer_4, json_rx_doc["value"]);
                sprintf(temp_str, "S%s", cur_config.msg_buffer_4);
                settings["msg_buffer_4"].second = std::string(temp_str);
                serializeConfig();
              }
              else if(json_rx_doc["config"] == "si5351_int_corr")
              {
                char temp_str[40];
                int32_t temp_si5351_int_corr = json_rx_doc["value"];
                cur_config.si5351_int_corr = temp_si5351_int_corr;
                sprintf(temp_str, "I%li", cur_config.si5351_int_corr);
//                settings["si5351_int_corr"].second = std::string(temp_str);
                serializeConfig();
              }
            }
          }
          else if(json_rx_doc["get"] == true)
          {
            if(json_rx_doc["config"] == "base_freq")
            {
              json_tx_doc["config"] = "base_freq";
              json_tx_doc["value"] = cur_config.base_freq;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "mode")
            {
              json_tx_doc["config"] = "mode";
              json_tx_doc["value"] = static_cast<uint8_t>(cur_config.mode);
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "band")
            {
              json_tx_doc["config"] = "band";
              json_tx_doc["value"] = static_cast<uint8_t>(cur_config.band);
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "wpm")
            {
              json_tx_doc["config"] = "wpm";
              json_tx_doc["value"] = cur_config.wpm;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "tx_intv")
            {
              json_tx_doc["config"] = "tx_intv";
              json_tx_doc["value"] = cur_config.tx_intv;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "dfcw_offset")
            {
              json_tx_doc["config"] = "dfcw_offset";
              json_tx_doc["value"] = cur_config.dfcw_offset;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "buffer")
            {
              json_tx_doc["config"] = "buffer";
              json_tx_doc["value"] = cur_config.buffer;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "callsign")
            {
              json_tx_doc["config"] = "callsign";
              json_tx_doc["value"] = cur_config.callsign;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "grid")
            {
              json_tx_doc["config"] = "grid";
              json_tx_doc["value"] = cur_config.grid;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "power")
            {
              json_tx_doc["config"] = "power";
              json_tx_doc["value"] = cur_config.power;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "pa_bias")
            {
              json_tx_doc["config"] = "pa_bias";
              json_tx_doc["value"] = cur_config.pa_bias;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "cwid")
            {
              json_tx_doc["config"] = "cwid";
              json_tx_doc["value"] = cur_config.cwid;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_1")
            {
              json_tx_doc["config"] = "msg_buffer_1";
              json_tx_doc["value"] = cur_config.msg_buffer_1;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_2")
            {
              json_tx_doc["config"] = "msg_buffer_2";
              json_tx_doc["value"] = cur_config.msg_buffer_2;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_3")
            {
              json_tx_doc["config"] = "msg_buffer_3";
              json_tx_doc["value"] = cur_config.msg_buffer_3;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "msg_buffer_4")
            {
              json_tx_doc["config"] = "msg_buffer_4";
              json_tx_doc["value"] = cur_config.msg_buffer_4;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
            else if(json_rx_doc["config"] == "si5351_int_corr")
            {
              json_tx_doc["config"] = "si5351_int_corr";
              json_tx_doc["value"] = cur_config.si5351_int_corr;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0x03, send_json);
            }
          }
          break;
        case 0x04:
          if(json_rx_doc["action"] == "tx_enable")
          {
            tx_enable = true;
            setTxState(TxState::Idle);
            // Re-compose the buffers to reflect changes
            selectBuffer(cur_config.buffer);
            composeWSPRBuffer();
            setNextTx(0);
          }
          else if(json_rx_doc["action"] == "tx_disable")
          {
            tx_enable = false;
            next_tx = UINT32_MAX;
          }
          else if(json_rx_doc["action"] == "tx_cancel")
          {
            if(cur_state != TxState::Idle)
            {
              setTxState(TxState::Idle);
              morse.reset();
              setNextTx(0);

              StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
              char send_json[JSON_MAX_SIZE + 6];
              json_tx_doc["level"] = 0;
              json_tx_doc["text"] = "TX End";
              json_tx_doc["data"] = cur_config.base_freq;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0xFE, send_json);
            }
          }
          break;
        case 0x06:
          if(json_rx_doc["enum"] == "modes")
          {
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray modes = root.createNestedArray("modes");

            for(auto m : mode_table)
            {
              modes.add(m.mode_name);
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "bands")
          {
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray bands = root.createNestedArray("bands");

//            for(auto b : band_table)
//            {
//              bands.add(b.name);
//            }

            for(auto b : band_table)
            {
              JsonObject nested = bands.createNestedObject();
              nested["name"] = b.name;
              nested["mod"] = b.module_index;
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "band_modules")
          {
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray band_modules = root.createNestedArray("band_modules");

            for(auto b : band_module_table)
            {
              band_modules.add(b.name);
            }

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          else if(json_rx_doc["enum"] == "inst_band_modules")
          {
            JsonObject root = json_tx_doc.to<JsonObject>();
            JsonArray band_modules = root.createNestedArray("inst_band_modules");

            band_modules.add(band_module_index_1);
            band_modules.add(band_module_index_2);

            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0x07, send_json);
          }
          break;
        default:
          break;
        }
      }
    }
  }
  else
  {
    // No connection detected
  }
  yield();
}

void processTxTrigger()
{
  if (rtc.getEpoch() >= next_tx && isTimeValid())
  {
    setTxState(next_state);
    next_tx = UINT32_MAX;
//    SerialUSB.write('\v');
//    for(uint i = 0; i < 79; ++i)
//    {
//      SerialUSB.print(mfsk_buffer[i]);
//    }
  }
  yield();
}

uint16_t sendSerialPacket(uint8_t msg_type, const char * payload)
{
  char serial_packet[JSON_MAX_SIZE + 6];
  uint16_t payload_len = strlen(payload);
  if(payload_len > JSON_MAX_SIZE)
  {
    return 0;
  }

  // Build packet header
  *serial_packet = PACKET_ID;
  *(serial_packet + 1) = msg_type;
  *(serial_packet + 2) = (payload_len >> 8) & 0xff;
  *(serial_packet + 3) = payload_len & 0xff;

  // Add in payload
  memcpy(serial_packet + 4, payload, payload_len);

  // Append terminator char
  *(serial_packet + 4 + payload_len) = PACKET_TERM;

  // Send it
  uint16_t bytes_written = SerialUSB.write(serial_packet, payload_len + 5);
  return bytes_written;
}

//void updateTimer(void)
//{
//  yield();
//
//  // Latch the current time
//  // MUST disable interrupts during this read or there will be an occasional corruption of cur_timer
//  noInterrupts();
//  cur_timer = millis();
//  interrupts();
//}

void txStateMachine()
{
  static bool prev_morse_tx = false;

  switch (meta_mode)
  {
    case MetaMode::CW:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::CW:
          if (!morse.busy)
          {
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_MAX_SIZE + 6];
            json_tx_doc["level"] = 0;
            json_tx_doc["text"] = "TX End";
            json_tx_doc["data"] = cur_config.base_freq;
            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0xFE, send_json);
            yield();
            setTxState(TxState::Idle);
            //frequency = (cur_config.base_freq * 100) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
            //frequency = (cur_config.base_freq * 100);
            //change_freq = true;
            //setNextTx(atoi(settings["TX Intv"].substr(1).c_str()));
//            selectMode(static_cast<uint8_t>(mode));

            setNextTx(0);
          }
          break;
      }
      break;
    case MetaMode::DFCW:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::IDDelay:
          if (cur_timer >= cwid_start)
          {
            setTxState(TxState::CWID);
          }
          break;
        case TxState::CWID:
          if (!morse.busy)
          {
            setNextTx(cur_config.tx_intv);
            setTxState(TxState::Idle);
            morse.tx_enable = false;
//            #ifdef REV_B
//            digitalWrite(TX_KEY, HIGH);
//            #endif
          }
          break;
        case TxState::DFCW:
          if (morse.tx != prev_morse_tx)
          {
            if (morse.tx)
            {
              frequency = (cur_config.base_freq * 100ULL) + cur_tone_spacing;
              change_freq = true;
            }
            else
            {
              frequency = (cur_config.base_freq * 100ULL);
              change_freq = true;
            }

            prev_morse_tx = morse.tx;
          }

          if (!morse.busy)
          {
//            StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
            char send_json[JSON_MAX_SIZE + 6];
            json_tx_doc["level"] = 0;
            json_tx_doc["text"] = "TX End";
            json_tx_doc["data"] = cur_config.base_freq;
            serializeJson(json_tx_doc, send_json);
            sendSerialPacket(0xFE, send_json);
            yield();
            prev_morse_tx = false;
            frequency = (cur_config.base_freq * 100ULL);
            change_freq = true;
              
            if(cur_config.cwid)
            {
              setTxState(TxState::IDDelay);
            }
            else
            {
              setTxState(TxState::Idle);
              setNextTx(cur_config.tx_intv);
              morse.tx_enable = false;
            }
          }
          break;
      }
      break;
    case MetaMode::MFSK:
      switch (cur_state)
      {
        case TxState::Idle:
          break;
        case TxState::IDDelay:
          if (cur_timer >= cwid_start)
          {
            setTxState(TxState::CWID);
          }
          break;
        case TxState::CWID:
          if (!morse.busy)
          {
            setTxState(TxState::Idle);

            setNextTx(cur_config.tx_intv);
            composeMFSKMessage();
          }
          break;
        case TxState::MFSK:
          if (cur_timer >= next_event)
          {
            ++cur_symbol;
            if (cur_symbol >= cur_symbol_count) //reset everything and switch to idle
            {
//              StaticJsonDocument<JSON_MAX_SIZE> json_tx_doc;
              char send_json[JSON_MAX_SIZE + 6];
              json_tx_doc["level"] = 0;
              json_tx_doc["text"] = "TX End";
              json_tx_doc["data"] = cur_config.base_freq;
              serializeJson(json_tx_doc, send_json);
              sendSerialPacket(0xFE, send_json);
              if(cur_config.cwid)
              {
                setTxState(TxState::IDDelay);
              }
              else
              {
                setTxState(TxState::Idle);
                setNextTx(cur_config.tx_intv);
                composeMFSKMessage();
              }
              //frequency = (cur_config.base_freq * 100) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
              //frequency = (cur_config.base_freq * 100);
              //change_freq = true;
//              setNextTx(cur_config.tx_intv);
//              setNextTx(atoi(settings["tx_intv"].second.substr(1).c_str()));
            }
            else // next symbol
            {
              next_event = cur_timer + cur_symbol_time;
              frequency = (cur_config.base_freq * 100ULL) + (mfsk_buffer[cur_symbol] * cur_tone_spacing);
              change_freq = true;
            }
          }
          break;
      }
      break;
  }
  yield();
}

void composeMorseBuffer(uint8_t buf)
{
  char temp_call[16];
  char temp_grid[6];

  sprintf(temp_call, "%s", cur_config.callsign);
  switch(buf)
  {
  case 1:
    sprintf(cur_config.msg_buffer_1, "%s", temp_call);
    break;
  case 2:
    sprintf(cur_config.msg_buffer_2, "%s", temp_call);
    break;
  case 3:
    sprintf(cur_config.msg_buffer_3, "%s", temp_call);
    break;
  case 4:
    sprintf(cur_config.msg_buffer_4, "%s", temp_call);
    break;
  }
      
  yield();
}

void composeWSPRBuffer()
{
  sprintf(cur_callsign, "%s", settings["callsign"].second.substr(1).c_str());
  sprintf(cur_grid, "%s", settings["grid"].second.substr(1).c_str());
  cur_power = atoi(settings["power"].second.substr(1).c_str());
  sprintf(wspr_buffer, "%s %s %u", cur_callsign, cur_grid, cur_power);
//  memset(mfsk_buffer, 0, 255);
//  jtencode.wspr_encode(settings["callsign"].second.substr(1).c_str(), settings["grid"].second.substr(1).c_str(),
//                       atoi(settings["power"].second.substr(1).c_str()), mfsk_buffer);
}

void composeJTBuffer(uint8_t buf)
{
  char temp_call[16];
  char temp_grid[6];

  sprintf(temp_call, "%s", settings["callsign"].second.substr(1).c_str());
  sprintf(temp_grid, "%s", settings["grid"].second.substr(1).c_str());
  switch(buf)
  {
  case 1:
    sprintf(cur_config.msg_buffer_1, "%s %s", temp_call, temp_grid);
    break;
  case 2:
    sprintf(cur_config.msg_buffer_2, "%s %s", temp_call, temp_grid);
    break;
  case 3:
    sprintf(cur_config.msg_buffer_3, "%s %s", temp_call, temp_grid);
    break;
  case 4:
    sprintf(cur_config.msg_buffer_4, "%s %s", temp_call, temp_grid);
    break;
  }
}

void composeMFSKMessage()
{
  switch(cur_config.mode)
  {
  case Mode::WSPR:
    memset(mfsk_buffer, 0, 255);
    jtencode.wspr_encode(cur_callsign, cur_grid, cur_power, mfsk_buffer);
    break;
  case Mode::JT65:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt65_encode(msg_buffer, mfsk_buffer);
    break;
  case Mode::JT9:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt9_encode(msg_buffer, mfsk_buffer);
    break;
  case Mode::JT4:
    memset(mfsk_buffer, 0, 255);
    jtencode.jt4_encode(msg_buffer, mfsk_buffer);
    break;
  case Mode::FT8:
    memset(mfsk_buffer, 0, 255);
    jtencode.ft8_encode(msg_buffer, mfsk_buffer);
//    disable_display_loop = true;
//    jtencode.ft8_encode("NT7S", mfsk_buffer);
//    disable_display_loop = false;
//    jtencode.jt65_encode(msg_buffer, mfsk_buffer);
    break;
  }
}

void serializeConfig()
{
  //flash_config = flash_store.read();
  #ifdef EXT_EEPROM
  byte * byte_array = (byte*) &cur_config;
  for(uint16_t i = 0; i < sizeof(cur_config); i += EEP_24AA64T_BLOCK_SIZE)
  {
    uint8_t eep_status = eeprom.write(i, byte_array, EEP_24AA64T_BLOCK_SIZE); // EEPROM_CONFIG_SIZE
    byte_array += EEP_24AA64T_BLOCK_SIZE;
  }
//  byte write_byte = 11;
//  uint8_t eep_status = eeprom.write(0, &write_byte, 1);
//  if(eep_status != 0)
//  {
//    if(eep_status == EEPROM_ADDR_ERR)
//    {
//      // Tried to write past the end of address space
//      SerialUSB.write('\v');
//      SerialUSB.print("Tried to write past the end of EEPROM address space");
//    }
//    else
//    {
//      SerialUSB.write('\v');
//      SerialUSB.print("EEPROM write error");
//    }
//  }
  #else
  flash_config.valid = true;
  flash_config.version = cur_config.version;
  flash_config.mode = cur_config.mode;
  flash_config.band = cur_config.band;
  flash_config.base_freq = cur_config.base_freq;
  flash_config.wpm = cur_config.wpm;
  flash_config.tx_intv = cur_config.tx_intv;
  flash_config.dfcw_offset = cur_config.dfcw_offset;
  flash_config.buffer = cur_config.buffer;
  strcpy(flash_config.callsign, cur_config.callsign);
  strcpy(flash_config.grid, cur_config.grid);
  flash_config.power = cur_config.power;
  flash_config.pa_bias = cur_config.pa_bias;
  flash_config.cwid = cur_config.cwid;
  strcpy(flash_config.msg_buffer_1, cur_config.msg_buffer_1);
  strcpy(flash_config.msg_buffer_2, cur_config.msg_buffer_2);
  strcpy(flash_config.msg_buffer_3, cur_config.msg_buffer_3);
  strcpy(flash_config.msg_buffer_4, cur_config.msg_buffer_4);
  flash_config.si5351_int_corr = cur_config.si5351_int_corr;
  flash_store.write(flash_config);
  #endif
}

void deserializeConfig()
{
  char temp_str[81];

  // TODO: need to add version checking
  #ifdef EXT_EEPROM
  byte * byte_array = (byte*) &cur_config;
  for(uint16_t i = 0; i < sizeof(cur_config); ++i)
  {
    *byte_array++ = eeprom.read(i);
  }
//  byte read_byte = 0;
//  uint8_t eep_status = eeprom.read(0, &read_byte, 1);
//  if(eep_status != 0)
//  {
//    SerialUSB.write('\v');
//    SerialUSB.print("EEPROM read error");
//  }
//  else
//  {
//    SerialUSB.write('\v');
//    SerialUSB.print("EEPROM read: ");
//    SerialUSB.print(read_byte);
//  }
//  mode = cur_config.mode;
//  strcpy(msg_buffer_1, cur_config.msg_buffer_1);
//  strcpy(msg_buffer_2, cur_config.msg_buffer_2);
//  strcpy(msg_buffer_3, cur_config.msg_buffer_3);
//  strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//  cur_buffer = cur_config.buffer;
  selectBuffer(cur_config.buffer);
  cur_tone_spacing = cur_config.dfcw_offset;
  sprintf(temp_str, "U%lu", cur_config.pa_bias);
  settings["pa_bias"].second = std::string(temp_str);
  sprintf(temp_str, "S%s", cur_config.callsign);
  settings["callsign"].second = std::string(temp_str);
  sprintf(temp_str, "S%s", cur_config.grid);
  settings["grid"].second = std::string(temp_str);
  sprintf(temp_str, "U%lu", cur_config.power);
  settings["power"].second = std::string(temp_str);
  sprintf(temp_str, "U%lu", cur_config.tx_intv);
  settings["tx_intv"].second = std::string(temp_str);
  sprintf(temp_str, "U%lu", cur_config.wpm);
  settings["wpm"].second = std::string(temp_str);
  if(cur_config.cwid)
  {
    settings["cwid"].second = "B1";
  }
  else
  {
    settings["cwid"].second = "B0";
  }
  #else
  flash_config = flash_store.read();
  if(flash_config.valid == true)
  {
    cur_config.mode = flash_config.mode;
    cur_config.band = flash_config.band;
    cur_config.base_freq = flash_config.base_freq;
    cur_config.wpm = flash_config.wpm;
    cur_config.tx_intv = flash_config.tx_intv;
    cur_config.dfcw_offset = flash_config.dfcw_offset;
    cur_config.buffer = flash_config.buffer;
    strcpy(cur_config.callsign, flash_config.callsign);
    strcpy(cur_config.grid, flash_config.grid);
    cur_config.power = flash_config.power;
    cur_config.pa_bias = flash_config.pa_bias;
    cur_config.cwid = flash_config.cwid;
    strcpy(cur_config.msg_buffer_1, flash_config.msg_buffer_1);
    strcpy(cur_config.msg_buffer_2, flash_config.msg_buffer_2);
    strcpy(cur_config.msg_buffer_3, flash_config.msg_buffer_3);
    strcpy(cur_config.msg_buffer_4, flash_config.msg_buffer_4);
    cur_config.si5351_int_corr = flash_config.si5351_int_corr;
//    mode = flash_config.mode;
    strcpy(msg_buffer_1, cur_config.msg_buffer_1);
    strcpy(msg_buffer_2, cur_config.msg_buffer_2);
    strcpy(msg_buffer_3, cur_config.msg_buffer_3);
    strcpy(msg_buffer_4, cur_config.msg_buffer_4);
//    cur_buffer = cur_config.buffer;
    selectBuffer(cur_config.buffer);
    cur_tone_spacing = cur_config.dfcw_offset;
    sprintf(temp_str, "U%lu", flash_config.pa_bias);
    settings["pa_bias"].second = std::string(temp_str);
    sprintf(temp_str, "S%s", flash_config.callsign);
    settings["callsign"].second = std::string(temp_str);
    sprintf(temp_str, "S%s", flash_config.grid);
    settings["grid"].second = std::string(temp_str);
    sprintf(temp_str, "U%lu", flash_config.power);
    settings["power"].second = std::string(temp_str);
    sprintf(temp_str, "U%lu", flash_config.tx_intv);
    settings["tx_intv"].second = std::string(temp_str);
    sprintf(temp_str, "U%lu", flash_config.wpm);
    settings["wpm"].second = std::string(temp_str);
    if(flash_config.cwid)
    {
      settings["cwid"].second = "B1";
    }
    else
    {
      settings["cwid"].second = "B0";
    }
//    SerialUSB.print('\v');
//    SerialUSB.print("Callsign: ");
//    SerialUSB.print(flash_config.callsign);
  }
  else
  {
//    SerialUSB.print('\v');
//    SerialUSB.print("Deserialize failure");
  }
  #endif

  selectMode(static_cast<uint8_t>(cur_config.mode));
//  composeWSPRBuffer();
  setTxState(TxState::Idle);
  next_tx = UINT32_MAX;
  frequency = (cur_config.base_freq * 100ULL);
  change_freq = true;
}

#ifdef EXT_EEPROM
void eraseEEPROM() // Erase by writing 0xFF to every byte
{
  uint8_t data[EEP_24AA64T_BLOCK_SIZE];
  for(uint16_t i = 0; i < EEP_24AA64T_BLOCK_SIZE; ++i)
  {
    data[i] = 0xff;
  }
  for(uint16_t i = 0; i < EEP_24AA64T_CAPACITY; i += EEP_24AA64T_BLOCK_SIZE)
  {
    uint8_t eep_status = eeprom.write(i, data, EEP_24AA64T_BLOCK_SIZE);
  }
}
#endif

// ===== Setup =====
void setup()
{
  // Start u8g2
  u8g2.begin();

  // Draw welcome message
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_prospero_bold_nbp_tr);
  u8g2.drawStr(64 - u8g2.getStrWidth("OpenBeacon Mini") / 2, 15, "OpenBeacon Mini");
  u8g2.setFont(u8g2_font_6x10_mr);
  u8g2.drawStr(64 - u8g2.getStrWidth("Sync with PC to begin") / 2, 30, "Sync with PC to begin");
  u8g2.sendBuffer();
  
  // Serial port init
  SerialUSB.begin(57600);
  while (!SerialUSB);
//
//  u8g2.clearBuffer();
//  u8g2.setDrawColor(1);
//  u8g2.setFont(u8g2_font_prospero_bold_nbp_tr);
//  u8g2.drawStr(64 - u8g2.getStrWidth("Hello") / 2, 15, "Hello");;
//  u8g2.sendBuffer();

  // Load config map
//  for (auto const& c : settings_table)
  for (auto c : settings_table)
  {
    settings[c[0]].first = c[1];
  }

//  // Populate configuration with defaults
//  char temp_str[81];
//  cur_config.valid = true;
//  cur_config.version = CONFIG_SCHEMA_VERSION;
//  cur_config.mode = DEFAULT_MODE;
//  cur_config.band = DEFAULT_BAND_INDEX;
//  cur_config.wpm = DEFAULT_WPM;
//  cur_config.tx_intv = DEFAULT_TX_INTERVAL;
//  cur_config.dfcw_offset = DEFAULT_DFCW_OFFSET;
//  cur_config.buffer = DEFAULT_CUR_BUFFER;
//  strcpy(cur_config.callsign, DEFAULT_CALLSIGN);
//  strcpy(cur_config.grid, DEFAULT_GRID);
//  cur_config.power = DEFAULT_POWER;
//  cur_config.pa_bias = DEFAULT_PA_BIAS;
//  cur_config.cwid = DEFAULT_CWID;
//  strcpy(cur_config.msg_buffer_1, DEFAULT_MSG_1);
//  strcpy(cur_config.msg_buffer_2, DEFAULT_MSG_2);
//  strcpy(cur_config.msg_buffer_3, DEFAULT_MSG_3);
//  strcpy(cur_config.msg_buffer_4, DEFAULT_MSG_4);
//  cur_config.si5351_int_corr = DEFAULT_SI5351_INT_CORR;
//  sprintf(temp_str, "U%lu", cur_config.pa_bias);
//  settings["pa_bias"].second = std::string(temp_str);
//  sprintf(temp_str, "S%s", cur_config.callsign);
//  settings["callsign"].second = std::string(temp_str);
//  sprintf(temp_str, "S%s", cur_config.grid);
//  settings["grid"].second = std::string(temp_str);
//  sprintf(temp_str, "U%lu", cur_config.power);
//  settings["power"].second = std::string(temp_str);
//  sprintf(temp_str, "U%lu", cur_config.tx_intv);
//  settings["tx_intv"].second = std::string(temp_str);
//  sprintf(temp_str, "U%lu", cur_config.wpm);
//  settings["wpm"].second = std::string(temp_str);
//  if(cur_config.cwid)
//  {
//    settings["cwid"].second = "B1";
//  }
//  else
//  {
//    settings["cwid"].second = "B0";
//  }

  // I/O init
  pinMode(BTN_DSP_1, INPUT_PULLUP);
  pinMode(BTN_DSP_2, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
//  pinMode(CLK_INPUT, INPUT);
  pinMode(TX_KEY, OUTPUT);
  #ifdef REV_B
  pinMode(BAND_SW, OUTPUT);
  pinMode(TX_LED, OUTPUT);
  #endif
  pinMode(SYNC_LED, OUTPUT);

//  SerialUSB.write('\v');
//  uint8_t pin_state = digitalRead(TX_KEY);
//  SerialUSB.println(pin_state);

  //attachInterrupt(digitalPinToInterrupt(BTN_BACK), handleMenuBack, FALLING);

  // ADC resolution
  analogReadResolution(12);
  analogReference(AR_DEFAULT);

  // Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);
  si5351.set_freq(cur_config.base_freq * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.set_freq(1000000UL, SI5351_CLK2);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  Wire.setClock(400000UL);

  // RTC setup
  // Can't use the RTC alarm interrupt, far too much trigger time variance
  rtc.begin();
  rtc.setTime(DEFAULT_TIME.tm_hour, DEFAULT_TIME.tm_min, DEFAULT_TIME.tm_sec);
  rtc.setDate(DEFAULT_TIME.tm_mday, DEFAULT_TIME.tm_mon, DEFAULT_TIME.tm_year);
//  time_sync_expire = rtc.getEpoch();
  time_sync_expire = 0;
  next_time_sync = rtc.getEpoch();


  // External EEPROM
  #ifdef EXT_EEPROM
  uint8_t eep_status = eeprom.begin(extEEPROM::twiClock400kHz);
  if(eep_status)
  {
    SerialUSB.write('\v');
    SerialUSB.println("EEPROM init failure");
  }
//  deserializeConfig();
  #endif

//  u8g2.clearBuffer();
//  u8g2.setDrawColor(1);
//  u8g2.setFont(u8g2_font_prospero_bold_nbp_tr);
//  u8g2.drawStr(64 - u8g2.getStrWidth("Hello") / 2, 15, "Hello");;
//  u8g2.sendBuffer();

  // If there isn't a valid configuration record in NVM,
  // create one based on the defaults
  #ifdef EXT_EEPROM
  deserializeConfig();
  if(cur_config.valid == false && cur_config.version != CONFIG_SCHEMA_VERSION)
  {
    cur_config.valid = true;
    cur_config.version = CONFIG_SCHEMA_VERSION;
    cur_config.mode = DEFAULT_MODE;
    cur_config.band = DEFAULT_BAND_INDEX;
    cur_config.base_freq = DEFAULT_FREQUENCY;
    cur_config.wpm = DEFAULT_WPM;
    cur_config.tx_intv = DEFAULT_TX_INTERVAL;
    cur_config.dfcw_offset = DEFAULT_DFCW_OFFSET;
    cur_config.buffer = DEFAULT_CUR_BUFFER;
    strcpy(cur_config.callsign, DEFAULT_CALLSIGN);
    strcpy(cur_config.grid, DEFAULT_GRID);
    cur_config.power = DEFAULT_POWER;
    cur_config.pa_bias = DEFAULT_PA_BIAS;
    cur_config.cwid = DEFAULT_CWID;
    strcpy(cur_config.msg_buffer_1, DEFAULT_MSG_1);
    strcpy(cur_config.msg_buffer_2, DEFAULT_MSG_2);
    strcpy(cur_config.msg_buffer_3, DEFAULT_MSG_3);
    strcpy(cur_config.msg_buffer_4, DEFAULT_MSG_4);
    cur_config.si5351_int_corr = DEFAULT_SI5351_INT_CORR;
    serializeConfig();
    SerialUSB.write('\v');
    SerialUSB.print("New EEPROM store written");
  }
  #else
  flash_config = flash_store.read();
  if(flash_config.valid == false)
  {
    flash_config.valid = true;
    flash_config.version = CONFIG_SCHEMA_VERSION;
    flash_config.mode = DEFAULT_MODE;
    flash_config.band = DEFAULT_BAND_INDEX;
    flash_config.base_freq = DEFAULT_FREQUENCY;
    flash_config.wpm = DEFAULT_WPM;
    flash_config.tx_intv = DEFAULT_TX_INTERVAL;
    flash_config.dfcw_offset = DEFAULT_DFCW_OFFSET;
    flash_config.buffer = DEFAULT_CUR_BUFFER;
    strcpy(flash_config.callsign, DEFAULT_CALLSIGN);
    strcpy(flash_config.grid, DEFAULT_GRID);
    flash_config.power = DEFAULT_POWER;
    flash_config.pa_bias = DEFAULT_PA_BIAS;
    flash_config.cwid = DEFAULT_CWID;
    strcpy(flash_config.msg_buffer_1, DEFAULT_MSG_1);
    strcpy(flash_config.msg_buffer_2, DEFAULT_MSG_2);
    strcpy(flash_config.msg_buffer_3, DEFAULT_MSG_3);
    strcpy(flash_config.msg_buffer_4, DEFAULT_MSG_4);
    flash_config.si5351_int_corr = DEFAULT_SI5351_INT_CORR;
    flash_store.write(flash_config);
    deserializeConfig();
    SerialUSB.print('\v');
    SerialUSB.print("New flash store written");
  }
  else
  {
    // load valid config from NVM to RAM
    deserializeConfig();
  }
  #endif

//  SerialUSB.write('\v');
//  uint8_t pin_state = digitalRead(TX_KEY);
//  SerialUSB.println(pin_state);
//  SerialUSB.write('\v');
//  SerialUSB.println(cur_config.tx_intv);

  // Set up for the mode
//  selectMode(static_cast<uint8_t>(mode));

  // Set PA bias
  setPABias(cur_config.pa_bias);

  // Init menu
  initMenu();

  // Set up ArduinoJSON
  const size_t bufferSize = JSON_OBJECT_SIZE(17) + 230;
//  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3) + 60;
//  StaticJsonDocument<800> jsonDoc;
  DynamicJsonDocument jsonDoc(bufferSize);
//  DynamicJsonBuffer jsonBuffer(bufferSize);
//  JsonObject& root = jsonBuffer.parseObject(default_config);
  DeserializationError error = deserializeJson(jsonDoc, default_config);
  if(error)
  {
    //TODO
  }
  JsonObject config_root = jsonDoc.as<JsonObject>();

  // Set up scheduler
  Scheduler.startLoop(txStateMachine);
  //Scheduler.startLoop(updateTimer);
  //Scheduler.startLoop(processTxTrigger);
  Scheduler.startLoop(drawOLED, 10000);
  Scheduler.startLoop(pollButtons);
  Scheduler.startLoop(selectBand);
  //Scheduler.startLoop(processTimeSync);

  cur_tone_spacing = mode_table[static_cast<uint8_t>(cur_config.mode)].tone_spacing;
  cur_symbol_count = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_count;
  cur_symbol_time = mode_table[static_cast<uint8_t>(cur_config.mode)].symbol_time;
//  next_state = TxState::MFSK;

  // Clear TX buffer
  selectBuffer(cur_config.buffer);
//  composeMFSKMessage();
//  jtencode.wspr_encode(settings["callsign"].second.substr(1).c_str(), settings["grid"].second.substr(1).c_str(),
//                       atoi(settings["power"].second.substr(1).c_str()), mfsk_buffer);
//  prev_state = TxState::Idle;
  setTxState(TxState::Idle);
  next_tx = UINT32_MAX;
  frequency = (cur_config.base_freq * 100ULL);
  change_freq = true;
  
//  composeWSPRBuffer();
//  composeMorseBuffer(cur_buffer);



  //morse.send("DE NT7S");
  //  SerialUSB.print("\v");
  //  SerialUSB.println("OpenBeacon Mini");
  //  //attachInterrupt(digitalPinToInterrupt(CLK_INPUT), handleClkInput, FALLING);
  //  for(auto a : settings_table)
  //  {
  ////    char first_setting[20];
  ////    char second_setting[20];
  ////    sprintf(first_setting, "%s", a.first().c_str());
  ////    sprintf(second_setting, "%s", a.second().c_str());
  //    SerialUSB.print("\v");
  //    SerialUSB.print(a[0]);
  //    SerialUSB.print(" - ");
  //    SerialUSB.println(settings[a[0]].c_str());
  //  }

  #ifdef REV_B
  morse.led_pin = TX_LED;
  #endif
  
  // Start Timer
  startTimer(TIMER_FREQUENCY); // 1 ms ISR
}

void loop()
{
  //  noInterrupts();
  //  cur_timer = millis();
  //  interrupts();
  //  yield();

  if (change_freq)
  {
    //noInterrupts();
    si5351.set_freq(frequency, SI5351_CLK0);
    change_freq = false;
    //interrupts();
  }

  yield();

  processTxTrigger();
  processSerialIn();
  processTimeSync();
}
