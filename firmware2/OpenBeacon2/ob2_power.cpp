#include "ob2_power.h"

uint16_t ob2_adc_counts_to_mv(uint16_t counts) {
  if (counts > 4095) {
    counts = 4095;
  }
  return static_cast<uint16_t>((static_cast<uint32_t>(counts) * OB2_ADC_VREF_MV) /
                                OB2_ADC_FULL_SCALE);
}

Ob2CcLevel ob2_cc_classify_mv(uint16_t cc1_mv, uint16_t cc2_mv) {
  if (cc1_mv < OB2_CC_ABSENT_MV && cc2_mv < OB2_CC_ABSENT_MV) {
    return Ob2CcLevel::None;
  }
  const uint16_t higher = cc1_mv > cc2_mv ? cc1_mv : cc2_mv;
  if (higher >= OB2_CC_3A_MV) {
    return Ob2CcLevel::A3000;
  }
  if (higher >= OB2_CC_1A5_MV) {
    return Ob2CcLevel::A1500;
  }
  return Ob2CcLevel::Default;
}

uint16_t ob2_isense_ma(uint16_t sense_mv) {
  return static_cast<uint16_t>(sense_mv / OB2_ISENSE_MV_PER_MA);
}

bool ob2_isense_specified(uint16_t ma_value) {
  return ma_value >= OB2_ISENSE_UNSPEC_MA;
}

const char* ob2_cc_name(Ob2CcLevel level) {
  switch (level) {
    case Ob2CcLevel::None:
      return "none";
    case Ob2CcLevel::Default:
      return "default";
    case Ob2CcLevel::A1500:
      return "1.5A";
    case Ob2CcLevel::A3000:
      return "3A";
  }
  return "?";
}
