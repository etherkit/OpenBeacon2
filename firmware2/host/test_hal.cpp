#include "doctest.h"

#include "ob2_bus.h"
#include "ob2_dac.h"
#include "ob2_hold.h"
#include "ob2_pins.h"
#include "ob2_power.h"
#include "ob2_pwm.h"
#include "ob2_relays.h"
#include "ob2_softstart.h"

#include <cstring>

TEST_CASE("only four relay codes are legal and none exceed four coils") {
  const uint8_t legal[4][3] = {{0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 0, 1}};
  const uint8_t coils[4] = {0, 2, 2, 4};
  const uint16_t load[4] = {0, 42, 42, 84};
  for (int i = 0; i < 4; ++i) {
    CHECK(ob2_relay_legal(legal[i][0], legal[i][1], legal[i][2]));
    const Ob2Relay r = ob2_relay_from_abc(legal[i][0], legal[i][1], legal[i][2]);
    CHECK(r.legal);
    CHECK(r.coils == coils[i]);
    CHECK(r.load_ma == load[i]);
    CHECK(r.coils <= 4);
  }

  const uint8_t illegal[4][3] = {{0, 0, 1}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1}};
  for (int i = 0; i < 4; ++i) {
    CHECK_FALSE(ob2_relay_legal(illegal[i][0], illegal[i][1], illegal[i][2]));
    const Ob2Relay r = ob2_relay_from_abc(illegal[i][0], illegal[i][1], illegal[i][2]);
    CHECK_FALSE(r.legal);
    CHECK(r.a == 0);
    CHECK(r.b == 0);
    CHECK(r.c == 0);
    CHECK(r.coils == 0);
  }
  CHECK_FALSE(ob2_relay_legal(2, 0, 0));
}

TEST_CASE("filters map to the section 2.2 table") {
  const Ob2Relay f2 = ob2_relay_for_filter(Ob2Filter::F2);
  const Ob2Relay f3 = ob2_relay_for_filter(Ob2Filter::F3);
  const Ob2Relay f4 = ob2_relay_for_filter(Ob2Filter::F4);
  const Ob2Relay f5 = ob2_relay_for_filter(Ob2Filter::F5);
  CHECK(f2.a == 0);
  CHECK(f2.b == 0);
  CHECK(f2.c == 0);
  CHECK(f3.a == 0);
  CHECK(f3.b == 1);
  CHECK(f3.c == 0);
  CHECK(f4.a == 1);
  CHECK(f4.b == 0);
  CHECK(f4.c == 0);
  CHECK(f5.a == 1);
  CHECK(f5.b == 0);
  CHECK(f5.c == 1);
}

TEST_CASE("every band lands on its filter or is refused") {
  struct Row {
    Ob2Band band;
    bool has;
    Ob2Filter filter;
  };
  const Row rows[] = {
      {Ob2Band::M2200, false, Ob2Filter::F2}, {Ob2Band::M630, false, Ob2Filter::F2},
      {Ob2Band::M160, false, Ob2Filter::F2},  {Ob2Band::M6, false, Ob2Filter::F2},
      {Ob2Band::M2, false, Ob2Filter::F2},    {Ob2Band::M80, true, Ob2Filter::F2},
      {Ob2Band::M60, true, Ob2Filter::F2},    {Ob2Band::M40, true, Ob2Filter::F3},
      {Ob2Band::M30, true, Ob2Filter::F3},    {Ob2Band::M20, true, Ob2Filter::F4},
      {Ob2Band::M17, true, Ob2Filter::F4},    {Ob2Band::M15, true, Ob2Filter::F5},
      {Ob2Band::M12, true, Ob2Filter::F5},    {Ob2Band::M10, true, Ob2Filter::F5},
  };
  CHECK(sizeof(rows) / sizeof(rows[0]) == OB2_BAND_COUNT);
  for (const Row& row : rows) {
    Ob2Filter got = Ob2Filter::F2;
    CHECK(ob2_filter_for_band(row.band, &got) == row.has);
    if (row.has) {
      CHECK(got == row.filter);
    }
  }
}

