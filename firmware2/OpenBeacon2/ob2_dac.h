#pragma once

/*
 * MCP4725A1 at 0x62, 5 V full scale. Code is (mV * 4096) / 5000,
 * clamped to the 12-bit range. 5000 mV would be code 4096, which does
 * not fit, so that input becomes 4095.
 *
 * OB2_BIAS_BRINGUP_MV is a conservative bench placeholder, not a
 * calibrated power level.
 */

#include <stdint.h>

constexpr uint8_t OB2_MCP4725_ADDR = 0x62;
constexpr uint16_t OB2_MCP4725_VREF_MV = 5000;
constexpr uint16_t OB2_MCP4725_FULL_SCALE = 4096;
constexpr uint16_t OB2_MCP4725_MAX_CODE = 4095;
constexpr uint16_t OB2_BIAS_BRINGUP_MV = 500;

uint16_t ob2_mcp4725_code(uint16_t mv);
