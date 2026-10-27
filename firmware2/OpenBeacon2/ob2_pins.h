#pragma once

/*
 * OpenBeacon 2 Rev C, U9 AVR128DA48 pin map.
 *
 * Transcribed from the firmware spec draft 3.5 (approved), section 2.1,
 * with the section 2.2 power-rework net changes applied. DxCore numbers are
 * the 48pin-standard map (PA0 = 0 ... PF5 = 39). This header includes no
 * Arduino headers, so host tests can compile it.
 *
 * POWER REWORK HARDWARE IS NOT COMMITTED IN THIS REPOSITORY.
 * origin/pcb-layout commit 88e9a62 still has PB1 no-connect and PE1-PE3 on
 * header J5 (labels D31_A9_PE1, D32_A10_PE2, D33_A11_PE3). Spec section 2.2
 * (decision D17) supersedes those four nets. This header follows section 2.2.
 * It is not evidence that the rework schematic has landed.
 *
 * Spec section 2.1 leaves the DxCore column blank for PF0, PF1, RESET/PF6,
 * and UPDI. DxCore still numbers PIN_PF0 = 34, PIN_PF1 = 35, and PIN_PF6 = 40.
 * Those pins are not given firmware GPIO constants here (crystal and reset).
 *
 * The three J5 spares are one row in the spec ("PD0-PD2 / J5"). They are
 * expanded to one entry per pin. Power pins come from the section 2.1
 * footnote so the 48-pin package is complete.
 */

#include <stdint.h>

constexpr char OB2_POWER_REWORK_NOTE[] =
    "Power rework (D17) is not committed in this repository. "
    "PB1 BAND_C, PE1 TX_ISENSE, PE2 CC1_SENSE, and PE3 CC2_SENSE follow "
    "spec section 2.2, which supersedes pcb-layout 88e9a62 "
    "(PB1 NC, PE1-PE3 on J5).";

constexpr uint8_t OB2_PIN_REWORK = 1u << 0;
constexpr uint8_t OB2_PIN_OUT = 1u << 1;
constexpr uint8_t OB2_PIN_IN = 1u << 2;
constexpr uint8_t OB2_PIN_ADC = 1u << 3;
constexpr uint8_t OB2_PIN_SPARE = 1u << 4;
constexpr uint8_t OB2_PIN_POWER = 1u << 5;
constexpr uint8_t OB2_PIN_NO_GPIO = 1u << 6;

