#include <Menu.h>
#include <RTCZero.h>
#include <Morse.h>
#include <morsechar.h>
#include <U8g2lib.h>
#include <si5351.h>
#include <Wire.h>

#include "bands.h"
#include "modes.h"

// Data structures


// Hardware constexprs
constexpr uint8_t BTN_DSP_1 = 0;
constexpr uint8_t BTN_DSP_2 = 1;
constexpr uint8_t BTN_UP = 2;
constexpr uint8_t BTN_DOWN = 3;
constexpr uint8_t BTN_LEFT = 4;
constexpr uint8_t BTN_RIGHT = 5;
constexpr uint8_t BTN_BACK = 6;
constexpr uint8_t TX_KEY = 13;

constexpr uint8_t ADC_BAND_ID = 0;

constexpr uint8_t MCP4725A1_BUS_BASE_ADDR = 0x62;
constexpr uint16_t MCP4725A1_VREF = 5000UL;
constexpr uint16_t ANALOG_REF = 3300UL;
constexpr uint16_t PA_BIAS_FULL = 1850UL;

constexpr uint16_t TIMER_PRESCALER_DIV = 1024;

constexpr static unsigned char lock_bits[] = {
   0x18, 0x24, 0x24, 0x7e, 0x81, 0x81, 0x81, 0x7e };

// Defaults
constexpr uint32_t DEFAULT_FREQUENCY = 0UL;
constexpr uint32_t DEFAULT_LOWER_FREQ_LIMIT = 0UL;
constexpr uint32_t DEFAULT_UPPER_FREQ_LIMIT = 0UL;
constexpr uint8_t DEFAULT_BAND_INDEX = 0;
constexpr MetaMode DEFAULT_METAMODE = MetaMode::MORSE;
constexpr Mode DEFAULT_MODE = Mode::CW;
constexpr bool DEFAULT_TX_LOCK = false;

// Limits

// Class constructors
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Si5351 si5351;
Morse morse(LED_BUILTIN, 15);
RTCZero rtc;
Menu menu;

// ISR global variables
volatile uint32_t frequency = DEFAULT_FREQUENCY; // 30m WSPR
volatile uint32_t last_reported_pos;   // change management
volatile uint32_t lower_freq_limit = DEFAULT_LOWER_FREQ_LIMIT;
volatile uint32_t upper_freq_limit = DEFAULT_UPPER_FREQ_LIMIT;
volatile uint8_t band_index = DEFAULT_BAND_INDEX;
volatile MetaMode meta_mode = DEFAULT_METAMODE;
volatile Mode mode = DEFAULT_MODE;
volatile bool tx_lock = DEFAULT_TX_LOCK;
volatile bool display_menu = false;

// Global variables
uint8_t tune_step = 0;
uint32_t band_id = 0;


// Timer code derived from:
// https://github.com/nebs/arduino-zero-timer-demo

void setTimerFrequency(int frequencyHz)
{
  int compareValue = (VARIANT_MCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC5;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

/*
This is a slightly modified version of the timer setup found at:
https://github.com/maxbader/arduino_tools
 */
void startTimer(int frequencyHz)
{
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TC4_TC5)) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 );

  TcCount16* TC = (TcCount16*) TC5;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1);

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC5_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void TC5_Handler()
{
  TcCount16* TC = (TcCount16*) TC5;

  if (TC->INTFLAG.bit.MC0 == 1)
  {
    TC->INTFLAG.bit.MC0 = 1;
    morse.update();
  }
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
  Wire.beginTransmission(MCP4725A1_BUS_BASE_ADDR);
  Wire.write(reg1);
  Wire.write(reg2);
  Wire.endTransmission();
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
  menu.addChild("Settings");

  //menu.selectChild(0);
}

// ===== ISRs =====
void rtcAlarm()
{
  morse.send("DE NT7S");
}

void handleMenuBack()
{
  display_menu ^= true;
}

