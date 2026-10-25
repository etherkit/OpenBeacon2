# OpenBeacon 2 rev C: open items to verify

Draft schematic, not reviewed. These items cover the AVR128DA32 MCU sheet (`pcb/mcu_usb.kicad_sch`) the relay-driver swap and the 6-relay filter-bank tree (`pcb/filter_bank.kicad_sch`).

- [ ] **U9 pin map**: TWI0 SDA/SCL on PA2/PA3; USART0 PA0/PA1 goes to the CP2102N; USART1 PC0/PC1 goes to J10 (UART_TX/UART_RX; fixed in 823d8bb, previously swapped with LPF_15_10); ADC PD0-PD2 serves A2-A4; band select BAND_A on PA5 (pin 3) and BAND_B on PA6 (pin 4), with PA7 (pin 5) and PC2 (pin 8) now spare/no-connect; TX keying on PA4; buttons on PD3-PD7/PF2/PF3; D10 on PC3; D4 on PF4; SYNC LED on PF5. The firmware pin table must follow this map.
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
- [ ] **Power**: +5V now comes only from USB-C VBUS (500 mA default). Relay load is 0 / 84 / 42 / 127 mA nominal for codes 00 / 01 / 10 / 11 (<= 147 mA worst case), compared with about 42 mA for the old one-filter-on design. The PA/boost current is not yet measured. U11 LP2985 (150 mA) carries about 45 mA on +3.3V.
  - Jason:
- [x] **U1 TLV61046A**: FB, EN and VIN are all on the +5V net (netlist checked).
  - Jason: FB tied to VIN selects internal 12 V output (TLV61046A datasheet).
