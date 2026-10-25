# OpenBeacon 2 rev C: open items to verify

Draft schematic, not reviewed. These items cover the AVR128DA48 MCU sheet (`pcb/mcu_usb.kicad_sch`) the relay-driver swap and the 6-relay filter-bank tree (`pcb/filter_bank.kicad_sch`).

- [ ] **U9 pin map (AVR128DA48-E/PT TQFP-48, LCSC C5227775; alternate AVR128DA48-I/PT C1012825)**: TWI0 SDA/SCL on PA2/PA3 (pins 46/47); USART0 PA0/PA1 (44/45) goes to the CP2102N; USART1 PC0/PC1 (10/11) goes to J10; ADC PD0-PD2 (20-22) serves A2-A4; BAND_A PA5 (1), BAND_B PA6 (2); TX keying PA4 (48); buttons PD3-PD7 (23-27), BTN_DSP_2 PF3 (37), BTN_DSP_1 moved to PE0 (30); D4 PA7 (3); SYNC LED PC2 (12); D10 PC3 (13); crystal PF0/PF1 (34/35); RESET PF6 (40); UPDI (41). GPS on USART3 DEFAULT: PB0 (4) TX to GPS_RXD, PB1 (5) RX from GPS_TXD (DS40002183E PORTMUX USARTROUTEA bits 7:6, 0x0 = PB0/PB1; USART2 is no longer used because its DEFAULT is the crystal pins). Pinout checked against DS 3.1, 48-pin column. Decoupling: C49 100n on VDD 14, C38 100n + C39 1u on VDD 42, C40 100n on AVDD 28 (DS 4.2, Fig 4-1). The firmware pin table must follow this map.
  - Jason:
- [x] **TX_LED**: resolved. D12 (KT-0603YG yellow-green, Vf 1.8-2.4 V) and R39 (470R) run from the TX net (PA4) to GND, so no extra MCU pin is used. Current is about (3.1 - 1.9)/470 = 2.6 mA, plus about 0.15 mA into R1 22k, all well under the 6 mA VOH spec (VOH >= VDD-0.7). Q1's gate stays above 3.0 V.
  - Jason: TX LED is essential; drive from TX keying net.
- [ ] **Y1/C41/C42**: Epson Q13FC13500004 (CL 12.5 pF). The calculation 2*(12.5-2)-5 = 16 pF gives 15 pF C0G. Use XOSC32K LPMODE=0 and trim after measuring the frequency error.
  - Jason:
- [x] **D10/R35**: resolved. The UPDI circuit is identical to Si5351AVFO J5 and EtherKeyerMini J3: header pin 2 goes to the 1N5711 cathode; pin 3 goes to the 1N5711 anode and then through 470R to UPDI; pin 1 is the VCC reference; pin 4 is GND. EtherKeyerMini's pin-1 power diode (D2) and its UART solder jumpers (JP2/JP3) are left out, as on Si5351AVFO.
  - Jason: Keep identical to Si5351AVFO and EtherKeyerMini.
- [x] **D11 SYNC LED colour**: resolved. D11 is now yellow KT-0603Y (C2287, Vf 1.9-2.3 V @ 20 mA) with R38 470R (C23179), giving about (3.2 - 1.9)/470 = 2.8 mA, or about 2 mA at the highest Vf.
  - Jason: Yellow.
- [ ] **RESET (PF6)**: DTR auto-reset uses C37/R36/R37. Do not enable the HV UPDI override on RESET while C37 is fitted.
  - Jason:
- [ ] **Q6/Q7 relay drivers**: AO3400A (C20917), 5.7 A continuous, 48 mR at Vgs 2.5 V. Q6 (BAND_A) drives K1/K4: 2 x G6K coils (5 V / 237 R = 21.1 mA each), 42 mA nominal, <= 49 mA at 5.25 V with -10 % coil R. Q7 (BAND_B) drives K2/K3/K5/K6: 84 mA nominal, <= 98 mA. Drop < 5 mV. Q8/Q9, R13/R16, K7/K8 and D7/D8 are removed. Flyback is one 1N4148W (C81598) per coil, at each relay (D1-D6).
  - Jason:
- [ ] **Filter-bank relay tree**: 6 x G6K-2F-Y-DC5 (C326376, one pole used, pole 2 NC), two levels on the input side (K1 = K_A1, K2 = K_B1, K3 = K_B2) mirrored on the output side (K4 = K_A2, K5 = K_B3, K6 = K_B4). Input and output never share a relay. Unselected filters float (open at both ends) instead of being grounded. Code (BAND_A, BAND_B): 00 = 80/60, 01 = 40/30, 10 = 20/17, 11 = 15/12/10. Default with all relays off (NC): LPF_IN -> K1 NC -> P1_IN -> K2 NC -> F80_60 -> K5 NC -> P1_OUT -> K4 NC -> LPF_OUT (netlist checked). The signal now passes 4 contacts instead of 2; Omron's G6K-2F-Y curves show about 0.1-0.2 dB insertion loss per contact at 30 MHz. Layout: keep K1-K2-K3 and K4-K5-K6 traces short, as 50 ohm CPWG over solid GND.
  - Jason:
- [ ] **Power**: +5V now comes only from USB-C VBUS (500 mA default). Relay load is 0 / 84 / 42 / 127 mA nominal for codes 00 / 01 / 10 / 11 (<= 147 mA worst case), compared with about 42 mA for the old one-filter-on design. The PA/boost current is not yet measured. U11 is now TLV75533PDBVR (500 mA, C404027); see the +3.3 V budget item under GPS.
  - Jason:
