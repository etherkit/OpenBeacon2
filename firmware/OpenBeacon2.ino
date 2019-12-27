// OpenBeacon 2
// Etherkit
//
// Rev 13 October 2019
//
// Hardware Requirements
// ---------------------
// This firmware must be run on an Etherkit Empyrean Alpha or Arduino Zero capable microcontroller
//
// Required Libraries
// ------------------
// Flash Storage (Library Manager)
// TinyGPS++ (http://arduiniana.org/libraries/tinygpsplus/)
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
// Arduino-MemoryFree (https://github.com/mpflaga/Arduino-MemoryFree)

//#define REV_A
#define REV_B

#ifdef REV_B
#define EXT_EEPROM // Microchip 24AA64T
#endif

#include <Scheduler.h>
#include <JTEncode.h>
#include <ArduinoJson.h>
#include <Menu.h>
#include <RTCZero.h>
#include <Morse.h>
#include <morsechar.h>
#include <U8g2lib.h>
#include <si5351.h>
#include <Wire.h>
#include <MemoryFree.h>
#ifdef EXT_EEPROM
#include <extEEPROM.h>
#else
#include <FlashStorage.h>
#endif

#include <cstdlib>
#include <map>
#include <string>
#include <cstring>
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
constexpr uint16_t JSON_MAX_SIZE = 2000;
constexpr uint16_t JSON_PACKET_MAX_SIZE = 400;

constexpr static unsigned char lock_bits[] = {
  0x18, 0x24, 0x24, 0x7e, 0x81, 0x81, 0x81, 0x7e
};

constexpr char SCREEN_SAVER_MESSAGE[] = "OpenBeacon 2";
constexpr uint32_t SCREEN_SAVER_TIMER_INTERVAL = 2;

const std::string settings_str_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-+/. ";

// Configuration schema version 1
// 8 Oct 2018
struct Config
{
  bool valid;
  uint8_t version;
  Mode mode;
  uint8_t band;
  uint32_t base_freq;
  float wpm;
  uint8_t tx_intv;
  uint8_t dfcw_offset;
  uint8_t buffer;
  char callsign[20];
  char grid[10];
  int8_t power;
  uint16_t pa_bias;
  bool cwid;
  char msg_buffer_1[MSG_BUFFER_SIZE];
  char msg_buffer_2[MSG_BUFFER_SIZE];
  char msg_buffer_3[MSG_BUFFER_SIZE];
  char msg_buffer_4[MSG_BUFFER_SIZE];
  int32_t si5351_int_corr; 
  bool rnd_tx;
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
constexpr int8_t DEFAULT_POWER = 23;
#ifdef REV_A
constexpr uint16_t DEFAULT_PA_BIAS = 1800;
#endif
#ifdef REV_B
constexpr uint16_t DEFAULT_PA_BIAS = 2000;
#endif
constexpr bool DEFAULT_CWID = false;
constexpr char DEFAULT_MSG_1[41] = "BUFFER1";
constexpr char DEFAULT_MSG_2[41] = "BUFFER2";
constexpr char DEFAULT_MSG_3[41] = "BUFFER3";
constexpr char DEFAULT_MSG_4[41] = "BUFFER4";
constexpr uint64_t DEFAULT_SI5351_INT_CORR = 0ULL;
constexpr bool DEFAULT_RND_TX = false;
constexpr uint8_t DEFAULT_SCREEN_SAVER_INTERVAL = 2; // In minutes
constexpr uint16_t DEFAULT_RANDOM_TX_GUARD_BAND = 20;

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
  {"rnd_tx", "Rnd TX"},
  {"wpm", "CW WPM"},
  {"cwid", "CW ID"}
};

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
//char cur_callsign[21];
//char cur_grid[5];
//uint8_t cur_power;
uint32_t next_tx = UINT32_MAX;
char next_tx_time[14];
uint32_t time_sync_expire = 0;
uint32_t next_time_sync = 0;
uint32_t initial_time_sync = 0;
//bool time_sync_request = false;
uint8_t mfsk_buffer[255];
std::string wspr_buffer;
//char msg_buffer[MSG_BUFFER_SIZE];
std::string msg_buffer;
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
bool select_band_reset = true;
char screen_saver_msg[40];
bool screen_saver_enable = false;
uint8_t screen_saver_interval = DEFAULT_SCREEN_SAVER_INTERVAL;
uint32_t screen_saver_timeout = screen_saver_interval * 60 * 1000UL; // Convert to ms for timer
uint8_t cur_screen_saver_x = 30;
uint8_t cur_screen_saver_y = 15;
int8_t screen_saver_x_accel = 1;
int8_t screen_saver_y_accel = 1;
uint32_t screen_saver_update = 0;
uint8_t tx_progress = 0;
//bool random_tx_freq = true;
//uint16_t random_tx_guard_band = DEFAULT_RANDOM_TX_GUARD_BAND;
uint16_t random_tx_guard_band = 30;

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

