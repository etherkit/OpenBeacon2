win\venv\Scripts\activate.bat

pyinstaller --onefile --noconfirm ^
  --distpath win ^
  --clean ^
  obmsync_win.spec

deactivate
