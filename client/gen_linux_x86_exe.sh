#!/bin/bash

source linux/venv/bin/activate

pyinstaller --onedir --noconfirm \
  --distpath ./linux \
  --clean \
  obmsync_linux.spec

deactivate
