#pragma once

/*
 * K3, checked against Microchip DS40002183E (AVR128DA28/32/48/64).
 *
 *   16.3.7 TCAROUTEA (p.161): TCA0 PORTA is PA0-PA5, so PA5 is WO5.
 *          TCA0 PORTB is PB0-PB5, so PB1 is WO1. One PORTMUX value.
 *          TCA1 PORTB is PB0-PB5. TCA1 has no PORTA route.
 *   16.3.8 TCBROUTEA (p.162): TCB0 PA2/PF4, TCB1 PA3/PF5, TCB2 PC0/PB4,
 *          TCB3 PB5/PC1, TCB4 PG3/PC6. No TCB waveform on PA5, PA6, or PB1.
 *   16.3.9 TCDROUTEA (p.163): default WOA PA4, WOB PA5, WOC PA6, WOD PA7.
 *   23.5.3 TCD.CTRLC (p.321): CMPCSEL reset 0 connects WOC to PWMA (WOA).
 *          A bench PWM build must set CMPCSEL to PWMB or PA6 follows PA4.
 *          WOC then has the same duty as WOB.
 *   TCD.FAULTCTRL CMPAEN must stay off. WOA is PA4, the TX key.
 *   I/O mux table: PA6 has TCD WOC and no TCA waveform output.
 *
 * TCA1 WO0 is PB0 (GPS USART3 TX). Leave that compare output disabled.
 * Shipped firmware does not PWM the relays. Do not analogWrite PA4-PA7:
 * DxCore can route that range to TCD0, which would drive the TX key.
 */

#include <stdint.h>

constexpr bool OB2_RELAY_PWM_SHIPPED = false;
constexpr bool OB2_TCA0_SERVES_PA5_AND_PB1 = false;
constexpr bool OB2_TCB_DRIVES_RELAY_PINS = false;

struct Ob2TimerPin {
  char port;
  int8_t bit;
  const char* timer;
  const char* signal;
  const char* portmux;
  bool bench_candidate;
  const char* note;
};

static constexpr Ob2TimerPin OB2_TIMER_PINS[] = {
    {'A', 5, "TCA0", "WO5", "PORTA", true, "TCA0 on PORTA. Not available if TCA0 moves to PORTB."},
    {'A', 5, "TCD0", "WOB", "DEFAULT", true, "Proposed bench output with PA6."},
    {'A', 6, "TCD0", "WOC", "DEFAULT", true,
     "Only timer output on PA6. Reset CMPCSEL follows WOA (PA4); set CMPCSEL to PWMB."},
    {'B', 1, "TCA1", "WO1", "PORTB", true, "Proposed bench output. Keep TCA1 WO0 (PB0) disabled."},
    {'B', 1, "TCA0", "WO1", "PORTB", true, "Uses the only TCA0 route, so PA5 WO5 is gone."},
    {'A', 4, "TCD0", "WOA", "DEFAULT", false, "TX key and TX LED. Enabling WOA keys the PA supply."},
    {'B', 0, "TCA1", "WO0", "PORTB", false, "USART3 TX to the GPS."},
    {'B', 0, "TCA0", "WO0", "PORTB", false, "Same pin when TCA0 is routed to PORTB."},
};

constexpr uint8_t OB2_TIMER_PIN_COUNT =
    sizeof(OB2_TIMER_PINS) / sizeof(OB2_TIMER_PINS[0]);
