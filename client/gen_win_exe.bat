win\venv\Scripts\activate.bat

pyinstaller --onefile --noconfirm ^
  --distpath win ^
  --clean ^
  ob2sync_win.spec

deactivate
