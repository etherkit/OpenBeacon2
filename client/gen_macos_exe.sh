#!/bin/bash

# To generate virtual environment:

# sudo apt install python3-venv (for Debian-based systems, if not yet installed)
# python3 -m venv macos/venv
# source macos/venv/bin/activate
# pip install pyinstaller
# pip install pyserial
# pip install cmd2

source macos/venv/bin/activate

pyinstaller --onefile --noconfirm --distpath ./macos --clean ob2sync_macos.spec

deactivate
