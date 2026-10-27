# firmware2

Clean-sheet firmware for the OpenBeacon 2 Rev C (AVR128DA48, DxCore). Milestone M1 is the hardware bring-up: Optiboot notes, OLED, buttons, the SYNC LED, 3-bit relays, the current and CC ADCs, soft-start waits, a fixed Si5351 CLK0 carrier, the MCP4725, and a 24AA64 presence check.

The SAMD21 firmware under `firmware/` is the capability guide only. Nothing here is ported from it.

## Toolchain

CI builds with [arduino-cli](https://arduino.github.io/arduino-cli/) and DxCore, which is what spec section 4.1 specifies, and what section 4.11 uses for Optiboot uploads over the CP2102N. PlatformIO can drive the same core, but the board options this hardware needs are DxCore menu ids (chip, clock, millis timer, bootloader USART). arduino-cli passes those as the FQBN.

The FQBN is:

```
DxCore:megaavr:avrdaopti:chip=avr128da48,clock=24internal,millis=tcb2,bootloaderusart=ser0
```

| Option | Why |
|---|---|
| `avrdaopti` / AVR128DA48 | Optiboot build. Flash limit is 130560 bytes (512 bytes reserved). |
| `clock=24internal` | 24 MHz internal OSCHF. See the autotune note below. |
| `millis=tcb2` | Section 4.15 keeps `millis` on TCB2 so TCB0 and TCB1 stay free. |
| `bootloaderusart=ser0` | USART0 on PA0/PA1, the CP2102N port. |

Pinned versions: arduino-cli 1.5.1, DxCore 1.6.2, Etherkit Si5351 2.4.0, U8g2 2.36.19. The core index is Spence Konde's GitHub copy of `package_drazzy.com_index.json`:

`https://raw.githubusercontent.com/SpenceKonde/ReleaseScripts/master/package_drazzy.com_index.json`

`drazzy.com`'s TLS certificate expired on 20 Oct 2026, so the canonical board-manager URL fails verification.

### OSCHF and the 32.768 kHz crystal

`clock=24internal` makes DxCore write `CLKCTRL.OSCHFCTRLA` frequency select for 24 MHz. It does not set the autotune bit, and DxCore has no menu for "OSCHF auto-tuned to XOSC32K". Section 4.1 describes that as the intended clock. M1 still does not turn autotune on. The sketch runs at 24 MHz internal, not crystal-disciplined.

## Layout

```
firmware2/
  OpenBeacon2/           sketch (the Arduino builder compiles this directory)
    OpenBeacon2.ino      bring-up loop
    ob2_pins.h           U9 pin map
    ob2_relays.*         section 2.2 codes (host-tested)
    ob2_power.*          ADC millivolts, CC class, isense
    ob2_softstart.*      50 ms boot gate and 20 ms boost settle
    ob2_dac.*            MCP4725 code
    ob2_pwm.h            DS40002183E timer check; PWM is not enabled
    ob2_hal.*            GPIO, ADC, I2C devices
    ob2_display.cpp      SH1106 on SPI1 ALT1
  host/                  CMake + doctest
  third_party/doctest/   doctest 2.4.11 (its own MIT license)
  tools/build.sh         install the core and libraries, then compile
```

Logic modules that the host tests build do not include Arduino headers.

## What the sketch does

Relays start at F2 (all coils off) and are plain GPIO. UP and DOWN cycle F2, F3, F4, F5. A filter change while the carrier is on turns the carrier off first. DSP1 (Menu1) is the only way to key: PA4 goes high after 50 ms from reset and a CC sample, then the sketch waits 20 ms before writing 500 mV to the MCP4725 and enabling CLK0. BACK or DSP2 turns bias off, disables CLK0, then drops PA4. The SYNC LED blinks and stays on while any button is held. The OLED is a SH1106 page buffer on SPI1 ALT1. Nothing calls `delay()`. U8g2's own reset pulse does.

CLK0 frequencies, one per filter, are the WSPR dials 3568600, 7038600, 14095600, and 21094600 Hz. CLK2 and CLK4 stay off. Drive is 8 mA because the old firmware used that; section 4.4 does not restate it. The 500 mV bias is a bench placeholder, not a calibration.

Power-rework hardware (PB1 `BAND_C`, PE1 `TX_ISENSE`, PE2 `CC1_SENSE`, PE3 `CC2_SENSE`) is still not in this repository. `88e9a62` on `pcb-layout` has PB1 unconnected and PE1-PE3 on J5.

## Bench 1 — Optiboot

Programmer: DxCore `serialupdi` (230400 baud) on the UPDI dongle attached to J13. Upload after that is the CP2102N, 115200, DTR through C37/R36.

DxCore 1.6.2 resolves this FQBN to entry condition `_extr` and this fuse image:

| Fuse | Value | Meaning |
|---|---|---|
| 0 WDTCFG | `0x00` | Watchdog off |
| 1 BODCFG | `0x00` | BOD off (menu default) |
| 2 OSCCFG | `0x00` | Internal HF oscillator |
| 5 SYSCFG0 | `0xC0` | What `burn-bootloader` writes. See below. |
| 6 SYSCFG1 | `0x04` | 8 ms startup |
| 7 CODESIZE | `0x00` | |
| 8 BOOTSIZE | `0x01` | 512-byte boot section |

Two DxCore 1.6.2 holes, from `arduino-cli compile --show-properties` on this FQBN:

1. `bootloader.class` is unset. The burn pattern looks for `{bootloader.class}_ser0_extr.hex`, which is not a file. The image that matches the 128 kB DA part, USART0 default pins, and `_extr` is `optiboot_128dx_ser0_extr.hex` in the DxCore `bootloaders/hex` directory.
2. Both Reset-pin menu choices set `resetpinbit=0`, so SYSCFG0 is `0b11000000` (`0xC0`). DS40002183E section 7.9.2.4: RSTPINCFG `00` makes PF6 a GPIO, and `0x2` enables external reset. DTR will not enter the bootloader until fuse 5 is `0xC8` (`0b11001000`: same CRC fields, RSTPINCFG = RESET, EESAVE still clear). The non-Optiboot DA menu does set the bit. This FQBN cannot.

EESAVE is clear, and the burn does a chip erase, so internal EEPROM is wiped. The 24AA64 is not part of that erase.

From the DxCore package directory, with `PROG` the SerialUPDI port and `APP` the CP2102N port:

```bash
FQBN='DxCore:megaavr:avrdaopti:chip=avr128da48,clock=24internal,millis=tcb2,bootloaderusart=ser0'
HEX="$HOME/.arduino15/packages/DxCore/hardware/megaavr/1.6.2/bootloaders/hex/optiboot_128dx_ser0_extr.hex"
PY="$HOME/.arduino15/packages/DxCore/hardware/megaavr/1.6.2/tools/prog.py"

# Fuse 5 is 0xC8 so PF6 is reset. 0xC0 is what burn-bootloader would have written.
python3 "$PY" -t uart -u "$PROG" -b 230400 -d avr128da48 \
  --fuses 0:0x00 1:0x00 2:0x00 5:0xC8 6:0x04 7:0x00 8:0x01 \
  -f "$HEX" -a write

arduino-cli upload -b "$FQBN" -p "$APP" firmware2/OpenBeacon2
```

Use a dummy load. DSP1 transmits. Recovery of a wiped bootloader is the same `prog.py` write over J13.

## Bench 2 — panel and relays

1. OLED shows `OB2 F2 80/60m`, `ABC 000`, `EE ok` after the 24AA64 answers, and `key locked` for the first 50 ms (it will already say `key idle` if the display init took longer than that).
2. Hold each button. Its letter in `btn UDLRB12` lights, and the SYNC LED stays on for the hold. Released, the LED blinks.
3. UP/DOWN steps F2 → F3 → F4 → F5 and back. Scope the three gates: `000`, `010`, `100`, `101` only. Coil current on the +5 V relay rail should step 0 / 42 / 42 / 84 mA, and never more than four coils. PA4 stays low during this.
4. DSP1 on F2: PA4 rises, and about 20 ms later CLK0 is 3.5686 MHz and the bias pin leaves 0 V (500 mV command into the MCP4725). BACK or DSP2 returns bias to 0, kills CLK0, then drops PA4. Repeat on F4 and confirm 14.0956 MHz, and that the relays changed only while the carrier was off.
5. CC and TX_ISENSE volts are on the OLED (`CC` line, `I` line). Under 100 mA the current line stays `<100`. No overcurrent trip in this milestone.

## Build

```bash
# arduino-cli 1.5.1 on PATH
firmware2/tools/build.sh

cmake -S firmware2/host -B firmware2/host/build -DCMAKE_CXX_COMPILER=g++
cmake --build firmware2/host/build
ctest --test-dir firmware2/host/build --output-on-failure
```

CI is `.github/workflows/firmware2.yml`. The host job uses the image's `cmake` and `g++` (no apt install).
