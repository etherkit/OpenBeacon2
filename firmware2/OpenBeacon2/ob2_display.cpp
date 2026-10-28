#include "ob2_hal.h"

#include <SPI.h>
#include <U8g2lib.h>

/*
 * SH1106 128x64 on SPI1 ALT1 (PC4 MOSI, PC6 SCK, PC7 CS as GPIO, PB4 DC, PB5 RES).
 * DxCore sets SPI_INTERFACES_COUNT to 1, so U8g2's 2nd-HW-SPI class compiles to a
 * no-op. The one SPI object is swapped onto SPI1 ALT1 before begin(). SPI0 default
 * pins are PA4-PA7 and must not be started.
 */

static U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI g_u8g2(U8G2_R0, OB2_OLED_CS, OB2_OLED_DC, OB2_OLED_RES);
static bool g_display_ok = false;

bool ob2_display_ok() {
  return g_display_ok;
}

bool ob2_display_begin() {
  pinMode(OB2_OLED_CS, OUTPUT);
  digitalWrite(OB2_OLED_CS, HIGH);
  pinMode(OB2_OLED_DC, OUTPUT);
  pinMode(OB2_OLED_RES, OUTPUT);
  if (!SPI.swap(SPI1_SWAP_ALT1)) {
    g_display_ok = false;
    return false;
  }
  g_u8g2.begin();
  g_display_ok = true;
  return true;
}

void ob2_display(const Ob2Ui& ui) {
  if (!g_display_ok) {
    return;
  }
  g_u8g2.firstPage();
  do {
    g_u8g2.setFont(u8g2_font_5x8_tr);

    g_u8g2.setCursor(0, 8);
    g_u8g2.print(F("OB2 "));
    g_u8g2.print(ui.filter_name);
    g_u8g2.print(' ');
    g_u8g2.print(ui.filter_bands);

    g_u8g2.setCursor(0, 16);
    g_u8g2.print(F("ABC "));
    g_u8g2.print(ui.a);
    g_u8g2.print(ui.b);
    g_u8g2.print(ui.c);
    g_u8g2.print(' ');
    g_u8g2.print(ui.coils);
    g_u8g2.print(F("c "));
    g_u8g2.print(ui.load_ma);
    g_u8g2.print(F("mA"));

    g_u8g2.setCursor(0, 24);
    g_u8g2.print(F("CC "));
    g_u8g2.print(ob2_cc_name(ui.cc_raw));
    g_u8g2.print(' ');
    g_u8g2.print(ui.cc1_mv);
    g_u8g2.print('/');
    g_u8g2.print(ui.cc2_mv);

    g_u8g2.setCursor(0, 32);
    g_u8g2.print(F("deb "));
    if (ui.cc_stable_valid) {
      g_u8g2.print(ob2_cc_name(ui.cc_stable));
    } else {
      g_u8g2.print(F("..."));
    }
    g_u8g2.print(F(" I "));
    if (!ui.isense_spec) {
      g_u8g2.print(F("<100"));
    } else {
      g_u8g2.print(ui.isense_ma);
    }

    g_u8g2.setCursor(0, 40);
    if (ui.bias_live) {
      g_u8g2.print(F("TX "));
      g_u8g2.print(ui.carrier_hz);
      g_u8g2.print(F("Hz"));
    } else if (ui.boost) {
      g_u8g2.print(F("boost settle"));
    } else if (!ui.may_key) {
      g_u8g2.print(F("key locked"));
    } else if (!ui.synth_ok) {
      g_u8g2.print(F("no synth"));
    } else {
      g_u8g2.print(F("key idle"));
    }

    g_u8g2.setCursor(0, 48);
    g_u8g2.print(F("EE "));
    if (ui.eeprom_busy) {
      g_u8g2.print(F("wr"));
    } else if (ui.eeprom_ok) {
      g_u8g2.print(F("ok"));
    } else {
      g_u8g2.print(F("fail"));
    }
    g_u8g2.print(F(" OLED "));
    g_u8g2.print(ui.display_ok ? F("ok") : F("no"));
    g_u8g2.print(F(" CLK "));
    g_u8g2.print(ui.bias_live ? F("on") : F("off"));

    g_u8g2.setCursor(0, 56);
    g_u8g2.print(F("btn "));
    static const char kMarks[] = "UDLRB12";
    for (uint8_t i = 0; i < Ob2BtnCount; ++i) {
      if ((ui.buttons & (1u << i)) != 0) {
        g_u8g2.print(kMarks[i]);
      } else {
        g_u8g2.print('.');
      }
    }

    g_u8g2.setCursor(0, 64);
    g_u8g2.print(F("DSP1 on  BACK off"));
  } while (g_u8g2.nextPage());
}
