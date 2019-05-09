#!/bin/bash

pyinstaller --onedir --noconfirm \
  --distpath ./linux \
  --clean \
  obmsync_linux.spec
