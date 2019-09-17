#!/bin/bash

source ./linux-arm/venv/bin/activate

pyinstaller --onefile --noconfirm --distpath ./linux-arm --clean ob2sync_linux_arm.spec

deactivate