- [x] **U1 TLV61046A**: FB, EN and VIN are all on the +5V net (netlist checked).
  - Jason: FB tied to VIN selects internal 12 V output (TLV61046A datasheet).

## GPS option (pcb/gps.kicad_sch, U12 u-blox MAX-M10S-00B)

- [x] **GPS_PPS MCU pin: PF2 (pin 36).** PF2 has full asynchronous sensing (DS 3.1 note: Px2/Px6). EVSYS: PORTF pin generators are 0x48-0x4F on CHANNEL4/CHANNEL5, so CHANNEL4 = 0x4A (PORTF_PIN2) and user USERTCB0CAPT (user 0x1F, async/sync both) gives TCB0 input-capture on PPS. PF2 resets as an input without pull-up/down, which keeps SAFEBOOT_N high (see below). J14 stays as a probe point.
- [x] **Backup source (was C47 supercap).** Resolved: no SMT supercap or rechargeable ML/MS cell is in stock at JLCPCB (searched XH414H, CPH3225A, ML614/621, MS621FE/920SE, DSK, DMF; all 0 stock or absent). C47, D13 and R41 are removed. BT1 is a CR1220 primary cell in a Kinghelm KH-BS1220-2-SMT holder (C5365932, 2394 in stock, $0.24 @1, $0.19 @50), straight onto V_BCKP with C46 1u. CR1220: 3 V, 35 mAh (Panasonic CR1220 datasheet), inside V_BCKP 1.65-3.6 V (DS Table 13). No charger, so a primary cell is safe. Hold time unpowered: 35 mAh / 28 uA (DS Table 17) = about 1250 h (about 52 days); while powered the backup draw is about 3 uA (DS Table 17 fn 29).
  - Jason:
- [ ] **BT1 footprint and cell.** KiCad has no KH-BS1220-2-SMT footprint (field left blank); draw it from the Kinghelm drawing before layout. The cell is user-fitted (JLC places the holder only). Do NOT fit an ML1220 rechargeable: there is no charge circuit.
  - Jason:
- [x] **Antenna short-circuit current.** Resolved: U13 TPS22941DCKR (C2653762, $1.12 @1, $1.00 @10) sits between VCC_RF and R40 as the current limiter of the IM 3.3.1.2 two-pin antenna supervisor (replacing T1/R8). TPS2294x DS 7.5/7.6: I_LIM 40/65/80 mA (min/typ/max) at 3.3 V; blanking 5/10/20 ms; auto-restart 40/80/160 ms. A dead short now draws at most 80 mA for at most 20 ms per cycle: average <= 80 x 20/(40+20) = 27 mA worst case, about 7 mA typical (was 320 mA DC). r_ON 0.5 ohm at 3.3 V, so a 25 mA antenna sees VCC_RF (VCC - 0.1 V, DS Table 13) minus ~0.27 V = about 2.9 V. U13 ~OC drives SDA, reassigned as ANT_SHORT_N (firmware: CFG-HW-ANT_CFG_SHORTDET=1, CFG-HW-ANT_SUP_SHORT_PIN = SDA's PIO number; optional PWRDOWN/RECOVER). ~ON tied to GND (always on).
  - Jason:
- [ ] **Antenna limiter residuals.** The 80 mA max peak exceeds the VCC_RF 50 mA operating limit (DS Table 13) for <= 20 ms bursts, though far below the 250 mA abs max (DS Table 12); confirm on the bench. TPS22941 stock at JLC is thin (26 pcs); source from DigiKey/Mouser for runs. ANT_SHORT_N relies on the module's PIO pull-up (IM 3.3.1.4); confirm SDA's PIO number in the interface description.
  - Jason:
- [x] **+3.3 V regulator / budget.** Resolved: U11 is TLV75533PDBVR (C404027, 108k in stock, $0.12 @1, extended part; no basic-library 300-500 mA 3.3 V LDO found). 500 mA, dropout <= 238 mV @ 500 mA, IQ 25 uA, PSRR 46 dB @ 100 kHz, stable with >= 1 uF ceramic, COUT <= 200 uF (TLV755P DS). Pin-compatible with the LP2985 SOT-23-5 except pin 4 (NC, so C35 is removed); C34 1u in, C36 4.7u out. Budget: existing 45 mA + GPS 14 mA + antenna up to 25 mA = 84 mA steady (17 %, (5-3.3) x 0.084 = 0.14 W, about +9 C at 60 C/W). Peak with 100 mA GPS start-up inrush (DS after Table 16): 170 mA (34 %).
  - Jason:
- [ ] **V_IO ramp rate.** It must be 25-35000 us/V (DS 4.1 Table 12); ramping faster can damage the part. Measure the +3.3 V rise at power-up with the TLV75533 and its output caps.
- [ ] **Decoupling values.** u-blox gives no explicit VCC/V_IO capacitor values. C43 100n + C44 4.7u at pins 7/8 and C46 1u at V_BCKP are my choice. Place them beside the pins with short GND vias.
- [ ] **TIMEPULSE = SAFEBOOT_N.** These are joined through 1k inside the module (DS Table 10 fn 15), and the module enters safeboot if the pin is low at start-up. Whatever connects to J14 / GPS_PPS must not pull low at power-up. The firmware pin must be a high-Z input with no pull-down.
- [ ] **RF layout.** Use a 50 ohm line from U12 pin 11 to J15 U.FL, with L3's pad on the line. RF_IN is DC-blocked internally (IM Table 1), so there is no external DC block.
