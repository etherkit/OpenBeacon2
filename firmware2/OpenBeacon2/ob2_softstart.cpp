#include "ob2_softstart.h"

bool ob2_may_enable_boost(uint32_t now_ms, bool cc_sampled) {
  return cc_sampled && now_ms >= OB2_BOOT_KEY_DELAY_MS;
}

bool ob2_may_ramp_bias(bool boost_on, uint32_t now_ms, uint32_t boost_on_ms) {
  if (!boost_on) {
    return false;
  }
  return (now_ms - boost_on_ms) >= OB2_PA_SETTLE_MS;
}
