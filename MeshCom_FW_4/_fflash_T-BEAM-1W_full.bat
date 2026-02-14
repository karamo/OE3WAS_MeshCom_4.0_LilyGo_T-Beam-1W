@echo on
mode |find "COM"
set /p pn=Serial Port:

::PYTHON -m esptool -p %pn% --baud 921600 erase_region 0x9000 0x5000
PYTHON -m esptool -p %pn% --baud 921600 write_flash 0x0 bootloader-s3.bin 0x8000 partitions.bin 0xE000 otadata.bin 0x10000 safeboot-s3.bin 0xC0000 .pio\build\LilyGo_T-Beam-1W\firmware.bin
pause
