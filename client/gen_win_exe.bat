rem To generate virtual environment:

rem sudo apt install python3-venv (for Debian-based systems, if not yet installed)
rem python3 -m venv linux-x86/venv
rem pip install pyinstaller
rem pip install pyserial
rem pip install cmd2

win\venv\Scripts\activate.bat

pyinstaller --onefile --noconfirm ^
  --distpath win ^
  --clean ^
  ob2sync_win.spec

deactivate
