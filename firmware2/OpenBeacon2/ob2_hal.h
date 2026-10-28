#pragma once

#include <Arduino.h>

#include "ob2_bus.h"
#include "ob2_dac.h"
#include "ob2_hold.h"
#include "ob2_pins.h"
#include "ob2_power.h"
#include "ob2_relays.h"
#include "ob2_softstart.h"
#include "ob2_ui.h"

void ob2_hal_begin();
bool ob2_display_begin();

bool ob2_display_ok();
bool ob2_synth_ok();
bool ob2_eeprom_ok();
bool ob2_eeprom_busy();
bool ob2_cc_sampled();
bool ob2_boost_is_on();

void ob2_eeprom_poll(uint32_t now_ms);
void ob2_relays_write(Ob2Relay relay);
void ob2_boost(bool on);
void ob2_bias_mv(uint16_t mv);
void ob2_carrier(uint32_t hz, bool on);
bool ob2_button_raw(uint8_t index);
void ob2_sync(bool on);
void ob2_adc_sample(uint16_t* isense_counts, uint16_t* cc1_counts, uint16_t* cc2_counts);
void ob2_display(const Ob2Ui& ui);
