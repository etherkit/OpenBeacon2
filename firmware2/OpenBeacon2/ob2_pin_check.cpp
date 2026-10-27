/*
 * Compile-time cross-check of ob2_pins.h against the DxCore 48-pin map.
 * A failure here means DxCore's numbers moved relative to spec section 2.1.
 * Do not edit the spec numbers in the header to silence an assert; record
 * the mismatch.
 */

#include <Arduino.h>

#include "ob2_pins.h"

static_assert(OB2_BAND_A == PIN_PA5, "PA5");
static_assert(OB2_BAND_B == PIN_PA6, "PA6");
static_assert(OB2_J4_PA7 == PIN_PA7, "PA7");
static_assert(OB2_GPS_RXD == PIN_PB0, "PB0");
static_assert(OB2_BAND_C == PIN_PB1, "PB1");
static_assert(OB2_CAL_CLK == PIN_PB2, "PB2");
static_assert(OB2_SI_INTR == PIN_PB3, "PB3");
static_assert(OB2_OLED_DC == PIN_PB4, "PB4");
static_assert(OB2_OLED_RES == PIN_PB5, "PB5");
static_assert(OB2_UART_TX == PIN_PC0, "PC0");
static_assert(OB2_UART_RX == PIN_PC1, "PC1");
static_assert(OB2_SYNC_LED == PIN_PC2, "PC2");
static_assert(OB2_J4_PC3 == PIN_PC3, "PC3");
static_assert(OB2_OLED_MOSI == PIN_PC4, "PC4");
static_assert(OB2_J4_PC5 == PIN_PC5, "PC5");
static_assert(OB2_OLED_SCK == PIN_PC6, "PC6");
static_assert(OB2_OLED_CS == PIN_PC7, "PC7");
static_assert(OB2_J5_PD0 == PIN_PD0, "PD0");
static_assert(OB2_J5_PD1 == PIN_PD1, "PD1");
static_assert(OB2_J5_PD2 == PIN_PD2, "PD2");
static_assert(OB2_BTN_UP == PIN_PD3, "PD3");
static_assert(OB2_BTN_DOWN == PIN_PD4, "PD4");
static_assert(OB2_BTN_LEFT == PIN_PD5, "PD5");
static_assert(OB2_BTN_RIGHT == PIN_PD6, "PD6");
static_assert(OB2_BTN_BACK == PIN_PD7, "PD7");
static_assert(OB2_BTN_DSP_1 == PIN_PE0, "PE0");
static_assert(OB2_TX_ISENSE == PIN_PE1, "PE1");
static_assert(OB2_CC1_SENSE == PIN_PE2, "PE2");
static_assert(OB2_CC2_SENSE == PIN_PE3, "PE3");
static_assert(OB2_GPS_PPS == PIN_PF2, "PF2");
static_assert(OB2_BTN_DSP_2 == PIN_PF3, "PF3");
static_assert(OB2_J4_PF4 == PIN_PF4, "PF4");
static_assert(OB2_GPS_TXD == PIN_PF5, "PF5");
static_assert(OB2_MCU_TX == PIN_PA0, "PA0");
static_assert(OB2_MCU_RX == PIN_PA1, "PA1");
static_assert(OB2_SDA == PIN_PA2, "PA2");
static_assert(OB2_SCL == PIN_PA3, "PA3");
static_assert(OB2_TX == PIN_PA4, "PA4");

static_assert(PIN_A0 == PIN_PD0, "A0");
static_assert(PIN_A1 == PIN_PD1, "A1");
static_assert(PIN_A2 == PIN_PD2, "A2");
static_assert(PIN_A9 == PIN_PE1, "AIN9");
static_assert(PIN_A10 == PIN_PE2, "AIN10");
static_assert(PIN_A11 == PIN_PE3, "AIN11");

static_assert(PIN_HWSERIAL0_TX == PIN_PA0, "USART0 TX");
static_assert(PIN_HWSERIAL0_RX == PIN_PA1, "USART0 RX");
static_assert(PIN_HWSERIAL1_TX == PIN_PC0, "USART1 TX");
static_assert(PIN_HWSERIAL1_RX == PIN_PC1, "USART1 RX");
static_assert(PIN_HWSERIAL2_RX_PINSWAP_1 == PIN_PF5, "USART2 ALT1 RX");
static_assert(PIN_HWSERIAL2_TX_PINSWAP_1 == PIN_PF4, "USART2 ALT1 TX is J4.7");
static_assert(PIN_HWSERIAL2_TX == PIN_PF0, "USART2 default TX is the crystal");
static_assert(PIN_HWSERIAL2_RX == PIN_PF1, "USART2 default RX is the crystal");
static_assert(PIN_HWSERIAL3_TX == PIN_PB0, "USART3 TX");
static_assert(PIN_HWSERIAL3_RX == PIN_PB1, "USART3 RX is BAND_C");

static_assert(PIN_SPI1_MOSI_PINSWAP_1 == PIN_PC4, "SPI1 ALT1 MOSI");
static_assert(PIN_SPI1_MISO_PINSWAP_1 == PIN_PC5, "SPI1 ALT1 MISO");
static_assert(PIN_SPI1_SCK_PINSWAP_1 == PIN_PC6, "SPI1 ALT1 SCK");
static_assert(PIN_SPI1_SS_PINSWAP_1 == PIN_PC7, "SPI1 ALT1 SS");
static_assert(PIN_WIRE_SDA == PIN_PA2, "TWI0 SDA");
static_assert(PIN_WIRE_SCL == PIN_PA3, "TWI0 SCL");

/*
 * Spec section 2.1 leaves these DxCore numbers blank. The macros still exist.
 * There is no OB2_* GPIO constant for them.
 */
static_assert(PIN_PF0 == 34, "crystal pin PF0");
static_assert(PIN_PF1 == 35, "crystal pin PF1");
static_assert(PIN_PF6 == 40, "RESET pin PF6");

/*
 * DxCore's default LED_BUILTIN is PA7, the J4.1 spare, not SYNC_LED (PC2)
 * and not the TX LED (the PA4 supply net). The stub does not use it.
 */
static_assert(LED_BUILTIN == PIN_PA7, "LED_BUILTIN is the J4 spare");

/*
 * pins_arduino.h comments on digital_pin_to_bit_mask[] call PF2-PF5
 * AIN12-AIN15. PIN_A18..PIN_A21 in that same file are PF2-PF5, and the
 * datasheet calls those AIN18-AIN21. Spec section 2.1 only names AIN9/10/11.
 * The asserts below lock the macros. Do not copy the AIN12-AIN15 comments.
 */
static_assert(PIN_A18 == PIN_PF2, "PF2 analog macro is A18, not A12");
static_assert(PIN_A19 == PIN_PF3, "PF3 analog macro is A19");
static_assert(PIN_A20 == PIN_PF4, "PF4 analog macro is A20");
static_assert(PIN_A21 == PIN_PF5, "PF5 analog macro is A21");