/* Firmware GPIO numbers. Spec section 2.1 "DxCore" column. */
constexpr uint8_t OB2_BAND_A = 5;      /* PA5 */
constexpr uint8_t OB2_BAND_B = 6;      /* PA6 */
constexpr uint8_t OB2_J4_PA7 = 7;      /* PA7, spare J4.1 */
constexpr uint8_t OB2_GPS_RXD = 8;     /* PB0, USART3 TX to the GPS */
constexpr uint8_t OB2_BAND_C = 9;      /* PB1, rework */
constexpr uint8_t OB2_CAL_CLK = 10;    /* PB2, Si5351C CLK4 */
constexpr uint8_t OB2_SI_INTR = 11;    /* PB3 */
constexpr uint8_t OB2_OLED_DC = 12;    /* PB4 */
constexpr uint8_t OB2_OLED_RES = 13;   /* PB5 */
constexpr uint8_t OB2_UART_TX = 14;    /* PC0, USART1 TX, J10 */
constexpr uint8_t OB2_UART_RX = 15;    /* PC1, USART1 RX, J10 */
constexpr uint8_t OB2_SYNC_LED = 16;   /* PC2 */
constexpr uint8_t OB2_J4_PC3 = 17;     /* PC3, spare J4.3 */
constexpr uint8_t OB2_OLED_MOSI = 18;  /* PC4, SPI1 ALT1 */
constexpr uint8_t OB2_J4_PC5 = 19;     /* PC5, spare J4.5, SPI1 ALT1 MISO */
constexpr uint8_t OB2_OLED_SCK = 20;   /* PC6, SPI1 ALT1 */
constexpr uint8_t OB2_OLED_CS = 21;    /* PC7, SPI1 ALT1 SS as GPIO */
constexpr uint8_t OB2_J5_PD0 = 22;     /* PD0, spare ADC A0 */
constexpr uint8_t OB2_J5_PD1 = 23;     /* PD1, spare ADC A1 */
constexpr uint8_t OB2_J5_PD2 = 24;     /* PD2, spare ADC A2 */
constexpr uint8_t OB2_BTN_UP = 25;
constexpr uint8_t OB2_BTN_DOWN = 26;
constexpr uint8_t OB2_BTN_LEFT = 27;
constexpr uint8_t OB2_BTN_RIGHT = 28;
constexpr uint8_t OB2_BTN_BACK = 29;   /* PD7. Do not select VREFA. */
constexpr uint8_t OB2_BTN_DSP_1 = 30;  /* PE0 */
constexpr uint8_t OB2_TX_ISENSE = 31;  /* PE1, AIN9, rework */
constexpr uint8_t OB2_CC1_SENSE = 32;  /* PE2, AIN10, rework */
constexpr uint8_t OB2_CC2_SENSE = 33;  /* PE3, AIN11, rework */
constexpr uint8_t OB2_GPS_PPS = 36;    /* PF2 */
constexpr uint8_t OB2_BTN_DSP_2 = 37;  /* PF3 */
constexpr uint8_t OB2_J4_PF4 = 38;     /* PF4, spare J4.7 */
constexpr uint8_t OB2_GPS_TXD = 39;    /* PF5, USART2 RX ALT1 */
constexpr uint8_t OB2_MCU_TX = 0;      /* PA0, USART0 TX to the CP2102N */
constexpr uint8_t OB2_MCU_RX = 1;      /* PA1, USART0 RX */
constexpr uint8_t OB2_SDA = 2;         /* PA2, TWI0 */
constexpr uint8_t OB2_SCL = 3;         /* PA3, TWI0 */
constexpr uint8_t OB2_TX = 4;          /* PA4, 12 V PA supply and TX LED */

constexpr int8_t OB2_AIN_J5_PD0 = 0;
constexpr int8_t OB2_AIN_J5_PD1 = 1;
constexpr int8_t OB2_AIN_J5_PD2 = 2;
constexpr int8_t OB2_AIN_TX_ISENSE = 9;
constexpr int8_t OB2_AIN_CC1_SENSE = 10;
constexpr int8_t OB2_AIN_CC2_SENSE = 11;

/*
 * USART2 on ALT1, RX only: GPS TXD -> PF5. ALT1 TX is PF4 (J4.7) and must
 * stay a GPIO. Default USART2 would sit on the 32 kHz crystal (PF0/PF1).
 * USART3 default, TX only: MCU -> GPS RXD on PB0. RX is PB1 (BAND_C);
 * RXEN must stay off or it takes the relay pin.
 */
constexpr uint8_t OB2_USART2_RX = OB2_GPS_TXD;
constexpr uint8_t OB2_USART2_ALT1_TX_DO_NOT_USE = OB2_J4_PF4;
constexpr uint8_t OB2_USART2_PORTMUX_ALT1 = 1;
constexpr uint8_t OB2_USART3_TX = OB2_GPS_RXD;
constexpr uint8_t OB2_USART3_RX_DISABLED = OB2_BAND_C;
constexpr uint8_t OB2_USART3_PORTMUX_DEFAULT = 0;

/* Si5351C CLK4 calibration input on PB2. */
constexpr uint8_t OB2_CLK4_CAL = OB2_CAL_CLK;

