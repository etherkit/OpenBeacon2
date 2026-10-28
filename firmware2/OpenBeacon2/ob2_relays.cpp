#include "ob2_relays.h"

bool ob2_relay_legal(uint8_t a, uint8_t b, uint8_t c) {
  if (a > 1 || b > 1 || c > 1) {
    return false;
  }
  if (c == 1 && a == 0) {
    return false; /* 001 and 011 */
  }
  if (b == 1 && a == 1) {
    return false; /* 110 and 111, the x11 codes with A on */
  }
  if (b == 1 && c == 1) {
    return false;
  }
  return true;
}

Ob2Relay ob2_relay_from_abc(uint8_t a, uint8_t b, uint8_t c) {
  Ob2Relay r = {0, 0, 0, 0, 0, false};
  if (!ob2_relay_legal(a, b, c)) {
    return r;
  }
  r.a = a;
  r.b = b;
  r.c = c;
  r.legal = true;
  if (a == 0 && b == 0 && c == 0) {
    r.coils = 0;
    r.load_ma = 0;
  } else if (c == 1) {
    r.coils = 4; /* K1 K4 K3 K6 */
    r.load_ma = 84;
  } else {
    r.coils = 2;
    r.load_ma = 42;
  }
  return r;
}

Ob2Relay ob2_relay_for_filter(Ob2Filter filter) {
  switch (filter) {
    case Ob2Filter::F2:
      return ob2_relay_from_abc(0, 0, 0);
    case Ob2Filter::F3:
      return ob2_relay_from_abc(0, 1, 0);
    case Ob2Filter::F4:
      return ob2_relay_from_abc(1, 0, 0);
    case Ob2Filter::F5:
      return ob2_relay_from_abc(1, 0, 1);
  }
  return ob2_relay_from_abc(2, 0, 0);
}

bool ob2_filter_for_band(Ob2Band band, Ob2Filter* out) {
  Ob2Filter filter = Ob2Filter::F2;
  switch (band) {
    case Ob2Band::M80:
    case Ob2Band::M60:
      filter = Ob2Filter::F2;
      break;
    case Ob2Band::M40:
    case Ob2Band::M30:
      filter = Ob2Filter::F3;
      break;
    case Ob2Band::M20:
    case Ob2Band::M17:
      filter = Ob2Filter::F4;
      break;
    case Ob2Band::M15:
    case Ob2Band::M12:
    case Ob2Band::M10:
      filter = Ob2Filter::F5;
      break;
    case Ob2Band::M2200:
    case Ob2Band::M630:
    case Ob2Band::M160:
    case Ob2Band::M6:
    case Ob2Band::M2:
      return false;
  }
  if (out != nullptr) {
    *out = filter;
  }
  return true;
}

const char* ob2_filter_name(Ob2Filter filter) {
  switch (filter) {
    case Ob2Filter::F2:
      return "F2";
    case Ob2Filter::F3:
      return "F3";
    case Ob2Filter::F4:
      return "F4";
    case Ob2Filter::F5:
      return "F5";
  }
  return "?";
}

const char* ob2_filter_bands(Ob2Filter filter) {
  switch (filter) {
    case Ob2Filter::F2:
      return "80/60m";
    case Ob2Filter::F3:
      return "40/30m";
    case Ob2Filter::F4:
      return "20/17m";
    case Ob2Filter::F5:
      return "15/12/10m";
  }
  return "?";
}

uint32_t ob2_filter_carrier_hz(Ob2Filter filter) {
  /* WSPR dial frequencies. Each one sits in that filter's band list. */
  switch (filter) {
    case Ob2Filter::F2:
      return 3568600UL;
    case Ob2Filter::F3:
      return 7038600UL;
    case Ob2Filter::F4:
      return 14095600UL;
    case Ob2Filter::F5:
      return 21094600UL;
  }
  return 0;
}
