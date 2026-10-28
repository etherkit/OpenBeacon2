#pragma once

#include "ob2_power.h"

#include <stdint.h>

enum Ob2Button : uint8_t {
  Ob2BtnUp = 0,
  Ob2BtnDown,
  Ob2BtnLeft,
  Ob2BtnRight,
  Ob2BtnBack,
  Ob2BtnDsp1,
  Ob2BtnDsp2,
  Ob2BtnCount
};

struct Ob2Ui {
  const char* filter_name;
  const char* filter_bands;
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t coils;
  uint16_t load_ma;
  Ob2CcLevel cc_raw;
  Ob2CcLevel cc_stable;
  bool cc_stable_valid;
  uint16_t cc1_mv;
  uint16_t cc2_mv;
  uint16_t isense_ma;
  bool isense_spec;
  bool may_key;
  bool boost;
  bool bias_live;
  bool synth_ok;
  bool display_ok;
  bool eeprom_ok;
  bool eeprom_busy;
  uint8_t buttons;
  uint32_t carrier_hz;
};
