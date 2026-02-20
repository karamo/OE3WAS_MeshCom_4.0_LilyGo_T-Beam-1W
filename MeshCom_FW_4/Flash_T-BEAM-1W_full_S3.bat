@echo on
mode |find "COM"
set /p pn=Serial Port:

::PYTHON -m esptool -p %pn% --baud 921600 erase_region 0x9000 0x5000
PYTHON -m esptool -p %pn% --baud 921600 -c esp32s3 --before usb-reset -a watchdog-reset write_flash 0x0 bootloader-s3.bin 0x8000 partitions.bin 0xE000 otadata.bin 0x10000 safeboot-s3.bin 0xF0000 firmware.bin
pause
