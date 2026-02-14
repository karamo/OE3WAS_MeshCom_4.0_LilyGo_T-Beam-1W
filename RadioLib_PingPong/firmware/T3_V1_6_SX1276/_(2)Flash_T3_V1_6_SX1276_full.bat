@echo on
mode |find "COM"
set /p pn=Serial Port:

PYTHON -m esptool -p %pn% --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
pause