TEST_CASE("fixed carriers are the WSPR dials inside each filter") {
  CHECK(ob2_filter_carrier_hz(Ob2Filter::F2) == 3568600UL);
  CHECK(ob2_filter_carrier_hz(Ob2Filter::F3) == 7038600UL);
  CHECK(ob2_filter_carrier_hz(Ob2Filter::F4) == 14095600UL);
  CHECK(ob2_filter_carrier_hz(Ob2Filter::F5) == 21094600UL);
  CHECK(std::strcmp(ob2_filter_bands(Ob2Filter::F2), "80/60m") == 0);
  CHECK(std::strcmp(ob2_filter_bands(Ob2Filter::F5), "15/12/10m") == 0);
}

TEST_CASE("soft-start refuses keying before 50 ms or the first CC sample") {
  CHECK_FALSE(ob2_may_enable_boost(0, true));
  CHECK_FALSE(ob2_may_enable_boost(49, true));
  CHECK_FALSE(ob2_may_enable_boost(50, false));
  CHECK_FALSE(ob2_may_enable_boost(1000, false));
  CHECK(ob2_may_enable_boost(50, true));
  CHECK(ob2_may_enable_boost(51, true));
}

TEST_CASE("bias ramp waits 20 ms after the boost is enabled") {
  CHECK_FALSE(ob2_may_ramp_bias(false, 100, 0));
  CHECK_FALSE(ob2_may_ramp_bias(true, 0, 0));
  CHECK_FALSE(ob2_may_ramp_bias(true, 19, 0));
  CHECK(ob2_may_ramp_bias(true, 20, 0));
  CHECK(ob2_may_ramp_bias(true, 70, 50));
  CHECK_FALSE(ob2_may_ramp_bias(true, 69, 50));
  CHECK(ob2_may_ramp_bias(true, 10, 0xFFFFFFF0UL));
}

TEST_CASE("ADC counts use a 2.5 V 12-bit scale") {
  CHECK(ob2_adc_counts_to_mv(0) == 0);
  CHECK(ob2_adc_counts_to_mv(1) == 0);
  CHECK(ob2_adc_counts_to_mv(2) == 1);
  CHECK(ob2_adc_counts_to_mv(4095) == 2499);
  CHECK(ob2_adc_counts_to_mv(4096) == 2499);
}

TEST_CASE("CC thresholds use the higher pin and both orientations") {
  CHECK(ob2_cc_classify_mv(0, 0) == Ob2CcLevel::None);
  CHECK(ob2_cc_classify_mv(199, 199) == Ob2CcLevel::None);
  CHECK(ob2_cc_classify_mv(200, 0) == Ob2CcLevel::Default);
  CHECK(ob2_cc_classify_mv(0, 200) == Ob2CcLevel::Default);
  CHECK(ob2_cc_classify_mv(659, 10) == Ob2CcLevel::Default);
  CHECK(ob2_cc_classify_mv(10, 659) == Ob2CcLevel::Default);
  CHECK(ob2_cc_classify_mv(660, 0) == Ob2CcLevel::A1500);
  CHECK(ob2_cc_classify_mv(0, 660) == Ob2CcLevel::A1500);
  CHECK(ob2_cc_classify_mv(1229, 100) == Ob2CcLevel::A1500);
  CHECK(ob2_cc_classify_mv(100, 1229) == Ob2CcLevel::A1500);
  CHECK(ob2_cc_classify_mv(1230, 0) == Ob2CcLevel::A3000);
  CHECK(ob2_cc_classify_mv(0, 1230) == Ob2CcLevel::A3000);
  CHECK(ob2_cc_classify_mv(2500, 100) == Ob2CcLevel::A3000);
  CHECK(std::strcmp(ob2_cc_name(Ob2CcLevel::None), "none") == 0);
  CHECK(std::strcmp(ob2_cc_name(Ob2CcLevel::A1500), "1.5A") == 0);
}

