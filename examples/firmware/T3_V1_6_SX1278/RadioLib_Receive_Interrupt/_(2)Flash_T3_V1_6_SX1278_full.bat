@echo on
mode |find "COM"
set /p pn=Serial Port:

cd firmware\T3_V1_6_SX1278
PYTHON -m esptool -p %pn% --baud 921600 write_flash 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
pause
