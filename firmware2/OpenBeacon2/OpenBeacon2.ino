/*
 * OpenBeacon 2 firmware2, milestone M1 bring-up.
 *
 * Optiboot image and fuse notes are in firmware2/README.md (Bench 1).
 * This sketch does not burn fuses. It does not call delay(). Relays are
 * GPIO, fully on. PA4 stays low until DSP1, and only after the 50 ms
 * power-up gate and a CC sample. Bias and CLK0 wait until the boost has
 * been on for 20 ms. Changing filter or pressing BACK/DSP2 drops the
 * carrier before the relays move.
 *
 * Clock: FQBN clock=24internal still does not set OSCHF autotune.
 * millis stays on TCB2. USART2 and USART3 stay off (GPS is later).
 */

#include "ob2_hal.h"

enum class Carrier : uint8_t { Off, BoostWait, On };

static Carrier g_carrier = Carrier::Off;
static uint8_t g_filter = 0;
static uint32_t g_boost_ms = 0;
static uint32_t g_last_adc_ms = 0;
static uint32_t g_last_draw_ms = 0;
static bool g_draw = true;
static uint16_t g_isense_counts = 0;
static uint16_t g_cc1_counts = 0;
static uint16_t g_cc2_counts = 0;
static Ob2Hold g_cc_hold;
static Ob2Hold g_btn[Ob2BtnCount];

static Ob2Filter filter_now() {
  return static_cast<Ob2Filter>(g_filter);
}

static void stop_carrier() {
  ob2_bias_mv(0);
  ob2_carrier(0, false);
  ob2_boost(false);
  g_carrier = Carrier::Off;
}

static void apply_filter(uint8_t index) {
  if (g_carrier != Carrier::Off) {
    stop_carrier();
  }
  g_filter = static_cast<uint8_t>(index % OB2_FILTER_COUNT);
  ob2_relays_write(ob2_relay_for_filter(filter_now()));
  g_draw = true;
}

static void request_carrier(uint32_t now_ms) {
  if (!ob2_synth_ok()) {
    return;
  }
  if (!ob2_may_enable_boost(now_ms, ob2_cc_sampled())) {
    return;
  }
  if (g_carrier != Carrier::Off) {
    return;
  }
  ob2_boost(true);
  g_boost_ms = now_ms;
  g_carrier = Carrier::BoostWait;
  g_draw = true;
}

static void on_press(uint8_t button, uint32_t now_ms) {
  if (button == Ob2BtnUp) {
    apply_filter(static_cast<uint8_t>(g_filter + 1));
  } else if (button == Ob2BtnDown) {
    apply_filter(static_cast<uint8_t>(g_filter + OB2_FILTER_COUNT - 1));
  } else if (button == Ob2BtnDsp1) {
    request_carrier(now_ms);
  } else if (button == Ob2BtnBack || button == Ob2BtnDsp2) {
    if (g_carrier != Carrier::Off) {
      stop_carrier();
      g_draw = true;
    }
  }
}

static void service_carrier(uint32_t now_ms) {
  if (g_carrier != Carrier::BoostWait) {
    return;
  }
  if (!ob2_may_ramp_bias(ob2_boost_is_on(), now_ms, g_boost_ms)) {
    return;
  }
  const uint32_t hz = ob2_filter_carrier_hz(filter_now());
  ob2_bias_mv(OB2_BIAS_BRINGUP_MV);
  ob2_carrier(hz, true);
  g_carrier = Carrier::On;
  g_draw = true;
}

