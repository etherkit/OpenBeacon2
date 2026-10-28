#pragma once

/*
 * TWI0 on PA2/PA3. Si5351C is 0x60. MCP4725A1 is 0x62 on the 5 V side
 * of the same bus. U8 is 24AA64T-I/OT in SOT-23-5: the symbol has SCL,
 * VSS, SDA, VCC, and WP, and no A0/A1/A2, so the address is 0x50.
 * The four magic bytes are a bring-up presence check, not the config schema.
 */

#include <stdint.h>

constexpr uint8_t OB2_SI5351_ADDR = 0x60;
constexpr uint8_t OB2_EEPROM_ADDR = 0x50;
constexpr uint8_t OB2_EEPROM_MAGIC[] = {'O', 'B', '2', '1'};
constexpr uint8_t OB2_EEPROM_MAGIC_LEN = 4;
constexpr uint32_t OB2_TWI_HZ = 100000UL;
