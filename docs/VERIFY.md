# OpenBeacon 2 rev C: open items to verify

Draft schematic, not reviewed. These items cover the AVR128DA32 MCU sheet (`pcb/mcu_usb.kicad_sch`) and the relay-driver swap.

- [ ] **U9 pin map**: TWI0 SDA/SCL on PA2/PA3; USART0 PA0/PA1 goes to the CP2102N; USART1 PC0/PC1 goes to J10 (UART_TX/UART_RX); ADC PD0-PD2 serves A2-A4; LPF selects on PA5-PA7/PC2; TX keying on PA4; buttons on PD3-PD7/PF2/PF3; D10 on PC3; D4 on PF4; SYNC LED on PF5. The firmware pin table must follow this map.
  - Jason:
- [ ] **TX_LED**: the Empyrean D13 LED has no pin left. Only SYNC (PF5) is fitted. Should J4 D4 or D10 give up a pin?
  - Jason:
- [ ] **Y1/C41/C42**: Epson Q13FC13500004 (CL 12.5 pF). The calculation 2*(12.5-2)-5 = 16 pF gives 15 pF C0G. Use XOSC32K LPMODE=0 and trim after measuring the frequency error.
  - Jason:
- [ ] **D10/R35**: the SerialUPDI diode and 470R are kept as on Si5351AVFO J5 (not a power diode). Confirm this matches the "no diodes" intent.
  - Jason:
- [ ] **RESET (PF6)**: DTR auto-reset uses C37/R36/R37. Do not enable the HV UPDI override on RESET while C37 is fitted.
  - Jason:
- [ ] **Q6-Q9**: changed from 2N7002 to AO3400A (C20917). The 2N7002 has Vth up to 2.5 V and no Rds(on) spec below 4.5 V, while each FET carries 2 x G6K coils (~45 mA).
  - Jason:
- [ ] **Power**: +5V now comes only from USB-C VBUS (500 mA default). The PA/boost current is not yet measured. U11 LP2985 (150 mA) carries about 45 mA on +3.3V.
  - Jason:
- [ ] **U1 TLV61046A**: FB is tied to +5V (existing design, not changed). Check this.
  - Jason:
