#!/bin/bash

source ./linux-arm/venv/bin/activate

pyinstaller --onefile --noconfirm --distpath ./linux-arm --clean obmsync_linux_arm.spec

deactivate