static Ob2Ui make_ui(uint32_t now_ms) {
  const Ob2Relay relay = ob2_relay_for_filter(filter_now());
  const uint16_t cc1_mv = ob2_adc_counts_to_mv(g_cc1_counts);
  const uint16_t cc2_mv = ob2_adc_counts_to_mv(g_cc2_counts);
  const uint16_t sense_mv = ob2_adc_counts_to_mv(g_isense_counts);
  const uint16_t ma = ob2_isense_ma(sense_mv);
  Ob2Ui ui = {};
  ui.filter_name = ob2_filter_name(filter_now());
  ui.filter_bands = ob2_filter_bands(filter_now());
  ui.a = relay.a;
  ui.b = relay.b;
  ui.c = relay.c;
  ui.coils = relay.coils;
  ui.load_ma = relay.load_ma;
  ui.cc_raw = ob2_cc_classify_mv(cc1_mv, cc2_mv);
  ui.cc_stable = static_cast<Ob2CcLevel>(g_cc_hold.stable);
  ui.cc_stable_valid = g_cc_hold.stable_valid;
  ui.cc1_mv = cc1_mv;
  ui.cc2_mv = cc2_mv;
  ui.isense_ma = ma;
  ui.isense_spec = ob2_isense_specified(ma);
  ui.may_key = ob2_may_enable_boost(now_ms, ob2_cc_sampled());
  ui.boost = g_carrier != Carrier::Off;
  ui.bias_live = g_carrier == Carrier::On;
  ui.synth_ok = ob2_synth_ok();
  ui.display_ok = ob2_display_ok();
  ui.eeprom_ok = ob2_eeprom_ok();
  ui.eeprom_busy = ob2_eeprom_busy();
  ui.carrier_hz = ob2_filter_carrier_hz(filter_now());
  ui.buttons = 0;
  for (uint8_t i = 0; i < Ob2BtnCount; ++i) {
    if (ob2_button_raw(i)) {
      ui.buttons = static_cast<uint8_t>(ui.buttons | (1u << i));
    }
  }
  return ui;
}

void setup() {
  ob2_hal_begin();
  ob2_hold_init(&g_cc_hold);
  for (uint8_t i = 0; i < Ob2BtnCount; ++i) {
    ob2_hold_init(&g_btn[i]);
  }
  apply_filter(0);
  g_last_adc_ms = millis();
  ob2_adc_sample(&g_isense_counts, &g_cc1_counts, &g_cc2_counts);
  const Ob2CcLevel raw = ob2_cc_classify_mv(ob2_adc_counts_to_mv(g_cc1_counts),
                                            ob2_adc_counts_to_mv(g_cc2_counts));
  ob2_hold_feed(&g_cc_hold, static_cast<uint8_t>(raw), g_last_adc_ms, OB2_CC_DEBOUNCE_MS);
}

void loop() {
  const uint32_t now_ms = millis();
  ob2_eeprom_poll(now_ms);

  if ((now_ms - g_last_adc_ms) >= 50) {
    g_last_adc_ms = now_ms;
    ob2_adc_sample(&g_isense_counts, &g_cc1_counts, &g_cc2_counts);
    const Ob2CcLevel raw = ob2_cc_classify_mv(ob2_adc_counts_to_mv(g_cc1_counts),
                                              ob2_adc_counts_to_mv(g_cc2_counts));
    if (ob2_hold_feed(&g_cc_hold, static_cast<uint8_t>(raw), now_ms, OB2_CC_DEBOUNCE_MS)) {
      g_draw = true;
    }
  }

  bool held = false;
  for (uint8_t i = 0; i < Ob2BtnCount; ++i) {
    const bool down = ob2_button_raw(i);
    held = held || down;
    if (ob2_hold_feed(&g_btn[i], down ? 1 : 0, now_ms, OB2_BUTTON_DEBOUNCE_MS) &&
        g_btn[i].stable == 1) {
      on_press(i, now_ms);
    }
  }
  ob2_sync(held || ((now_ms / 500U) & 1U) != 0);

  service_carrier(now_ms);

  if (g_draw || (now_ms - g_last_draw_ms) >= 200) {
    g_last_draw_ms = now_ms;
    g_draw = false;
    ob2_display(make_ui(now_ms));
  }
}
