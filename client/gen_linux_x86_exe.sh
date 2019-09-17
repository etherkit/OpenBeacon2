#!/bin/bash

# To generate virtual environment:

# sudo apt install python3-venv (for Debian-based systems, if not yet installed)
# python3 -m venv linux-x86/venv
# source linux-x86/venv/bin/activate
# pip install pyinstaller
# pip install pyserial
# pip install cmd2

source linux-x86/venv/bin/activate

pyinstaller --onefile --noconfirm --distpath ./linux-x86 --clean ob2sync_linux_x86.spec

deactivate
