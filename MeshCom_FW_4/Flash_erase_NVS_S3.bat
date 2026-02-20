@echo on
mode |find "COM"
set /p pn=Serial Port:

PYTHON -m esptool -p %pn% --baud 921600 -c esp32s3 --before usb-reset -a watchdog-reset erase_region 0x9000 0x5000
pause
