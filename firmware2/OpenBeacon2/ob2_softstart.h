#pragma once

/*
 * Section 4.14 soft-start. No keying until 50 ms after power-up and the
 * first CC sample. After PA4 turns the boost on, wait at least 20 ms
 * before the bias ramp. Times are milliseconds from the same clock the
 * caller uses (millis() on the AVR).
 */

#include <stdint.h>

constexpr uint32_t OB2_BOOT_KEY_DELAY_MS = 50;
constexpr uint32_t OB2_PA_SETTLE_MS = 20;

bool ob2_may_enable_boost(uint32_t now_ms, bool cc_sampled);
bool ob2_may_ramp_bias(bool boost_on, uint32_t now_ms, uint32_t boost_on_ms);