// ===== Setup =====
void setup()
{
  // Start u8g2
  u8g2.setBusClock(400000);
  u8g2.begin();

  // Draw welcome message
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_prospero_bold_nbp_tr);
  u8g2.drawStr(64 - u8g2.getStrWidth("OpenBeacon 2") / 2, 15, "OpenBeacon 2");
  u8g2.setFont(u8g2_font_6x10_mr);
  u8g2.drawStr(64 - u8g2.getStrWidth("Sync with PC to begin") / 2, 30, "Sync with PC to begin");
  u8g2.sendBuffer();
  
  // Serial port init
  SerialUSB.begin(57600);
  while (!SerialUSB);

  // Load config map
  for (auto const& c : settings_table)
  {
    settings[c[0]].first = c[1];
  }

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

  randomSeed(analogRead(5));

  //attachInterrupt(digitalPinToInterrupt(BTN_BACK), handleMenuBack, FALLING);

  // ADC resolution
  analogReadResolution(12);
  analogReference(AR_DEFAULT);

  // Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);
  si5351.set_freq(cur_config.base_freq * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.set_freq(1000000UL, SI5351_CLK2);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

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
    cur_config.rnd_tx = DEFAULT_RND_TX;
    serializeConfig();
    sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"New EEPROM store written\"}");
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
    flash_config.rnd_tx = DEFAULT_RND_TX;
    flash_store.write(flash_config);
    deserializeConfig();
    sendSerialPacket(0xFE, "{\"level\":0,\"text\":\"New flash store written\"}");
  }
  else
  {
    // load valid config from NVM to RAM
    deserializeConfig();
  }
  #endif

  // Set PA bias
  setPABias(cur_config.pa_bias);

  // Init menu
  initMenu();

  // Set up scheduler
  Scheduler.startLoop(txStateMachine);
  Scheduler.startLoop(drawOLED, 2000);
  Scheduler.startLoop(pollButtons);
  Scheduler.startLoop(selectBand);
  // Scheduler.startLoop(processScreenSaver);
  //Scheduler.startLoop(processTimeSync);

  selectMode(static_cast<uint8_t>(cur_config.mode));

  // Clear TX buffer
  selectBuffer(cur_config.buffer);
  prev_state = TxState::Idle;
  setTxState(TxState::Idle);
  next_tx = UINT32_MAX;
  frequency = (cur_config.base_freq * 100ULL);
  change_freq = true;
  
//  composeWSPRBuffer();
//  composeMorseBuffer(cur_buffer);

  #ifdef REV_B
  morse.led_pin = TX_LED;
  #endif

  // screen_saver_timeout = cur_timer + (screen_saver_interval * 60 * 1000UL); // Convert to ms for timer
  
  // Start Timer
  startTimer(TIMER_FREQUENCY); // 1 ms ISR
}

void loop()
{
  if (change_freq)
  {
    //noInterrupts();
    si5351.set_freq(frequency, SI5351_CLK0);
    change_freq = false;
    //interrupts();
  }

  yield();

  processTxTrigger();
//  yield();
  processSerialIn();
//  yield();
  processTimeSync();
//  yield();
  processScreenSaver();
  yield();
}
