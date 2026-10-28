#include "ob2_dac.h"

uint16_t ob2_mcp4725_code(uint16_t mv) {
  if (mv >= OB2_MCP4725_VREF_MV) {
    return OB2_MCP4725_MAX_CODE;
  }
  return static_cast<uint16_t>((static_cast<uint32_t>(mv) * OB2_MCP4725_FULL_SCALE) /
                                OB2_MCP4725_VREF_MV);
}
