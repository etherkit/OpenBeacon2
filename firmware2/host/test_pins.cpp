/*
 * Host check of the pin header against spec draft 3.5 sections 2.1 and 2.2.
 * Identity fields are listed here separately from ob2_pins.h. Role text is
 * only spot-checked, so wording can move without hiding a wrong net.
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ob2_pins.h"

#include <cstring>

struct Expect {
  uint8_t package_pin;
  char port;
  int8_t bit;
  int8_t dxcore;
  int8_t ain;
  const char *net;
  uint8_t flags;
};

/* Section 2.1 rows, section 2.2 nets on PB1 and PE1-PE3, power footnote. */
static const Expect kSpec[] = {
    {1, 'A', 5, 5, -1, "BAND_A", OB2_PIN_OUT},
    {2, 'A', 6, 6, -1, "BAND_B", OB2_PIN_OUT},
    {3, 'A', 7, 7, -1, "D7_PA7", OB2_PIN_SPARE},
    {4, 'B', 0, 8, -1, "GPS_RXD", 0},
    {5, 'B', 1, 9, -1, "BAND_C", OB2_PIN_OUT | OB2_PIN_REWORK},
    {6, 'B', 2, 10, -1, "CAL_CLK", OB2_PIN_IN},
    {7, 'B', 3, 11, -1, "SI_INTR", OB2_PIN_IN},
    {8, 'B', 4, 12, -1, "OLED_DC", OB2_PIN_OUT},
    {9, 'B', 5, 13, -1, "OLED_RES", OB2_PIN_OUT},
    {10, 'C', 0, 14, -1, "UART_TX", 0},
    {11, 'C', 1, 15, -1, "UART_RX", 0},
    {12, 'C', 2, 16, -1, "SYNC_LED", OB2_PIN_OUT},
    {13, 'C', 3, 17, -1, "J4.3", OB2_PIN_SPARE},
    {14, 0, -1, -1, -1, "VDD", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {15, 0, -1, -1, -1, "GND", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {16, 'C', 4, 18, -1, "OLED_MOSI", 0},
    {17, 'C', 5, 19, -1, "J4.5", OB2_PIN_SPARE},
    {18, 'C', 6, 20, -1, "OLED_SCK", 0},
    {19, 'C', 7, 21, -1, "OLED_CS", OB2_PIN_OUT},
    {20, 'D', 0, 22, 0, "J5", OB2_PIN_SPARE | OB2_PIN_ADC},
    {21, 'D', 1, 23, 1, "J5", OB2_PIN_SPARE | OB2_PIN_ADC},
    {22, 'D', 2, 24, 2, "J5", OB2_PIN_SPARE | OB2_PIN_ADC},
    {23, 'D', 3, 25, -1, "BTN_UP", OB2_PIN_IN},
    {24, 'D', 4, 26, -1, "BTN_DOWN", OB2_PIN_IN},
    {25, 'D', 5, 27, -1, "BTN_LEFT", OB2_PIN_IN},
    {26, 'D', 6, 28, -1, "BTN_RIGHT", OB2_PIN_IN},
    {27, 'D', 7, 29, -1, "BTN_BACK", OB2_PIN_IN},
    {28, 0, -1, -1, -1, "AVDD", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {29, 0, -1, -1, -1, "GND", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {30, 'E', 0, 30, -1, "BTN_DSP_1", OB2_PIN_IN},
    {31, 'E', 1, 31, 9, "TX_ISENSE", OB2_PIN_ADC | OB2_PIN_REWORK},
    {32, 'E', 2, 32, 10, "CC1_SENSE", OB2_PIN_ADC | OB2_PIN_REWORK},
    {33, 'E', 3, 33, 11, "CC2_SENSE", OB2_PIN_ADC | OB2_PIN_REWORK},
    {34, 'F', 0, -1, -1, "Y1", OB2_PIN_NO_GPIO},
    {35, 'F', 1, -1, -1, "Y1", OB2_PIN_NO_GPIO},
    {36, 'F', 2, 36, -1, "GPS_PPS", OB2_PIN_IN},
    {37, 'F', 3, 37, -1, "BTN_DSP_2", OB2_PIN_IN},
    {38, 'F', 4, 38, -1, "J4.7", OB2_PIN_SPARE},
    {39, 'F', 5, 39, -1, "GPS_TXD", 0},
    {40, 'F', 6, -1, -1, "RESET", OB2_PIN_NO_GPIO},
    {41, 0, -1, -1, -1, "J13", OB2_PIN_NO_GPIO},
    {42, 0, -1, -1, -1, "VDD", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {43, 0, -1, -1, -1, "GND", OB2_PIN_POWER | OB2_PIN_NO_GPIO},
    {44, 'A', 0, 0, -1, "MCU_TX", 0},
    {45, 'A', 1, 1, -1, "MCU_RX", 0},
    {46, 'A', 2, 2, -1, "SDA", 0},
    {47, 'A', 3, 3, -1, "SCL", 0},
    {48, 'A', 4, 4, -1, "TX", OB2_PIN_OUT},
};

TEST_CASE("power rework disclaimer says the hardware is not committed") {
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "not committed") != nullptr);
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "BAND_C") != nullptr);
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "TX_ISENSE") != nullptr);
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "CC1_SENSE") != nullptr);
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "CC2_SENSE") != nullptr);
  CHECK(std::strstr(OB2_POWER_REWORK_NOTE, "88e9a62") != nullptr);
}