/*
 * Display is SPI1 ALT1 (MOSI PC4, MISO PC5, SCK PC6, SS PC7 as GPIO).
 * Do not start SPI0: its default pins are PA4-PA7 (TX, BAND_A, BAND_B, J4.1).
 * Do not start SPI1 on the default mux: MOSI/SCK/SS land on PC0/PC2/PA3
 * (UART_TX, SYNC_LED, SCL). PC5 is forced to an input while SPI1 is master.
 */
constexpr uint8_t OB2_SPI1_MOSI = OB2_OLED_MOSI;
constexpr uint8_t OB2_SPI1_MISO = OB2_J4_PC5;
constexpr uint8_t OB2_SPI1_SCK = OB2_OLED_SCK;
constexpr uint8_t OB2_SPI1_CS = OB2_OLED_CS;

struct Ob2Pin {
  uint8_t package_pin;
  char port;          /* 'A'..'F', or 0 for power and UPDI */
  int8_t bit;         /* 0..7, or -1 */
  int8_t dxcore;      /* Arduino pin, or -1 where the spec column is blank */
  int8_t ain;         /* ADC channel named by the spec, or -1 */
  const char *net;
  const char *role;
  uint8_t flags;
};

static constexpr Ob2Pin OB2_PINS[] = {
    {1, 'A', 5, 5, -1, "BAND_A", "GPIO out to Q6 (K1/K4)", OB2_PIN_OUT},
    {2, 'A', 6, 6, -1, "BAND_B",
     "GPIO out to Q7 (K2/K5). Spec section 2.1 still says Q7 drives K2/K3/K5/K6; "
     "section 2.2 assigns K3/K6 to BAND_C.",
     OB2_PIN_OUT},
    {3, 'A', 7, 7, -1, "D7_PA7", "Spare header J4.1. Committed label D7_PA7.", OB2_PIN_SPARE},
    {4, 'B', 0, 8, -1, "GPS_RXD",
     "USART3 TxD (default) to GPS RXD. USART3 RXEN stays off.", 0},
    {5, 'B', 1, 9, -1, "BAND_C",
     "GPIO out to Q9 (K3/K6), low at reset, R91 22k pull-down. "
     "USART3 RX pin: RXEN stays off. Rework net; 88e9a62 leaves PB1 unconnected.",
     OB2_PIN_OUT | OB2_PIN_REWORK},
    {6, 'B', 2, 10, -1, "CAL_CLK", "Si5351C CLK4 input. EVSYS to TCB0 count.", OB2_PIN_IN},
    {7, 'B', 3, 11, -1, "SI_INTR", "Si5351C INTR (LOS/LOL), input and pin interrupt.", OB2_PIN_IN},
    {8, 'B', 4, 12, -1, "OLED_DC", "GPIO out, SH1106 D/C.", OB2_PIN_OUT},
    {9, 'B', 5, 13, -1, "OLED_RES", "GPIO out, SH1106 reset.", OB2_PIN_OUT},
    {10, 'C', 0, 14, -1, "UART_TX", "USART1 TxD to J10.", 0},
    {11, 'C', 1, 15, -1, "UART_RX", "USART1 RxD from J10.", 0},
    {12, 'C', 2, 16, -1, "SYNC_LED", "GPIO out, SYNC LED D11.", OB2_PIN_OUT},
    {13, 'C', 3, 17, -1, "J4.3", "Spare header J4.3. Committed label D17_PC3.", OB2_PIN_SPARE},
    {14, 0, -1, -1, -1, "VDD", "Power. Package pins 14 and 42.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {15, 0, -1, -1, -1, "GND", "Ground. Package pins 15, 29, and 43.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {16, 'C', 4, 18, -1, "OLED_MOSI", "SPI1 ALT1 MOSI.", 0},
    {17, 'C', 5, 19, -1, "J4.5",
     "Spare header J4.5. SPI1 ALT1 MISO: input-overridden while SPI1 master runs. "
     "Committed label D19_PC5.",
     OB2_PIN_SPARE},
    {18, 'C', 6, 20, -1, "OLED_SCK", "SPI1 ALT1 SCK.", 0},
    {19, 'C', 7, 21, -1, "OLED_CS", "SPI1 ALT1 SS, driven as GPIO chip select (must be an output).", OB2_PIN_OUT},
    {20, 'D', 0, 22, 0, "J5",
     "Spare header J5, ADC A0 / AIN0. Committed label D22_A0_PD0. Stays on J5 after the rework.",
     OB2_PIN_SPARE | OB2_PIN_ADC},
    {21, 'D', 1, 23, 1, "J5",
     "Spare header J5, ADC A1 / AIN1. Committed label D23_A1_PD1. Stays on J5 after the rework.",
     OB2_PIN_SPARE | OB2_PIN_ADC},
    {22, 'D', 2, 24, 2, "J5",
     "Spare header J5, ADC A2 / AIN2. Committed label D24_A2_PD2. Stays on J5 after the rework.",
     OB2_PIN_SPARE | OB2_PIN_ADC},
    {23, 'D', 3, 25, -1, "BTN_UP", "Input, pressed = low.", OB2_PIN_IN},
    {24, 'D', 4, 26, -1, "BTN_DOWN", "Input, pressed = low.", OB2_PIN_IN},
    {25, 'D', 5, 27, -1, "BTN_LEFT", "Input, pressed = low.", OB2_PIN_IN},
    {26, 'D', 6, 28, -1, "BTN_RIGHT", "Input, pressed = low.", OB2_PIN_IN},
    {27, 'D', 7, 29, -1, "BTN_BACK",
     "Input, pressed = low. PD7 is VREFA: do not select VREFA as the ADC reference.", OB2_PIN_IN},
    {28, 0, -1, -1, -1, "AVDD", "Analog supply. Package pin 28.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {29, 0, -1, -1, -1, "GND", "Ground. Package pins 15, 29, and 43.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {30, 'E', 0, 30, -1, "BTN_DSP_1", "Input, pressed = low. S1 Menu1.", OB2_PIN_IN},
    {31, 'E', 1, 31, 9, "TX_ISENSE",
     "ADC AIN9, PA current. Rework net; 88e9a62 label is D31_A9_PE1 on J5.",
     OB2_PIN_ADC | OB2_PIN_REWORK},
    {32, 'E', 2, 32, 10, "CC1_SENSE",
     "ADC AIN10, USB-C CC1. Rework net; 88e9a62 label is D32_A10_PE2 on J5.",
     OB2_PIN_ADC | OB2_PIN_REWORK},
    {33, 'E', 3, 33, 11, "CC2_SENSE",
     "ADC AIN11, USB-C CC2. Rework net; 88e9a62 label is D33_A11_PE3 on J5.",
     OB2_PIN_ADC | OB2_PIN_REWORK},
    {34, 'F', 0, -1, -1, "Y1",
     "XOSC32K TOSC1. Not a firmware GPIO. Spec DxCore column is blank; DxCore PIN_PF0 is 34.",
     OB2_PIN_NO_GPIO},
    {35, 'F', 1, -1, -1, "Y1",
     "XOSC32K TOSC2. Not a firmware GPIO. Spec DxCore column is blank; DxCore PIN_PF1 is 35.",
     OB2_PIN_NO_GPIO},
    {36, 'F', 2, 36, -1, "GPS_PPS", "GPS timepulse. EVSYS capture.", OB2_PIN_IN},
    {37, 'F', 3, 37, -1, "BTN_DSP_2", "Input, pressed = low. S2 Menu2.", OB2_PIN_IN},
    {38, 'F', 4, 38, -1, "J4.7",
     "Spare header J4.7. Must not become USART2 TxD. Committed label D38_A20_PF4.",
     OB2_PIN_SPARE},
    {39, 'F', 5, 39, -1, "GPS_TXD",
     "USART2 RxD, PORTMUX ALT1, RX only. GPS TXD into the MCU. Do not enable TXEN.", 0},
    {40, 'F', 6, -1, -1, "RESET",
     "DTR auto-reset through C37/R36. Not a firmware GPIO. Spec DxCore column is blank; "
     "DxCore PIN_PF6 is 40.",
     OB2_PIN_NO_GPIO},
    {41, 0, -1, -1, -1, "J13", "UPDI programming header.", OB2_PIN_NO_GPIO},
    {42, 0, -1, -1, -1, "VDD", "Power. Package pins 14 and 42.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {43, 0, -1, -1, -1, "GND", "Ground. Package pins 15, 29, and 43.", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {44, 'A', 0, 0, -1, "MCU_TX", "USART0 TxD to CP2102N RXD. Host protocol.", 0},
    {45, 'A', 1, 1, -1, "MCU_RX", "USART0 RxD from CP2102N TXD. Host protocol.", 0},
    {46, 'A', 2, 2, -1, "SDA", "TWI0 SDA. Si5351C, 24AA64T, MCP4725 via the shifter, J8.", 0},
    {47, 'A', 3, 3, -1, "SCL", "TWI0 SCL.", 0},
    {48, 'A', 4, 4, -1, "TX", "GPIO out: 12 V PA supply and TX LED D12.", OB2_PIN_OUT},
};

constexpr uint8_t OB2_PIN_COUNT = sizeof(OB2_PINS) / sizeof(OB2_PINS[0]);

constexpr bool ob2_pins_unique() {
  for (uint8_t i = 0; i < OB2_PIN_COUNT; ++i) {
    for (uint8_t j = 0; j < i; ++j) {
      if (OB2_PINS[i].package_pin == OB2_PINS[j].package_pin) {
        return false;
      }
      if (OB2_PINS[i].port != 0 && OB2_PINS[i].port == OB2_PINS[j].port &&
          OB2_PINS[i].bit == OB2_PINS[j].bit) {
        return false;
      }
      if (OB2_PINS[i].dxcore >= 0 && OB2_PINS[i].dxcore == OB2_PINS[j].dxcore) {
        return false;
      }
    }
  }
  return OB2_PIN_COUNT == 48;
}

constexpr int ob2_rework_count() {
  int n = 0;
  for (uint8_t i = 0; i < OB2_PIN_COUNT; ++i) {
    if ((OB2_PINS[i].flags & OB2_PIN_REWORK) != 0) {
      n++;
    }
  }
  return n;
}

static_assert(ob2_pins_unique(), "U9 pin table has a duplicate or the wrong count");
static_assert(ob2_rework_count() == 4, "spec section 2.2 changes four pins");
static_assert(OB2_USART2_RX == 39, "GPS RX is USART2 PF5");
static_assert(OB2_USART2_ALT1_TX_DO_NOT_USE == 38, "USART2 ALT1 TX is the J4.7 spare");
static_assert(OB2_USART3_TX == 8, "MCU TX to the GPS is USART3 PB0");
static_assert(OB2_USART3_RX_DISABLED == 9, "USART3 RX is BAND_C and stays off");
static_assert(OB2_CLK4_CAL == 10, "CLK4 cal input is PB2");
static_assert(OB2_SPI1_MISO == 19, "SPI1 ALT1 MISO is PC5");
static_assert(OB2_AIN_TX_ISENSE == 9, "TX_ISENSE is AIN9");
static_assert(OB2_AIN_CC1_SENSE == 10, "CC1_SENSE is AIN10");
static_assert(OB2_AIN_CC2_SENSE == 11, "CC2_SENSE is AIN11");

inline const Ob2Pin *ob2_pin_at(uint8_t package_pin) {
  for (uint8_t i = 0; i < OB2_PIN_COUNT; ++i) {
    if (OB2_PINS[i].package_pin == package_pin) {
      return &OB2_PINS[i];
    }
  }
  return nullptr;
}
