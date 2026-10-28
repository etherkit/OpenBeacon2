#pragma once

/*
 * ADC math for section 4.14. The converter is 12-bit with the internal
 * 2.5 V reference. VREFA is never used (PD7 is a button).
 *
 * CC: use the higher of CC1 and CC2. Both under 0.2 V means no source,
 * which policy treats as Default. Under 0.66 V is Default, 0.66 V up to
 * but not including 1.23 V is 1.5 A, and 1.23 V and above is 3 A.
 *
 * PA current is V(TX_ISENSE) / 4.0 V/A. Readings under 100 mA are unspecified.
 * isense_k is a later calibration; this milestone uses the nominal scale.
 */

#include <stdint.h>

enum class Ob2CcLevel : uint8_t { None = 0, Default, A1500, A3000 };

constexpr uint16_t OB2_ADC_VREF_MV = 2500;
constexpr uint16_t OB2_ADC_FULL_SCALE = 4096;
constexpr uint16_t OB2_CC_ABSENT_MV = 200;
constexpr uint16_t OB2_CC_1A5_MV = 660;
constexpr uint16_t OB2_CC_3A_MV = 1230;
constexpr uint16_t OB2_CC_DEBOUNCE_MS = 200;
constexpr uint16_t OB2_ISENSE_UNSPEC_MA = 100;
constexpr uint16_t OB2_ISENSE_MV_PER_MA = 4;

uint16_t ob2_adc_counts_to_mv(uint16_t counts);
Ob2CcLevel ob2_cc_classify_mv(uint16_t cc1_mv, uint16_t cc2_mv);
uint16_t ob2_isense_ma(uint16_t sense_mv);
bool ob2_isense_specified(uint16_t ma_value);
const char* ob2_cc_name(Ob2CcLevel level);