TEST_CASE("CC debounce holds a new level for 200 ms") {
  Ob2Hold hold;
  ob2_hold_init(&hold);
  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::Default), 0, OB2_CC_DEBOUNCE_MS));
  CHECK_FALSE(hold.stable_valid);
  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::Default), 199, OB2_CC_DEBOUNCE_MS));
  CHECK_FALSE(hold.stable_valid);
  CHECK(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::Default), 200, OB2_CC_DEBOUNCE_MS));
  CHECK(hold.stable == static_cast<uint8_t>(Ob2CcLevel::Default));

  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::A1500), 200, OB2_CC_DEBOUNCE_MS));
  CHECK(hold.stable == static_cast<uint8_t>(Ob2CcLevel::Default));
  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::A1500), 399, OB2_CC_DEBOUNCE_MS));
  CHECK(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::A1500), 400, OB2_CC_DEBOUNCE_MS));
  CHECK(hold.stable == static_cast<uint8_t>(Ob2CcLevel::A1500));

  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::A3000), 400, OB2_CC_DEBOUNCE_MS));
  CHECK_FALSE(ob2_hold_feed(&hold, static_cast<uint8_t>(Ob2CcLevel::Default), 500, OB2_CC_DEBOUNCE_MS));
  CHECK(hold.stable == static_cast<uint8_t>(Ob2CcLevel::A1500));
}

TEST_CASE("button edge requires the debounce interval") {
  Ob2Hold hold;
  ob2_hold_init(&hold);
  CHECK_FALSE(ob2_hold_feed(&hold, 1, 0, OB2_BUTTON_DEBOUNCE_MS));
  CHECK_FALSE(ob2_hold_feed(&hold, 1, 24, OB2_BUTTON_DEBOUNCE_MS));
  CHECK(ob2_hold_feed(&hold, 1, 25, OB2_BUTTON_DEBOUNCE_MS));
  CHECK(hold.stable == 1);
  CHECK_FALSE(ob2_hold_feed(&hold, 1, 80, OB2_BUTTON_DEBOUNCE_MS));
}

TEST_CASE("isense is millivolts over 4 and unspecified below 100 mA") {
  CHECK(ob2_isense_ma(0) == 0);
  CHECK_FALSE(ob2_isense_specified(ob2_isense_ma(396)));
  CHECK(ob2_isense_ma(396) == 99);
  CHECK(ob2_isense_ma(400) == 100);
  CHECK(ob2_isense_specified(100));
  CHECK(ob2_isense_ma(1400) == 350);
  CHECK(ob2_isense_ma(2500) == 625);
}

TEST_CASE("MCP4725 code matches mV times 4096 over 5000") {
  CHECK(OB2_MCP4725_ADDR == 0x62);
  CHECK(ob2_mcp4725_code(0) == 0);
  CHECK(ob2_mcp4725_code(500) == 409);
  CHECK(ob2_mcp4725_code(2000) == 1638);
  CHECK(ob2_mcp4725_code(5000) == 4095);
  CHECK(ob2_mcp4725_code(6000) == 4095);
  CHECK(OB2_BIAS_BRINGUP_MV == 500);
}

TEST_CASE("EEPROM bring-up address is the SOT-23 part with no address pins") {
  CHECK(OB2_EEPROM_ADDR == 0x50);
  CHECK(OB2_SI5351_ADDR == 0x60);
  CHECK(OB2_EEPROM_MAGIC_LEN == 4);
  CHECK(OB2_EEPROM_MAGIC[0] == 'O');
  CHECK(OB2_EEPROM_MAGIC[1] == 'B');
  CHECK(OB2_EEPROM_MAGIC[2] == '2');
  CHECK(OB2_EEPROM_MAGIC[3] == '1');
  CHECK(OB2_TWI_HZ == 100000UL);
}