void selectMode(uint8_t sel)
{
  //switch(static_cast<Mode>(menu.active_child))
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

//void selectModeDFCW3()
//{
//  mode = Mode::DFCW3;
//}
//
//void selectModeDFCW6()
//{
//  mode = Mode::DFCW6;
//}
//
//void selectModeDFCW10() {}
//void selectModeDFCW120() {}
//void selectModeQRSS3() {}
//void selectModeQRSS6() {}
//void selectModeQRSS10() {}
//void selectModeQRSS120() {}
//void selectModeCW() {}
//void selectModeHELL() {}
//void selectModeWSPR() {}
//void selectModeJT65() {}
//void selectModeJT9() {}
//void selectModeJT4() {}
//void selectModeCAL() {}

// ===== Setup =====
void setup()
{
  // Serial port init
//  SerialUSB.begin(57600);
//  while(!SerialUSB);
  
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
  pinMode(TX_KEY, OUTPUT);

  //attachInterrupt(digitalPinToInterrupt(BTN_BACK), handleMenuBack, FALLING);

  //pinMode(LED_BUILTIN, OUTPUT);

  // ADC resolution
  analogReadResolution(12);
  analogReference(AR_DEFAULT);

  // Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);
  si5351.set_freq(frequency * SI5351_FREQ_MULT, SI5351_CLK0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  // Set PA bias
  setPABias(PA_BIAS_FULL);
  //set_pa_bias(0);

  // TEST: turn on keying
  delay(1000);
  digitalWrite(TX_KEY, HIGH);

  // RTC setup
  rtc.begin();
  rtc.setTime(18, 1, 50);
  rtc.setDate(25, 1, 2018UL);
  rtc.setAlarmTime(18, 2, 40);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(rtcAlarm);

  // Init menu
  initMenu();

  // Start Timer
  startTimer(1000); // 1 ms ISR

  //morse.send("DE NT7S");
}

void drawOLED()
{
  static char temp_str[8];
  static char temp_chr[2];
  static uint32_t freq;
  static uint8_t zero_pad = 0;
  static uint8_t underline;
  static char menu_1[16];
  static char menu_2[16];
  static uint8_t menu_1_x, menu_2_x;

  // u8g2 draw loop
  // --------------
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_logisoso16_tn);
  u8g2.setDrawColor(1);
  //u8g2.setFont(u8g2_font_inb19_mn);

  // MHz
  //yield();
  freq = frequency;
  if(freq / 1000000UL > 0)
  {
    sprintf(temp_str, "%3lu", freq / 1000000UL);
    zero_pad = 1;
  }
  else
  {
    sprintf(temp_str, "   ");
  }
  // We do this because the desired font isn't quite monospaced :-/
  for(uint8_t i = 0; i < 3; ++i) {
    memmove(temp_chr, temp_str + i, 1);
    u8g2.drawStr(i * 9, 17, temp_chr);
  }
  freq %= 1000000UL;
  
  // kHz
  //yield();
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
  for(uint8_t i = 0; i < 3; ++i) {
    memmove(temp_chr, temp_str + i, 1);
    u8g2.drawStr(i * 9 + 29, 17, temp_chr);
  }
  freq %= 1000UL;
  
  // Hz
  //yield();
  if(zero_pad == 1)
  {
    sprintf(temp_str, "%03lu", freq);
  }
  else
  {
    sprintf(temp_str, "%3lu", freq);
  }
  //u8g2.drawStr(78, 20, temp_str);
  for(uint8_t i = 0; i < 3; ++i) {
    memmove(temp_chr, temp_str + i, 1);
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
  u8g2.setFont(u8g2_font_5x7_tn);
  sprintf(temp_str, "%02u:%02u:%02u", rtc.getHours(),
    rtc.getMinutes(), rtc.getSeconds());
  u8g2.drawStr(88, 6, temp_str);

  // Draw mode
  //u8g2.setFont(u8g2_font_5x8_mr);
  u8g2.setFont(u8g2_font_6x10_mr);
  sprintf(temp_str, "%s", mode_table[static_cast<uint8_t>(mode)].mode_name);
  u8g2.drawStr(88, 15, temp_str);

  // Draw TX lock
  u8g2.drawXBM(120, 8, 8, 8, lock_bits);

  // Draw buffer or menu items
  if(display_menu)
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

    // Arrows
    constexpr uint8_t triangle_top = 22;
    constexpr uint8_t triangle_bottom = 31;
    constexpr uint8_t triangle_center = 27;
    constexpr uint8_t left_triangle_left = 0;
    constexpr uint8_t left_triangle_right = 4;
    constexpr uint8_t right_triangle_left = 112;
    constexpr uint8_t right_triangle_right = 117;
    
//    if(menu.countChildren() > 2)
//    {
      u8g2.drawTriangle(left_triangle_right, triangle_top,
        left_triangle_right, triangle_bottom, 
        left_triangle_left, triangle_center);
      u8g2.drawTriangle(right_triangle_left, triangle_top,
        right_triangle_left, triangle_bottom, 
        right_triangle_right, triangle_center);
//    }
    
    // Back icon
    u8g2.setFont(u8g2_font_m2icon_9_tf);
    u8g2.drawGlyph(118, 31, 0x0061);
  }
  else // Show the current buffer
  {
    // Menu icon
    u8g2.setFont(u8g2_font_m2icon_9_tf);
    u8g2.drawGlyph(121, 31, 0x0042);
  }
//  sprintf(menu_1, "%s", "DFCW");
//  sprintf(menu_2, "%s", "TX Enb");
//  menu_1_x = 32 - ((u8g2.getStrWidth(menu_1) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_1) / 2);
//  menu_2_x = 96 - ((u8g2.getStrWidth(menu_2) / 2) > 32 ? 32 : u8g2.getStrWidth(menu_2) / 2);
//  
//  u8g2.setFont(u8g2_font_6x10_mr);
//  //u8g2.setDrawColor(0);
//  u8g2.drawStr(menu_1_x, 31, menu_1);
//  u8g2.drawStr(menu_2_x, 31, menu_2);
  
  u8g2.sendBuffer();          // transfer internal memory to the display
}

void pollButtons()
{
  // Read buttons
  // ------------
  
  // Update frequency based on encoder
  if(last_reported_pos != frequency)
  {
    last_reported_pos = frequency;
    
    si5351.set_freq(frequency * SI5351_FREQ_MULT, SI5351_CLK0);
  }

  // Handle up button
  if(digitalRead(BTN_UP) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_UP) == LOW)
    {
      if(display_menu)
      {
        
      }
      else
      {
        if(frequency + power_10(tune_step) > upper_freq_limit)
        {
          frequency = upper_freq_limit;
        }
        else
        {
          frequency += power_10(tune_step);
        }
      }
      
      delay(50); //delay to avoid many steps at one
    }
  }

  // Handle down button
  if(digitalRead(BTN_DOWN) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_DOWN) == LOW)
    {
      if(display_menu)
      {
        
      }
      else
      {
        if(frequency - power_10(tune_step) < lower_freq_limit)
        {
          frequency = lower_freq_limit;
        }
        else
        {
          frequency -= power_10(tune_step);
        }
      }
      
      delay(50); //delay to avoid many steps at one
    }
  }

  // Handle left button
  if(digitalRead(BTN_LEFT) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_LEFT) == LOW)
    {
      if(display_menu)
      {
        menu--;
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
    }
  }

  // Handle right button
  if(digitalRead(BTN_RIGHT) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_RIGHT) == LOW)
    {
      if(display_menu)
      {
        menu++;
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
    }
  }

  // Handle display 1 button
  if(digitalRead(BTN_DSP_1) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_DSP_1) == LOW)
    {
      if(display_menu)
      {
        if(menu.selectChild(menu.active_child))
        {
          display_menu = false;
          menu.selectRoot();
        }
      }
      
      delay(50); //delay to avoid many steps at one
    }
  }

  // Handle display 2 button
  if(digitalRead(BTN_DSP_2) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_DSP_2) == LOW)
    {
      if(display_menu)
      {
        if(menu.selectChild(menu.active_child + 1))
        {
          display_menu = false;
          menu.selectRoot();
        }
      }
      
      delay(50); //delay to avoid many steps at one
    }
  }

  // Handle menu button
  if(digitalRead(BTN_BACK) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BTN_BACK) == LOW)
    {
      if(display_menu)
      {
        if(menu.selectParent()) // if we are at root menu, exit
        {
          display_menu = false;
        }
      }
      else
      {
        display_menu = true;
      }
      //display_menu ^= true;
      
      delay(50); //delay to avoid many steps at one
    }
  }
}

void selectBand()
{
  // TODO: handle out of bounds
  band_id = analogRead(A0);
  band_id = (band_id * ANALOG_REF) / 4096UL;
  for(auto band : band_table)
  {
    if(band_id < band.upper_v && band_id > band.lower_v)
    {
      if(band.index != band_index)
      {
        band_index = band.index;
        lower_freq_limit = band.lower_limit;
        upper_freq_limit = band.upper_limit;
        if(frequency > upper_freq_limit || frequency < lower_freq_limit)
        {
          frequency = band.wspr_freq;
        }
      }
    }
  }
}

void loop()
{
  drawOLED();
  pollButtons();
  selectBand();

  switch(meta_mode)
  {
  case MetaMode::MORSE:
    break;
  case MetaMode::MFSK:
    break;
  }
}
