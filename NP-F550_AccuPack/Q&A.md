### • Die 1W, sind die immer aktiv? Muss der Power Regler auf 22 stehen?

A: default ist 8 dBm eingestellt. Du kannst mit **`--TXpower xx`** von **xx = -9 .. +22 dBm** einstellen. Das mapping auf die reale HP-Leistung ist von LilyGo nicht verfügbar und ist auch von der Frequenz abhängig.  
1W = 30 dBm, siehe Originalunterlagen von LilyGo  


Wolf, [03.02.2026 20:49]
Ok, da steht auch das man mit einem Delay Daten senden sollte... Wegen der PA..

wolfgang z, [03.02.2026 20:53]
ist eingebaut in der FW und auch in der PingPong-FW

Wolf, [03.02.2026 20:54]
Findet man denn irgendwo in eine Protokoll eine Information das die PA auch ihren Job macht. Über die Temp oder einen Current.?

wolfgang z, [03.02.2026 20:56]
Nein
1) Ich habe einen 2-Punkt-Regler eingebaut, der die NTC-Temperatur misst und den Lüfter entsprechend ansteuert.
2) Deswegen gibt es ja auch meine PingPong-FW, um verschiedene Boards und Einstellungen und Antennen gegeneinander zu testen.
3) Die NTC-Temperatur und der Status des Lüfters wird am Terminal ausgegeben.

Wolf, [03.02.2026 20:58]
Ok.. Pingpong. Das läuft dann als Alternative zur normalen Firmware? Ich habe jetzt ja das MeshDash wieder verbunden. Das wäre dann alles wieder weg, die Settings usw?

wolfgang z, [03.02.2026 21:00]
Ja, alternativ zur MC-FW, aber die "Gegenstelle" muss die auch geflasht haben.
Settings bleiben, da dieser NVS-Bereich nicht angetastet wird. Man muss nur einfach alle 5 Dateien von MC wieder flashen um zu MC zurück zu kehren.
Daher gibt es von mir auch die .bat Dateien, um das sehr schnell erledigen zu können.

du bist sicher in der Lage, diese  Flash_T-BEAM-1W_full.bat herzunehmen und eine entsprechende .sh Datei zu erstellen.
wäre eine gute Sache, wenn das wer machen würde.

wolfgang z, [03.02.2026 21:04]
NTC-Temp: 25.246_°C 1928_raw 1659.000_mV 9891.50_Ohm
21:04:24;[TEMP];25.25;off

wolfgang z, [03.02.2026 21:05]
Aber Achtung! Es gibt nur eine begrenzte Anzahl von Boards derzeit, die von PongPong unterstützt werden.

Wolf, [03.02.2026 21:06]
Oh, na viele nutzen Heltec V3

wolfgang z, [03.02.2026 21:07]
leider nein, da ich keinen Heltec V3 zum Testen habe.
Hier dazu: https://t.me/c/1987218802/11778/28349

Wolf, [03.02.2026 21:14]
Ok, verstehe. Ich lasse erst mal den 1W so laufen, habe Power auf 12 gestellt. Langsam an.

Wolf, [03.02.2026 21:17]
Mit diesem Akku. Der macht ja keinen Sinn, weil er nicht geladen wird. Was passiert denn, wenn der Akku und USB dran sind? Puff?

wolfgang z, [03.02.2026 21:20]
nix passiert, außer das der PA mit 6,5V statt 5V versorgt wird und damit möglicherweise mehr Leitung abgeben kann.
Beide sind entkoppelt.
Geladen wird der Akku deswegen nicht, da die eine 2S Konfiguration sind und handelsübliches Produkt für Kameras z.B.

Aber ich plane, eine DIY-Lösung mit BMS und USB-Booster.


Wolf, [03.02.2026 21:22]
Der Akku ist dann irgendwann auf 5 Volt runter und bleibt da dann?

wolfgang z, [03.02.2026 21:23]
na ja, 2S = 2x4,2V max. mit Entladeschlussspannung von 2,5V/LiPo = 5V. Dann schaltet der Akku intern selber ab.
Aber dazu kommt es nicht, da der Regler bei <6,5V Schluss macht. Also kommt der Akku niemals in diese Grenzsituation.
Mit 3,25V an einer Zelle ist der LiPo im sicheren Bereich.
