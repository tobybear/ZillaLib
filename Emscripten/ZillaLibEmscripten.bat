@echo off
set PATH=%PATH%;%_ACP_PATH%
python "%~dp0ZillaLibEmscripten.py" run %*%
