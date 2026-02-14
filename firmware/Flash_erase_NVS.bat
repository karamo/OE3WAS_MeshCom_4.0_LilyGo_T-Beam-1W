@echo on
mode |find "COM"
set /p pn=Serial Port:

PYTHON -m esptool -p %pn% --baud 921600 erase_region 0x9000 0x5000
pause
