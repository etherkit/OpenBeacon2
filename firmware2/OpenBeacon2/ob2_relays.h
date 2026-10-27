#pragma once

/*
 * Section 2.2 filter codes. Only 000, 010, 100, and 101 are legal.
 * 001, 011, 110, and 111 are never driven. At most four coils.
 * Release firmware drives these as GPIO, fully on. See ob2_pwm.h.
 */

#include <stdint.h>

enum class Ob2Filter : uint8_t { F2 = 0, F3 = 1, F4 = 2, F5 = 3 };

enum class Ob2Band : uint8_t {
  M2200 = 0,
  M630,
  M160,
  M80,
  M60,
  M40,
  M30,
  M20,
  M17,
  M15,
  M12,
  M10,
  M6,
  M2
};

constexpr uint8_t OB2_BAND_COUNT = 14;
constexpr uint8_t OB2_FILTER_COUNT = 4;
constexpr uint8_t OB2_RELAY_SETTLE_MS = 10;

struct Ob2Relay {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t coils;
  uint16_t load_ma;
  bool legal;
};

bool ob2_relay_legal(uint8_t a, uint8_t b, uint8_t c);
Ob2Relay ob2_relay_from_abc(uint8_t a, uint8_t b, uint8_t c);
Ob2Relay ob2_relay_for_filter(Ob2Filter filter);

/* False when the band has no filter. Transmit is refused in that case. */
bool ob2_filter_for_band(Ob2Band band, Ob2Filter* out);

const char* ob2_filter_name(Ob2Filter filter);
const char* ob2_filter_bands(Ob2Filter filter);

/* Fixed bring-up carrier, inside that filter's band list. Hertz. */
uint32_t ob2_filter_carrier_hz(Ob2Filter filter);
