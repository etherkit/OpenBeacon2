#pragma once

/* Non-blocking "sample must hold for N ms" used by buttons and CC. */

#include <stdint.h>

constexpr uint16_t OB2_BUTTON_DEBOUNCE_MS = 25;

struct Ob2Hold {
  uint8_t stable;
  uint8_t pending;
  uint32_t since_ms;
  bool pending_valid;
  bool stable_valid;
};

void ob2_hold_init(Ob2Hold* hold);

/* True when the stable value changes, including the first time it locks. */
bool ob2_hold_feed(Ob2Hold* hold, uint8_t sample, uint32_t now_ms, uint32_t hold_ms);
