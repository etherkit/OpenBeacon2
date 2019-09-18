rem To generate virtual environment:

rem python -m venv win\venv
rem win\venv\Scripts\activate.bat
rem pip install pyinstaller
rem pip install pyserial
rem pip install cmd2

rem win\venv\Scripts\activate.bat

pyinstaller --onefile --noconfirm ^
  --distpath win ^
  --clean ^
  ob2sync_win.spec

deactivate
