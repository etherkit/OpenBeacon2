#include "ob2_hold.h"

void ob2_hold_init(Ob2Hold* hold) {
  hold->stable = 0;
  hold->pending = 0;
  hold->since_ms = 0;
  hold->pending_valid = false;
  hold->stable_valid = false;
}

bool ob2_hold_feed(Ob2Hold* hold, uint8_t sample, uint32_t now_ms, uint32_t hold_ms) {
  if (hold->stable_valid && sample == hold->stable) {
    hold->pending_valid = false;
    return false;
  }
  if (!hold->pending_valid || sample != hold->pending) {
    hold->pending = sample;
    hold->since_ms = now_ms;
    hold->pending_valid = true;
    return false;
  }
  if ((now_ms - hold->since_ms) < hold_ms) {
    return false;
  }
  const bool changed = !hold->stable_valid || hold->stable != sample;
  hold->stable = sample;
  hold->stable_valid = true;
  hold->pending_valid = false;
  return changed;
}