TEST_CASE("pin table matches spec 2.1 with the 2.2 rework nets") {
  REQUIRE(OB2_PIN_COUNT == 48);
  REQUIRE(sizeof(kSpec) / sizeof(kSpec[0]) == OB2_PIN_COUNT);

  bool seen[49] = {};
  for (const Expect &e : kSpec) {
    const Ob2Pin *pin = ob2_pin_at(e.package_pin);
    REQUIRE(pin != nullptr);
    CHECK(pin->port == e.port);
    CHECK(pin->bit == e.bit);
    CHECK(pin->dxcore == e.dxcore);
    CHECK(pin->ain == e.ain);
    CHECK(std::strcmp(pin->net, e.net) == 0);
    CHECK(pin->flags == e.flags);
    CHECK(e.package_pin >= 1);
    CHECK(e.package_pin <= 48);
    CHECK_FALSE(seen[e.package_pin]);
    seen[e.package_pin] = true;
  }
  for (uint8_t n = 1; n <= 48; ++n) {
    CHECK(seen[n]);
  }
}

TEST_CASE("section 2.2 constraints on USART, CLK4, and the sense pins") {
  const Ob2Pin *band_c = ob2_pin_at(5);
  const Ob2Pin *tx_isense = ob2_pin_at(31);
  const Ob2Pin *cc1 = ob2_pin_at(32);
  const Ob2Pin *cc2 = ob2_pin_at(33);
  REQUIRE(band_c != nullptr);
  REQUIRE(tx_isense != nullptr);
  REQUIRE(cc1 != nullptr);
  REQUIRE(cc2 != nullptr);

  CHECK(std::strcmp(band_c->net, "BAND_C") == 0);
  CHECK(std::strstr(band_c->role, "RXEN") != nullptr);
  CHECK(std::strstr(band_c->role, "88e9a62") != nullptr);
  CHECK(std::strcmp(tx_isense->net, "TX_ISENSE") == 0);
  CHECK(tx_isense->ain == 9);
  CHECK(std::strstr(tx_isense->role, "D31_A9_PE1") != nullptr);
  CHECK(std::strcmp(cc1->net, "CC1_SENSE") == 0);
  CHECK(cc1->ain == 10);
  CHECK(std::strcmp(cc2->net, "CC2_SENSE") == 0);
  CHECK(cc2->ain == 11);

  CHECK(OB2_USART2_RX == OB2_GPS_TXD);
  CHECK(OB2_USART2_PORTMUX_ALT1 == 1);
  CHECK(OB2_USART2_ALT1_TX_DO_NOT_USE == OB2_J4_PF4);
  CHECK(std::strstr(ob2_pin_at(39)->role, "ALT1") != nullptr);
  CHECK(std::strstr(ob2_pin_at(39)->role, "RX only") != nullptr);
  CHECK(std::strstr(ob2_pin_at(38)->role, "USART2") != nullptr);

  CHECK(OB2_USART3_TX == OB2_GPS_RXD);
  CHECK(OB2_USART3_RX_DISABLED == OB2_BAND_C);
  CHECK(OB2_USART3_PORTMUX_DEFAULT == 0);
  CHECK(std::strstr(ob2_pin_at(4)->role, "USART3") != nullptr);

  CHECK(OB2_CLK4_CAL == OB2_CAL_CLK);
  CHECK(std::strcmp(ob2_pin_at(6)->net, "CAL_CLK") == 0);

  CHECK(OB2_SPI1_MOSI == OB2_OLED_MOSI);
  CHECK(OB2_SPI1_MISO == OB2_J4_PC5);
  CHECK(OB2_SPI1_SCK == OB2_OLED_SCK);
  CHECK(OB2_SPI1_CS == OB2_OLED_CS);
  CHECK(std::strstr(ob2_pin_at(17)->role, "MISO") != nullptr);
  CHECK(std::strstr(ob2_pin_at(27)->role, "VREFA") != nullptr);

  /* Section 2.1's BAND_B cell still lists K2/K3/K5/K6. Section 2.2 does not. */
  CHECK(std::strstr(ob2_pin_at(2)->role, "K2/K5") != nullptr);
  CHECK(std::strstr(ob2_pin_at(2)->role, "K2/K3/K5/K6") != nullptr);
  CHECK(std::strstr(ob2_pin_at(1)->role, "K1/K4") != nullptr);
  CHECK(std::strstr(ob2_pin_at(5)->role, "K3/K6") != nullptr);
}
