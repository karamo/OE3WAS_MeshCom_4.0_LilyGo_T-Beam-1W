## RadioLib_PingPong

#### ✅ derzeit unterstützte Boards
* **T3_V1_6_SX1276** (@433 MHz)
* **T_BEAM_1W_433M** (SX1262/61 @433 MHz)
* **ESP32-E22-PCB** (SX1268 @433 MHz)

---
#### 🔥 in Planung
* ...

---
#### ✅ default Parameter
* QRG = 433.525 MHz
* TXpower = 10 dBm = 10 mW

![PingPong_v1 06_400](https://github.com/user-attachments/assets/159ff0c4-abdb-4351-b711-4545937ec14a)

---
#### ✅ Befehle
* ein "**`:`**" leitet eine **Textmessage** ein, welche an das remote-Modul via LoRa gesendet wird.
* Befehle an das remode-Modul werden als Textmessages übertragen.
* ein "**`+`**" leitet einen **lokalen Befehl** ein
* ab **v1.06**
  *  **`PING`** an alle oder selektive Adressierung an ID, der am Screen links unten angezeigt wird.
  * **Randomisierte Sendezeitpunkte**
* ab **v1.07**
  * Kollisionserkennung und Kollisionsvermeidung **Listen-Before-Talk (LBT)** bzw. **Channel Activity Detection (CAD)**
 
  
```
[D] 1 : ?
[D]Befehlsliste ...

[I]PingPong v1.06
[I] ?                   list all commands
[I] :textmessage        TX_LORA textmessage
[I] :PING *             fordert PONG von ALLEN an
[I] :PING ID            fordert PONG von ID an
[I] :CLEAR              löscht Screen
[I] :TXpower xx         set TXpower remote
[I] :RXboost {on|off}   setRXboostedGain remote (SX1261/2 v2.1, SX1268 v1.1)
[I] :QRG ffffff         setzt Frequenz [kHz] remote
[I] :REBOOT             reboot remote ESP32

[I] +TXpower xx         set TXpower local
[I] +RXboost {on|off}   setRXboostedGain local (SX1261/2 v2.1, SX1268 v1.1)
[I] +QRG ffffff         setzt Frequenz [kHz] lokal
[I] +REBOOT             reboot local ESP32
[D]----------------------------------------------
```
#### ✅ TXpower
Sets transmission output power.
| Chip | TXpower range |
| ----- | ----- |
| SX1262/SX1268 | -9 to 22 dBm virtual to allow override from the SX1261 class |
| SX1276/SX1278 | -3 to 15 dBm (RFO pin) or +2 to +17 dBm (PA_BOOST pin) High power +20 dBm operation is also supported, on the PA_BOOST pin. Defaults to PA_BOOST |

---
#### ⚠️ Anwendung & Nahfeld vs. Fernfeld
* für kurze Antennen **L<λ** ist das Nahfeld bis **≈2λ**, dh. für 433MHz(=70cm) entspricht das **≈1,4 m**
* valide Messungen sind aber m.E. nur im Fernfeld sinnvoll, welches **>4λ** beträgt. @433MHz also ab **>2,8 m**
* bei sehr geringem Antennenabstand und abhängig von der Sendeleistung und Antenne erfolgt eine Übersteuerung des RX-Pfades, was sich dadurch bemerkbar macht, dass der **RSSI = 0.00 dBm** ausgewiesen wird!

➡️ https://de.wikipedia.org/wiki/Nahfeld_und_Fernfeld_(Antennen)

---
#### Links
* https://www.digikey.at/de/resources/conversion-calculators/conversion-calculator-dbm-to-watts
* **LBT**: https://www.multitech.net/developer/software/lora/listen-before-talk/
* **CAD**: RadioLib Examples SX126x u. SX127x 


⚠️▶️➡️✔️✅🔹🔥
---
***:copyright: 3.2.2026 by OE3WAS - Wolfgang***
