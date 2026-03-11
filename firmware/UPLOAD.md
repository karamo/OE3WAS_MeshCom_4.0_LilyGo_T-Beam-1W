# Firmware Upload

* **Variante 1**: aus **VSC/PlatformIO** siehe [Pkt. 7](https://github.com/karamo/MeshCom_4.0_LilyGo_T-Beam-1W/blob/main/README.md#wie-kompilieren-%EF%B8%8F)
* **Variante 2**: nativ mit **`esptool`** auf Commandozeilen-Ebene z.B. mittels .bat-Datei (siehe **Releases**)
* **Variante 3**: https://esptool.spacehuhn.com/ ❗ **Chrome-Browser** ❗
  * ESP32-S3 u.a. Module, die **keinen USB-Serial-Chip** haben, müssen vor dem **`[CONNECT]`** in den **BOOT-Loader-Modus** gebracht werden:  
  ➡️ **BOOT-Taste halten - RESET-Taste drücken und loslassen - BOOT-Taste loslassen**
  * ❗⚠️ wenn direkt mittels (ab) esptool V5.x geflasht wird, ist der Boot-Modus nicht mehr notwendig, aber in der Befehlszeile müssen zusätzliche Befehlsparameter ergänzt werden (... sind entsprechend auszufüllen):  
    `PYTHON -m esptool -p ... --baud 921600 -c esp32s3 --before usb-reset -a watchdog-reset write_flash ...`
  * Wenn nach **`[CONNECT]`** im Textfenster "`Couldn't sync to ESP`" auftritt, dann bedeutet das, dass der ESP32 **nicht** im BOOT-Loader-Modus war.
  * Für die verschiedenen Module gibt es dezitierte Verzeichnisse, in denen sich die FW-Dateien (`*.bin`) befinden, die im [esptool](https://esptool.spacehuhn.com) lt. Bild auf richtiger Adresse ausgewählt werden müssen und anschließend **`[PROGRAM]`**
  * nach dem Flashen **RST/RESET**-Taste am Modul drücken und gleich darauf den **Serial-Monitor in VCS** starten (ist gleichbedeutend mit einem Terminal).
  * <img width="152" height="61" alt="grafik" src="https://github.com/user-attachments/assets/16ec3eed-cbb5-451f-a08f-72886bdff5b9" />

  * **❗⚠️ Normaler Modus und BOOT-Loader-Modus sind je nach Board u.U. auf verschiedenen COM-Ports (Windows).**
  * **❗⚠️ Die Adresse von bootloader.bin (ESP32) und bootloader-s3.bin (ESP32-S3) sind unterschiedlich.**
  * 🟢 **bei nachfolgenden FW-Updates ist nur mehr die `firmware.bin` zu flashen erforderlich.**
  * **❗⚠️ Die <ins>Examples</ins> und die <ins>reguläre MeshCom-FW</ins> unterscheiden sich erheblich in der Anzahl der Dateien, die zu flashen sind. Daher sind die Flash-Adressen für die MC-FW in den jeweiligen Releases angegeben.❗**

### Linux Spezialität
<img width="616" height="150" alt="grafik" src="https://github.com/user-attachments/assets/f53dea6c-f165-4fc7-96ea-28b8bebfff4c" />

#### Links zu diesem Thema:
https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/boot-mode-selection.html#select-bootloader-mode

z.B. auch hier wird es korrekt beschrieben: https://prilchen.de/belegungsplan-esp32-s3/  
**Boot Taste** Download-Schaltfläche. _Wenn du Boot gedrückt hast und dann Reset drückst, wird der Firmware-Download-Modus für das Herunterladen von Firmware über die serielle Schnittstelle gestartet._  
**Reset Taste** _Drücken Sie diese Taste, um das System neu zu starten._

**Google KI:** <i>"Zum manuellen Eintritt in den Download-Modus (Bootloader-Sequenz) wird meist die BOOT-Taste (GPIO0) gedrückt gehalten, während die EN-Taste (Reset) betätigt wird."</i>

----
## Flash-Adressen für die Examples (exemplarisch)

* **ESP32**  (`T3_V1_6_SX1278` u.ä.)  
<img width="330" height="239" alt="grafik" src="https://github.com/user-attachments/assets/70ceea55-2270-4aac-90ce-d16d2afbb64a" />

----
* **ESP32-S3**  (`T_BEAM_1W_433M` u.ä.)  
<img width="339" height="236" alt="grafik" src="https://github.com/user-attachments/assets/869b74d0-da1f-4b65-9130-b35545079fcb" />

----
***:copyright: 11.3.2026 by OE3WAS - Wolfgang***
