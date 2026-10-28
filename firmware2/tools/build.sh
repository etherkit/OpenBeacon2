#!/usr/bin/env bash
# Build firmware2 for the AVR128DA48 with DxCore. Used by CI and locally.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FQBN="${OB2_FQBN:-DxCore:megaavr:avrdaopti:chip=avr128da48,clock=24internal,millis=tcb2,bootloaderusart=ser0}"
INDEX="${OB2_DXCORE_INDEX:-https://raw.githubusercontent.com/SpenceKonde/ReleaseScripts/master/package_drazzy.com_index.json}"
CORE_VER="${OB2_DXCORE_VERSION:-1.6.2}"

export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="$INDEX"
export ARDUINO_DIRECTORIES_DATA="${ARDUINO_DIRECTORIES_DATA:-$ROOT/.arduino/data}"
export ARDUINO_DIRECTORIES_DOWNLOADS="${ARDUINO_DIRECTORIES_DOWNLOADS:-$ROOT/.arduino/downloads}"
export ARDUINO_DIRECTORIES_USER="${ARDUINO_DIRECTORIES_USER:-$ROOT/.arduino/user}"
mkdir -p "$ARDUINO_DIRECTORIES_DATA" "$ARDUINO_DIRECTORIES_DOWNLOADS" "$ARDUINO_DIRECTORIES_USER"

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "arduino-cli is not on PATH" >&2
  exit 1
fi

if ! arduino-cli core list | awk -v ver="$CORE_VER" 'NR > 1 && $1 == "DxCore:megaavr" && $2 == ver { found = 1 } END { exit !found }'; then
  arduino-cli core update-index
  arduino-cli core install "DxCore:megaavr@${CORE_VER}"
fi

if ! arduino-cli lib list | grep -q "Etherkit Si5351.*2\\.4\\.0"; then
  arduino-cli lib install "Etherkit Si5351@2.4.0"
fi
if ! arduino-cli lib list | grep -q "U8g2.*2\\.36\\.19"; then
  arduino-cli lib install "U8g2@2.36.19"
fi

echo "OB2_FQBN ${FQBN}"
echo "OB2_DXCORE ${CORE_VER}"
arduino-cli compile --warnings all --fqbn "$FQBN" "$ROOT/OpenBeacon2"
