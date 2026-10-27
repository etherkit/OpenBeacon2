/*
 * OpenBeacon 2 firmware2, milestone M0.
 *
 * Builds for the AVR128DA48 on DxCore. Holds the relay drivers and the SYNC
 * LED off so a later bring-up starts from the reset state in spec section 2.2
 * (BAND_C low; R91 pull-down on the rework, which is not in this repo yet).
 * No radio, GPS, display, or protocol code.
 *
 * Clock: the FQBN selects 24 MHz internal OSCHF. DxCore writes FREQSEL only.
 * It does not set CLKCTRL.OSCHFCTRLA.AUTOTUNE, so this stub is not disciplined
 * to the 32.768 kHz crystal. That belongs with the timebase, not M0.
 */

#include "ob2_pins.h"

void setup() {
  pinMode(OB2_BAND_A, OUTPUT);
  pinMode(OB2_BAND_B, OUTPUT);
  pinMode(OB2_BAND_C, OUTPUT);
  digitalWrite(OB2_BAND_A, LOW);
  digitalWrite(OB2_BAND_B, LOW);
  digitalWrite(OB2_BAND_C, LOW);

  pinMode(OB2_SYNC_LED, OUTPUT);
  digitalWrite(OB2_SYNC_LED, LOW);
}

void loop() {
}