TEST_CASE("datasheet timer check keeps relay PWM out of the firmware") {
  CHECK_FALSE(OB2_RELAY_PWM_SHIPPED);
  CHECK_FALSE(OB2_TCA0_SERVES_PA5_AND_PB1);
  CHECK_FALSE(OB2_TCB_DRIVES_RELAY_PINS);
  CHECK(OB2_TIMER_PIN_COUNT == 8);

  bool pa5_tca = false;
  bool pa5_tcd = false;
  bool pa6_tcd = false;
  bool pa6_other = false;
  bool pb1_tca1 = false;
  bool pb1_tca0 = false;
  bool pa4_woa = false;
  bool pb0_tca1 = false;
  for (uint8_t i = 0; i < OB2_TIMER_PIN_COUNT; ++i) {
    const Ob2TimerPin& row = OB2_TIMER_PINS[i];
    CHECK(std::strncmp(row.timer, "TCB", 3) != 0);
    if (row.port == 'A' && row.bit == 5 && std::strcmp(row.timer, "TCA0") == 0) {
      pa5_tca = true;
      CHECK(std::strcmp(row.signal, "WO5") == 0);
      CHECK(std::strcmp(row.portmux, "PORTA") == 0);
    }
    if (row.port == 'A' && row.bit == 5 && std::strcmp(row.timer, "TCD0") == 0) {
      pa5_tcd = true;
      CHECK(std::strcmp(row.signal, "WOB") == 0);
    }
    if (row.port == 'A' && row.bit == 6) {
      CHECK(std::strcmp(row.timer, "TCD0") == 0);
      CHECK(std::strcmp(row.signal, "WOC") == 0);
      pa6_tcd = true;
      pa6_other = std::strcmp(row.timer, "TCD0") != 0;
    }
    if (row.port == 'B' && row.bit == 1 && std::strcmp(row.timer, "TCA1") == 0) {
      pb1_tca1 = true;
      CHECK(std::strcmp(row.signal, "WO1") == 0);
    }
    if (row.port == 'B' && row.bit == 1 && std::strcmp(row.timer, "TCA0") == 0) {
      pb1_tca0 = true;
      CHECK(std::strcmp(row.portmux, "PORTB") == 0);
    }
    if (row.port == 'A' && row.bit == 4) {
      pa4_woa = true;
      CHECK_FALSE(row.bench_candidate);
      CHECK(std::strcmp(row.signal, "WOA") == 0);
    }
    if (row.port == 'B' && row.bit == 0 && std::strcmp(row.timer, "TCA1") == 0) {
      pb0_tca1 = true;
      CHECK_FALSE(row.bench_candidate);
      CHECK(std::strcmp(row.signal, "WO0") == 0);
    }
  }
  CHECK(pa5_tca);
  CHECK(pa5_tcd);
  CHECK(pa6_tcd);
  CHECK_FALSE(pa6_other);
  CHECK(pb1_tca1);
  CHECK(pb1_tca0);
  CHECK(pa4_woa);
  CHECK(pb0_tca1);
}

TEST_CASE("stale spec lines stay visible on the pin rows") {
  const char* band_b = nullptr;
  const char* band_c = nullptr;
  for (const Ob2Pin& pin : OB2_PINS) {
    if (pin.port == 'A' && pin.bit == 6) {
      band_b = pin.role;
    }
    if (pin.port == 'B' && pin.bit == 1) {
      band_c = pin.role;
    }
  }
  REQUIRE(band_b != nullptr);
  REQUIRE(band_c != nullptr);
  CHECK(std::strstr(band_b, "K2/K5") != nullptr);
  CHECK(std::strstr(band_b, "K2/K3/K5/K6") != nullptr);
  CHECK(std::strstr(band_c, "pull-down") != nullptr);
  CHECK(std::strstr(band_c, "RXEN") != nullptr);
  CHECK(std::strstr(band_c, "pull-up") == nullptr);
}
