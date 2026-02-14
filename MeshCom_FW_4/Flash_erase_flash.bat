@echo on
mode |find "COM"
set /p pn=Serial Port:

PYTHON -m esptool -p %pn% --baud 921600 erase_flash
pause
