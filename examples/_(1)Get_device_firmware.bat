::@echo off
set builds=.pio\build
dir /B /A:D %builds% >builds.txt
type builds.txt

for /F %%i in (builds.txt) do md firmware\%%i

for /F %%i in (builds.txt) do copy /Y %builds%\%%i\firmware.bin firmware\%%i\firmware.bin
for /F %%i in (builds.txt) do copy /Y %builds%\%%i\bootloader.bin firmware\%%i\bootloader.bin
for /F %%i in (builds.txt) do copy /Y %builds%\%%i\partitions.bin firmware\%%i\partitions.bin
for /F %%i in (builds.txt) do copy /Y %builds%\%%i\firmware.elf firmware\%%i\firmware.elf

:: bootloader.bin auf "esp32s3.c" testen und dann umbenennen
:: anfangen mit:
findstr /M /C:"bootloader_esp32s3.c" *.bin > s3.txt
:: dann s3.txt auswerten, ob ein Dateinamen enthalten ist, der dann umbenannt werden muss
type s3.txt

pause
