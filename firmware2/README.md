# firmware2

Clean-sheet firmware for the OpenBeacon 2 Rev C (AVR128DA48, DxCore). Milestone M0 is the tree, the CI build, a host test, and the U9 pin map. Feature code starts at M1.

The SAMD21 firmware under `firmware/` is the capability guide only. Nothing here is ported from it.

## Toolchain

CI builds with [arduino-cli](https://arduino.github.io/arduino-cli/) and DxCore, which is what spec section 4.1 specifies, and what section 4.11 uses for Optiboot uploads over the CP2102N. PlatformIO can drive the same core, but the board options this hardware needs are DxCore menu ids (chip, clock, millis timer, bootloader USART). arduino-cli passes those as the FQBN, so CI does not depend on a PlatformIO platform package tracking DxCore.

The FQBN is:

```
DxCore:megaavr:avrdaopti:chip=avr128da48,clock=24internal,millis=tcb2,bootloaderusart=ser0
```

| Option | Why |
|---|---|
| `avrdaopti` / AVR128DA48 | Optiboot build. Flash limit is 130560 bytes (512 bytes reserved), matching section 4.11. M0 does not burn the bootloader. |
| `clock=24internal` | 24 MHz internal OSCHF. See the autotune note below. |
| `millis=tcb2` | Spec section 4.15 keeps `millis` on TCB2 so TCB0/TCB1 stay free for the CLK4 counter. |
| `bootloaderusart=ser0` | USART0 default pins PA0/PA1, the CP2102N port. That menu entry is internal-clock only, which matches the clock option. |

Pinned versions: arduino-cli 1.5.1, DxCore 1.6.2 (the current board-manager release). The core is installed from Spence Konde's GitHub copy of `package_drazzy.com_index.json`:

`https://raw.githubusercontent.com/SpenceKonde/ReleaseScripts/master/package_drazzy.com_index.json`

`drazzy.com`'s TLS certificate expired on 20 Oct 2026, so the canonical board-manager URL fails verification. The GitHub copy is the same index and its platform and toolchain URLs are on `github.io` and S3.

### OSCHF and the 32.768 kHz crystal

`clock=24internal` makes DxCore write `CLKCTRL.OSCHFCTRLA` frequency select for 24 MHz (`0x09 << 2` in `wiring.c`). It does not set the autotune bit, and DxCore has no menu for "OSCHF auto-tuned to XOSC32K". Spec section 4.1 describes that as the intended clock. M0 does not turn autotune on in the stub: it needs the crystal running, and it belongs with the timebase. The stub is 24 MHz internal, not crystal-disciplined.

## Layout

```
firmware2/
  OpenBeacon2/           sketch (Arduino builder compiles this directory)
    OpenBeacon2.ino      M0 stub: relay drivers and SYNC LED held off
    ob2_pins.h           U9 pin map, no Arduino headers
    ob2_pin_check.cpp    static_asserts against the DxCore pin macros
  host/                  CMake + doctest, Linux
  third_party/doctest/   doctest 2.4.11 (its own MIT license, not this project's)
  tools/build.sh         install DxCore if needed, then compile
```

Later logic modules stay free of Arduino headers so the host tests can build them. M0 has no logic modules yet. The sample test locks the pin table.

## Pin map

`ob2_pins.h` follows spec section 2.1, with section 2.2 applied:

* PE1 `TX_ISENSE`, PE2 `CC1_SENSE`, PE3 `CC2_SENSE`
* PB1 `BAND_C` (USART3 RXEN stays off)
* GPS RX on USART2 PF5, ALT1, RX only (PF4 stays GPIO)
* MCU TX to the GPS on USART3 PB0
* CLK4 cal input on PB2

The power-rework schematic is not in this repository. Commit `88e9a62` on `pcb-layout` still has PB1 unconnected and PE1-PE3 on J5. The header says so, in `OB2_POWER_REWORK_NOTE` and on those four rows.

## Build

```bash
# arduino-cli 1.5.1 on PATH
firmware2/tools/build.sh

cmake -S firmware2/host -B firmware2/host/build -DCMAKE_CXX_COMPILER=g++
cmake --build firmware2/host/build
ctest --test-dir firmware2/host/build --output-on-failure
```

CI is `.github/workflows/firmware2.yml`. It compiles the sketch and runs the host test.
