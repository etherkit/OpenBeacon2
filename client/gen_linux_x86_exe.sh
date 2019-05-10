#!/bin/bash

source linux/venv/bin/activate

pyinstaller --onefile --noconfirm \
  --distpath ./linux-arm \
  --clean \
  obmsync_linux_x86.spec

deactivate
