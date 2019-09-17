#!/bin/bash

# To generate virtual environment:

# sudo apt install python3-venv (for Debian-based systems, if not yet installed)
# python3 -m venv linux-arm/venv
# source ./linux-arm/venv/bin/activate
# pip install pyinstaller
# pip install pyserial
# pip install cmd2

source ./linux-arm/venv/bin/activate

pyinstaller --onefile --noconfirm --distpath ./linux-arm --clean ob2sync_linux_arm.spec

deactivate
