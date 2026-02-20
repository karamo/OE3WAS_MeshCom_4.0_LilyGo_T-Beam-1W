<h1 align = "center">🌟LilyGo T-Beam-1W🌟</h1>

* [Implementierung](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/tree/main/MeshCom_FW_4#implementierung---meshcom-fw-435l-mmdd%CE%B1)
* [ADC-Funktion](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/tree/main/MeshCom_FW_4#adc-funktion)
---
## Implementierung - MeshCom FW 4.35l-mm.dd.α
- [x] Start with Fork **FW-Version 4.35k** from [**MeshCom-FW**  ](https://github.com/icssw-org/MeshCom-Firmware)
- [x] **variants** files & include in platformio.ini [variants/LilyGo_T-Beam-1W/*]
- [x] **boards** JSON-Datei [boards/esp32-s3-wroom-1-n16r8.json]
- [x] **pinning** anpassen
- [x] **Display**: SH1106
- [x] **WiFi** als Gateway & **Webserver**
- [X] **BLE**
- [x] **HW-ID** 51 = TBEAM_1W
- [x] On-Board-**LED** (**`--board led {on|off}`**)
- [x] User-**BUTTON** (**`--button gpio 17 --button on --reboot`**, LongPress = Display on/off, Click = Screens blättern, DoubleClick = Track POS sent, TrippleClick = POS-Anzeige on/off)
- [x] **LoRa**: SX1262 [Receive-Interrupt-Example](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/tree/main/examples2/Receive_Interrupt)
- [x] **BATT**
- [x] **Temperaturmessung** und **Lüfter-Steuerung**
- [x] **LoRa**: SX1262 [Transmit-Interrupt-Example](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/tree/main/examples2/Transmit_Interrupt)
- [x] **GPS**: L76K (LAT,LON,ALT,Sats,HDOP,Date, TimeUTC)
- [x] `--setboostedgain {on|off}`
- [x] `--txpower {2..22}` für gefundenem SX1261
- [x] Freigabe aller I²C **Sensoren** (über Zusatzplatine bzw. QWICC-Stecker I²C)
- [x] **OTA**
- [x] **ADC-Funktion**
- [x] FW **4.35k.02.19** eingearbeitet [**FW 4.35l.02.20**](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/releases/tag/v4.35l.02.20)
- [ ] ??? tools/**download_meshcom.py** u.a. noch anpassen ???

---
- [x] Test-FW [**PingPong**](https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/tree/main/RadioLib_PingPong)
- [ ] **`--temp`** Befehl für Anzeige der NTC-Temperatur & Fan-Status
- [ ] **`--pong`** Befehl & Verfahren für **Ping-Pong** Modus
- [ ] **MQTT**

---
## ADC-Funktion

<ins>• **DoxyGen Beschreibung**</ins>  
<img src="https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/blob/main/images/ADC_DoxyGen.PNG" />

<ins>• **Debug Ausgabe**</ins>  
```
[ADC1]; GPIO4; 17:54:49; 143; 0.001; 797; 694; 694.0; 693.6; 693.6
```

<ins>• **JSON Struktur Definition**</ins>  
<img src="https://github.com/karamo/OE3WAS_MeshCom_4.0_LilyGo_T-Beam-1W/blob/main/images/ADC_JSON-Struktur.PNG" />

<ins>• **JSON Ausgabe**</ins>  
```
D{"TYP":"AN","APN":4,"AFC":1,"AK":0,"AFL":false,"ACK":true,"ADC":0.693633,"ADCRAW":797,"ADCE1":693.601,"ADCE2":693.6334,"ADCSL":1,"ADCOF":0,"ADCAT":0}
```
---
***20.2.2026 by OE3WAS - Wolfgang***

