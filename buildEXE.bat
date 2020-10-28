@ECHO OFF
CD Python
call win_env\Scripts\activate.bat
pyinstaller main.spec
PAUSE