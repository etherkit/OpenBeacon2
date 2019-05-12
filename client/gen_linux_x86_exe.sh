#!/bin/bash

source linux-x86/venv/bin/activate

pyinstaller --onefile --noconfirm \
  --distpath ./linux-x86 \
  --clean \
  obmsync_linux_x86.spec

deactivate
