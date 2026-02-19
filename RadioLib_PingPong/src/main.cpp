/*
    Test-FW Ping-Pong [OE3WAS]

    basierend auf:
    * RadioLib Transmit with Interrupts Example
    * RadioLib Receive with Interrupts Example
    * RadioLib PingPong
    * For full API reference: https://jgromes.github.io/RadioLib/
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_mac.h>
#include "soc/rtc.h"

#if defined(ARDUINO_ARCH_ESP32)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#include "hal/gpio_hal.h"
#endif
#include "driver/gpio.h"
#endif //ARDUINO_ARCH_ESP32

#include <U8g2lib.h>
#include <RadioLib.h>

#include "configuration.h"

//====== Timer for periodical events u.a.
#include "Timeout.h"
Timeout timerSerial;

#define VERSION_STRING "PingPong v1.14" 
// v1.09 add SX1276 for 868 MHz

//=======================================================================================
uint8_t  display_address = 0x3c;    // It might be 0x3D
DISPLAY_MODEL *disp = NULL;
#define U8G2_HOR_ALIGN_CENTER(t)    ((disp->getDisplayWidth() -  (disp->getUTF8Width(t))) / 2)
#define U8G2_HOR_ALIGN_RIGHT(t)     ( disp->getDisplayWidth()  -  disp->getUTF8Width(t))

//=======================================================================================
// 10 dBm = 10 mW
#ifndef CONFIG_RADIO_OUTPUT_POWER
    #define CONFIG_RADIO_OUTPUT_POWER   10
#endif

#if defined(USING_SX1262)
    SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
#endif // USING_SX1262

#ifdef USING_SX1268
    // cs - irq - reset - interrupt gpio
    // If you have RESET of the E22 connected to a GPIO on the ESP you must initialize the GPIO as output and perform a LOW - HIGH cycle, 
    // otherwise your E22 is in an undefined state. RESET can be connected, but is not a must. IF so, make RESET before INIT!
    SX1268 radio = new Module(SX1268_CS, SX1268_IRQ, SX1268_RST, SX1268_GPIO);
#endif

#if defined(USING_SX1276)
    SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);
#endif

#if defined(USING_SX1278)
    SX1278 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);
#endif

static int transmissionState = RADIOLIB_ERR_NONE;  // save transmission state between loops
static volatile bool transmitFlag = false;  // flag to indicate transmission or reception state
static volatile bool operationDone = false; // flag to indicate that a packet was sent or received
static volatile bool receiveFlag = false;   // for SX1276 ()

uint8_t mac[6];
char macStr[18] = { 0 };
char macStr6[18] = { 0 };
unsigned long deltatime;

char receivedChar;
const byte numChars = 250;
char inLine[numChars];   // an array to store the received data
bool newData = false;

static String TXpayload;
static String RXpayload;
static String rssi;
static String snr;
static String power;
float LocalFreq;

//=======================================================================================
// this function is called when a complete packet is transmitted by the module
// IMPORTANT: this function MUST be 'void' type and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) { operationDone = true; } // we sent or received a packet

void setReceiveFlag(void) { receiveFlag = true;
  //operationDone = true;
}  // for SX1276



//=======================================================================================
bool beginDisplay()
{
  #if OLED_RST
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, HIGH); delay(20);
  digitalWrite(OLED_RST, LOW);  delay(20);
  digitalWrite(OLED_RST, HIGH); delay(20);
  #endif

  Wire.beginTransmission(display_address);
  if (Wire.endTransmission() == 0) {
    disp = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
    Serial.printf("[I]Find Display model at 0x%X address\n", display_address);
    disp->begin();
    disp->clearBuffer();
    disp->setFont(u8g2_font_inb19_mr);
    disp->drawStr(0, 30, "LilyGo");
    disp->drawHLine(2, 35, 47);
    disp->drawHLine(3, 36, 47);
    disp->drawVLine(45, 32, 12);
    disp->drawVLine(46, 33, 12);
    disp->setFont(u8g2_font_inb19_mf);
    disp->drawStr(58, 60, "LoRa");
    disp->sendBuffer();
    disp->setFont(u8g2_font_fur11_tf);
    delay(3000);
    return true;
  }
  Serial.printf("[E]Warning: Failed to find Display at 0x%0X address\n", display_address);
  return false;
}

//=======================================================================================
void drawStart()
{
  if (disp) {
    disp->clearBuffer();
    disp->drawRFrame(0, 0, 128, 64, 5);
    disp->setFont(u8g2_font_crox1h_tr);
    disp->setCursor(4, 12);   disp->print("FW:");
    disp->setCursor(5, 24);   disp->print("     ");
    disp->setCursor(4, 36);   disp->print("QRG:");
    disp->setCursor(5, 48);   disp->print("     ");
    disp->setCursor(5, 60);   disp->printf("ID: %s", macStr);

    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(VERSION_STRING) - 5, 12 );
    disp->print(VERSION_STRING);
    char buff[30];
    snprintf(buff,sizeof(buff), "%.3f MHz", LocalFreq);
    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(buff) - 5, 36 );
    disp->print(buff);

    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(power.c_str()) - 5, 60 );
    disp->print(power);
    disp->sendBuffer();
  }
}

//=======================================================================================
void drawMain()
{
  if (disp) {
    disp->clearBuffer();
    disp->drawRFrame(0, 0, 128, 64, 5);
    disp->setFont(u8g2_font_crox1h_tr);
    disp->setCursor(4, 12);   disp->print("TX:");
    disp->setCursor(4, 24);   disp->print("RX:");
    disp->setCursor(5, 36);   disp->print("RSSI:");
    disp->setCursor(5, 48);   disp->print("SNR:");
    disp->setCursor(5, 60);   disp->printf("ID: %s", macStr);

    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(TXpayload.c_str()) - 5, 12 );
    disp->print(TXpayload);
    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(RXpayload.c_str()) - 5, 24 );
    disp->print(RXpayload);

    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(rssi.c_str()) - 5, 36 );
    disp->print(rssi);
    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(snr.c_str()) - 5, 48 );
    disp->print(snr);
    disp->setCursor( U8G2_HOR_ALIGN_RIGHT(power.c_str()) - 10, 60 );
    disp->print(power);
    disp->sendBuffer();
  }
}

//==============================================================================
void recvWithEndMarker() {
  static byte ndx = 0;
  char endMark1 = '\n'; // LF
  char endMark2 = '\r'; // CR
  char rc;
  bool endLine = false;
  
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    Serial.print(rc);  // echo local
    endLine = ((rc == endMark1) || (rc == endMark2));

    if (!endLine) {
      inLine[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {  // auf die def. Anzahl der Zeichen begrenzt
          ndx = numChars - 1;
      }
    } else {
      inLine[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

//==============================================================================
std::string helpStr = "[D]Eingabe: :Text | +Befehl | ?Abfrage | ? Hilfe";
char strSendText[250]; // for LoRa
int isTXLORA = 1; // 0=off, 1=on, 2=auto
bool isTXLoRaStr = false; // flag for TX_LORA data
std::string fullLine;
std::string cmd = "";
std::string pars = "";
unsigned long int intval = 0;

//==============================================================================
void setParameter(char* inLine) {
  Serial.printf("[D]setParameter: %s\n",inLine);
  //std::string para = inLine;
  int num = strlen(inLine);
  cmd = "";
  pars = "";

  // Eingabe zerteilen
  fullLine.assign(inLine);
  Serial.printf("in: %s |full: %s\n", inLine, fullLine.c_str());
  size_t pos = fullLine.find(" ");
  if (pos != std::string::npos) {  //"cmd xxxxxx"
    cmd = fullLine.substr(0, pos);   //extract cmd
    pars = fullLine.erase(0, pos+1);       //remove cmd from input, remains pars
  } else { cmd = fullLine; }        //only "cmd"
  // pars "" of more parameters
  bool isInt = false;
  if (pars != "") {
    try
    {
      intval = stoul(pars);  // versuche Integer
      Serial.printf("[D] cmd='%s' pars='%s' intval=%lu\n", cmd.c_str(),pars.c_str(),intval);
      isInt = true;
    }
    catch(const std::exception& e)
    {
      intval = 0;
      Serial.printf("[D] cmd='%s' pars='%s'\n", cmd.c_str(),pars.c_str());      
    }
  }  
    
  //----------------------------------
  if ((cmd == "+TXpower") && (pars != "")) {
    Serial.printf("[D] set TXpower local to %lu\n", intval);
    if (radio.setOutputPower(intval) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println("Selected output power is invalid for this module!"); }
    radio.startReceive();  // listen for response
    TXpayload = "";
    rssi = "";
    power = String(intval)+" dBm";
    drawMain();
  }  
  #if defined(USING_SX1262) || defined(USING_SX1268)
  if ((cmd == "+RXboost") && (pars != "")) {
    if (pars == "on") radio.setRxBoostedGainMode(true);
    if (pars == "off") radio.setRxBoostedGainMode(false);
  }
  #endif

  if ((cmd == "+QRG") && (pars != "")) {
    LocalFreq = float(intval)/1000.0;
    Serial.printf("[D] set Frequency local to %.3f MHz\n", LocalFreq);
    if (radio.setFrequency(LocalFreq) == RADIOLIB_ERR_INVALID_FREQUENCY) {
      Serial.println("Selected frequency is invalid for this module!"); }
    radio.startReceive();  // listen for response
    drawStart();
    delay(5000);
    drawMain();
  }

  if (cmd == "+CLEAR") {
    drawStart();
    delay(5000);
    TXpayload = "";
    RXpayload = "";
    snr = "";
    rssi = "";
    drawMain();
  }

  if (cmd == "+REBOOT") {
    TXpayload = "";
    RXpayload = "=>REBOOT";
    snr = "";
    rssi = "";
    drawMain();
    #ifdef ESP32S3_DEV
      esp_restart(); // ESP32-S3
    #elif defined(ESP32)
      ESP.restart();
    #endif
  }
}

//==============================================================================
void outputParameterabfrage(char* inLine) {}

//==============================================================================
void outputBefehlsliste() {
  Serial.println("[D]Befehlsliste ...");
  Serial.printf("\n[I]%s\n", VERSION_STRING); //CRLF
  Serial.printf("[I] ?                   list all commands\n");
  Serial.printf("[I] :textmessage        TX_LORA textmessage\n");
  Serial.printf("[I] :PING *             fordert PONG von ALLEN an\n");
  Serial.printf("[I] :PING ID            fordert PONG von ID an\n");
  Serial.printf("[I] :CLEAR              löscht Screen\n");
  Serial.printf("[I] :TXpower xx         set TXpower remote\n");
  Serial.printf("[I] :RXboost {on|off}   setRXboostedGain remote (SX1261/2 v2.1, SX1268 v1.1)\n");
  Serial.printf("[I] :QRG ffffff         setzt Frequenz [kHz] remote\n");
  Serial.printf("[I] :REBOOT             reboot remote ESP32\n");
  Serial.printf("\n");
  Serial.printf("[I] +TXpower xx         set TXpower local\n");
  Serial.printf("[I] +RXboost {on|off}   setRXboostedGain local (SX1261/2 v2.1, SX1268 v1.1)\n");
  Serial.printf("[I] +QRG ffffff         setzt Frequenz [kHz] lokal\n");
  Serial.printf("[I] +CLEAR              löscht Screen\n");
  Serial.printf("[I] +REBOOT             reboot local ESP32\n");
  Serial.printf("[D]----------------------------------------------\n");
  // ...
}

//==============================================================================
void getTextToSend(char* inLine) {
  //snprintf(strSendText, sizeof(strSendText), inLine);
  snprintf(strSendText, sizeof(strSendText), inLine);
  if (isTXLORA >0) {isTXLoRaStr = true;} // set flag for TX_LORA data
}

//==============================================================================
void Parser() {
  if (newData) {  // Eingabe erfolgt
    int num = strlen(inLine);
    if (num>0) {
      Serial.printf("[D] %i : %s\n",num,inLine); // echo input
      if ((inLine[0]=='?') && (num==1)) { outputBefehlsliste(); }
        else if ((inLine[0]=='?') && (num>=2)) { outputParameterabfrage(inLine); }
          else if ((inLine[0]=='+') && (num>=2)) { setParameter(inLine); }
            else if ((inLine[0]=='-') && (inLine[1]=='-') && (num>=3)) { setParameter(inLine); } // auch --befehl unterstützen
              else if (inLine[0]==':') { getTextToSend(inLine); }
                else { Serial.printf("[E] '%s' unbekannter Befehl\n",inLine); //CRLF
                       Serial.println(helpStr.c_str()); //CRLF
                }
    }
    newData = false;
  }
}

//=======================================================================================
void setup()
{
  timerSerial.start(2000);
  Serial.begin(115200);
  while (!Serial && !timerSerial.time_over());
  //#if defined(T_BEAM_1W)
  if (Serial) { for (int i=0;i<10;i++) { Serial.println("."); delay(1000); } }
  //#endif

  #ifdef I2C_SDA
    Wire.begin(I2C_SDA, I2C_SCL);
  #endif

  Serial.printf("\n[D]Setup Board\n");
  
  beginDisplay();

  #if defined(FAN_CTRL)
    pinMode(FAN_CTRL, OUTPUT);
    digitalWrite(FAN_CTRL, HIGH);
  #endif

  Serial.println(VERSION_STRING);

  #if defined(HAS_SDCARD) && defined(SD_SHARE_SPI_BUS)
    // Share spi bus with lora , set lora cs,rst to high
    pinMode(RADIO_CS_PIN, OUTPUT);
    pinMode(RADIO_RST_PIN, OUTPUT);
    digitalWrite(RADIO_CS_PIN, HIGH);
    digitalWrite(RADIO_RST_PIN, HIGH);
  #endif

  #ifdef RADIO_LDO_EN
    // T-BEAM-1W Control SX1262, LNA, must set RADIO_LDO_EN to HIGH to power the Radio
    pinMode(RADIO_LDO_EN, OUTPUT);
    digitalWrite(RADIO_LDO_EN, HIGH);
    delay(500);
  #endif

  #ifdef RADIO_CTRL
    // T-BEAM-1W LoRa RX/TX Control.
    // RADIO_CTRL = HIGH Only when RX DATA to turn on LNA.
    // RADIO_CTRL = LOW when TX DATA to turn off LNA
    pinMode(RADIO_CTRL, OUTPUT);
    digitalWrite(RADIO_CTRL, HIGH);  // RX Mode
    delay(500);
  #endif

  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);

  Serial.printf("[D]... radio.begin ...");
  int state = radio.begin(CONFIG_RADIO_FREQ, CONFIG_RADIO_BW, CONFIG_RADIO_SF, CONFIG_RADIO_CR, CONFIG_RADIO_SW, CONFIG_RADIO_OUTPUT_POWER, CONFIG_RADIO_PRE);    // initialize radio 
  Serial.printf("[D][%s]:", RADIO_TYPE_STR);
  Serial.printf("[D]... radio state: %u\n", state);  // 0 = OK
  if (state == RADIOLIB_ERR_NONE) { Serial.println("success!");
  } else {
    Serial.printf("failed, code %u >>>>>> stopped", state);
    while (true);
  }
  //radio.setTCXO(3.0);

  #if defined(BOARD_E22) || defined(BOARD_E22_S3)
    // if RESET Pin is connected
    pinMode(LORA_RST, PULLUP);
    digitalWrite(LORA_RST, LOW);
    delay(200);
    digitalWrite(LORA_RST, HIGH);

    // we have TXEN and RXEN Pin connected
    radio.setRfSwitchPins(E22_RXEN, E22_TXEN);
  #endif
  
  #if defined(T3_V1_6_SX1278)
    // set the function that will be called when new packet is received
    radio.setDio0Action(setFlag, RISING);
  #endif

  #if defined(T3_V1_6_SX1276)
    // set the function that will be called when new packet is received
    radio.setPacketReceivedAction(setReceiveFlag);
    radio.setDio0Action(setFlag, RISING); // testweise
  #endif

  // auch für E22-PCB ???
  #if defined(USING_SX1262) || defined(BOARD_E22)
    // set the function that will be called when new packet is received
    radio.setDio1Action(setFlag);
  #endif

  // Sets carrier frequency.
  // SX1268/SX1262 : Allowed values are in range from 150.0 to 960.0 MHz
  // SX1276 : Allowed values range from 137 MHz to 1020 MHz.
  // SX1278 : Allowed values range from 137 MHz to 525 MHz.
  if (radio.setFrequency(CONFIG_RADIO_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println("[E]Selected frequency is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Sets LoRa link bandwidth.
  // SX1268/SX1262 : Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz
  // SX1278/SX1276 : Allowed values are 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250 and 500 kHz. Only available in %LoRa mode.
  if (radio.setBandwidth(CONFIG_RADIO_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println("[E]Selected bandwidth is invalid for this module! >>>>> stopped");
    while (true);
  }

  // SX1262 (SX1268) : LoRa link spreading factor allowed values range from 5 to 12
  // SX1278/SX1276   : Allowed values range from 6 to 12. Only available in LoRa mode.
  if (radio.setSpreadingFactor(CONFIG_RADIO_SF) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println("[E]Selected spreading factor is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Sets LoRa coding rate denominator.
  // SX1278/SX1276/SX1268/SX1262 : Allowed values range from 5 to 8. Only available in LoRa mode
  if (radio.setCodingRate(CONFIG_RADIO_CR) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    Serial.println("[E]Selected coding rate is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Sets LoRa sync word.
  // SX1278/SX1276/SX1268/SX1262/SX1280 : Sets LoRa sync word. Only available in LoRa mode
  if (radio.setSyncWord(CONFIG_RADIO_SW) != RADIOLIB_ERR_NONE) {
    Serial.println("[E]Unable to set sync word! >>>>> stopped");
    while (true);
  }

  // Sets transmission output power.
  // SX1262/SX1268 :  Allowed values are in range from -9 to 22 dBm.
  //                  This method is virtual to allow override from the SX1261 class
  // SX1276/SX1278 :  Allowed values range from -3 to 15 dBm (RFO pin) or +2 to +17 dBm (PA_BOOST pin).
  //     High power +20 dBm operation is also supported, on the PA_BOOST pin. Defaults to PA_BOOST.
  if (radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println("[E]Selected output power is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Sets current limit for over current protection at transmitter amplifier.
  // SX1262/SX1268 : Allowed values range from 45 to 120 mA in 2.5 mA steps and 120 to 240 mA in 10 mA steps
  // SX1278/SX1276 : Allowed values range from 45 to 120 mA in 5 mA steps and 120 to 240 mA in 10 mA steps.
  // NOTE: set value to 0 to disable overcurrent protection
  if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    Serial.println("[E]Selected current limit is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Sets preamble length for LoRa or FSK modem.
  // SX1262/SX1268 : Allowed values range from 1 to 65535.
  // SX1278/SX1276 : Allowed values range from 6 to 65535 in LoRa mode or 0 to 65535 in FSK mode.
  if (radio.setPreambleLength(CONFIG_RADIO_PRE) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    Serial.println("[E]Selected preamble length is invalid for this module! >>>>> stopped");
    while (true);
  }

  // Enables or disables CRC check of received packets
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    Serial.println("[E]Selected CRC is invalid for this module! >>>>> stopped");
    while (true);
  }


  #ifdef RADIO_CTRL
    Serial.println("Turn on LNA, Enter Rx mode.");
    digitalWrite(RADIO_CTRL, HIGH);  // RX mode
  #endif /*RADIO_CTRL*/

  #ifdef USING_DIO2_AS_RF_SWITCH
  #ifdef USING_SX1262
    // Some SX126x modules use DIO2 as RF switch.
    // To enable this feature, the following method can be used.
    // NOTE: As long as DIO2 is configured to control RF switch, it can't be used as interrupt pin!
    if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
      Serial.println(F("Failed to set DIO2 as RF switch!"));
      while (true);
    }
  #endif //USING_SX1262
  #endif //USING_DIO2_AS_RF_SWITCH

  delay(1000);

  esp_efuse_mac_get_default(mac);
  sprintf(macStr6, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("EFUSE MAC: %s\n", macStr6);
  sprintf(macStr, "%02X%02X", mac[4], mac[5]);
  deltatime = (mac[4] * 256 + mac[5])/30 + 100;  // Delta-Zeitraster aus MAC pseudo Random
  Serial.printf("deltatime= %lu\n", deltatime);

  RXpayload = "";
  LocalFreq = CONFIG_RADIO_FREQ;
  rssi = "";
  snr = "";
  // macStr bereits zuvor zugewiesen
  power = String(CONFIG_RADIO_OUTPUT_POWER)+" dBm";
  drawStart();

  Serial.println("[D]Setup finished");

  transmissionState = radio.startTransmit("Hello World!");
  transmitFlag = true;

/*
  // start listening for LoRa packets
  Serial.print("[D]Radio Starting to listen ... ");
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) { Serial.println("success!");
  } else { Serial.printf("failed, code %u\n", state); }
*/
  isTXLoRaStr = false;
}

//=======================================================================================
void loop()
{
  recvWithEndMarker();        // Befehlseingaben abfragen und verarbeiten
  //if (strlen(inLine) >0) Serial.printf("[<] %s\n", inLine);
  if (newData) { Parser(); }  // ... und parsen

  if (receiveFlag) {
    Serial.println("receiveFlag !");
    receiveFlag = false;
  }

  if (isTXLoRaStr) {

/*  // siehe RadioLib: macht Problem bez. DIO0/DIO1 falls es die verschiednen Boards bzw. SX12xx nicht unterstützen bzw. nicht verfügbar ist.
    Serial.printf("[SX12xx] Prüfe Kanal (LBT/CAD)...\n");
    // Startet CAD. Erkennt LoRa-Signale auf dem aktuellen Kanal.
    int state = radio.scanChannel();
    if (state == RADIOLIB_LORA_DETECTED) { // Kanal belegt!
      Serial.printf("Kanal belegt! Warte Random ...\n");
      delay(random(100, 1000)); // Zufällige Pause zur Kollisionsvermeidung
    } else {
      if (state == RADIOLIB_CHANNEL_FREE) { Serial.printf("Kanal frei! ...\n"); }
    }
*/
        isTXLoRaStr = false;
        TXpayload = String(strSendText);
        RXpayload = "";  // ev. nur löschen, wenn kein Response?
        rssi = "";
        snr = "";
        drawMain();
        #ifdef RADIO_CTRL
          Serial.println("Turn off LNA, Turn on PA, Enter Tx mode.");
          digitalWrite(RADIO_CTRL, LOW);  // TX mode
        #endif /*RADIO_CTRL*/

        delay(deltatime);  // wait a random moment before transmitting
        transmissionState = radio.startTransmit(TXpayload);
        transmitFlag = true;
        Serial.printf("Radio Sending packet: %s ... ", TXpayload.c_str());
  }

  if(operationDone) {
    operationDone = false;  // reset flag

    if (transmitFlag) {    // check if the previous transmission finished
      // the previous operation was transmission, listen for response, print the result
  
      if (transmissionState == RADIOLIB_ERR_NONE) {
        Serial.println("transmission finished!");  // packet was successfully sent
        // NOTE: when using interrupt-driven transmit method,
        // it is not possible to automatically measure transmission data rate using getDataRate()
      } else { Serial.printf("failed, code %u\n", transmissionState); }

      #ifdef RADIO_CTRL
        Serial.println("Turn on LNA, Enter Rx mode.");
        digitalWrite(RADIO_CTRL, HIGH);  // RX mode
        delay(100);
      #endif /*RADIO_CTRL*/
      radio.startReceive();  // listen for response
      transmitFlag = false;  // reset flag
      
    } else {  // the previous operation was reception
      String payload;
      int state = radio.readData(payload, 0);

      if (state == RADIOLIB_ERR_NONE) // packet was successfully received
      {
        rssi = String(radio.getRSSI()) + " dBm";
        snr = String(radio.getSNR()) + " dB";
        RXpayload = payload;
        drawMain();

        Serial.printf("Radio Received packet: %s |RSSI: %s SNR: %s\n", 
                        RXpayload.c_str(), rssi.c_str(), snr.c_str());

        if (RXpayload == ":CLEAR") {
          RXpayload = "";
          snr = "";
          rssi = "";
          drawStart();
          delay(5000);
          drawMain();
        }

        if (RXpayload == ":REBOOT") {
          TXpayload = "";
          RXpayload = "=>REBOOT";
          snr = "";
          rssi = "";
          drawMain();
          #ifdef ESP32S3_DEV
            esp_restart(); // ESP32-S3
          #elif defined(ESP32)
            ESP.restart();
          #endif
        }

        //strncpy(inLine, RXpayload.c_str(), sizeof(inLine));
        snprintf(inLine, sizeof(inLine),"%s", RXpayload.c_str());
        setParameter(inLine);
        inLine[0] = 0;
        
        if ((cmd == ":PING") && ((pars == macStr) || (pars == "*"))) {
          sprintf(strSendText, "PONG %s %s %s",
                  macStr, String(radio.getRSSI()).c_str(),String(radio.getSNR()).c_str());
          TXpayload = String(strSendText);
          drawMain();
          isTXLoRaStr = true;
        }

        if (cmd == ":TXpower") {
          if (radio.setOutputPower(intval) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
              Serial.println("Selected output power is invalid for this module!"); }
          sprintf(strSendText, "TXpower %s", String(intval).c_str()); //response
          TXpayload = String(strSendText);
          rssi = "";
          snr = "";
          drawMain();
          isTXLoRaStr = true; 
        }

        #ifdef USING_SX1262
        if ((cmd == ":RXboost") && (pars != "")) {
          if (pars == "on") radio.setRxBoostedGainMode(true);
          if (pars == "off") radio.setRxBoostedGainMode(false);
          sprintf(strSendText, "RXboost %s", pars.c_str()); //response
          TXpayload = String(strSendText);
          drawMain();
          isTXLoRaStr = true; 
        }
        #endif
    
        if ((cmd == ":QRG") && (pars != "")) {
          LocalFreq = float(intval)/1000.0;
          if (radio.setFrequency(LocalFreq) == RADIOLIB_ERR_INVALID_FREQUENCY) {
              Serial.println("Selected frequency is invalid for this module!"); }
          sprintf(strSendText, "QRG %.3f MHz", float(intval)/1000.0); //response
          TXpayload = String(strSendText);
          drawStart();
          delay(5000);
          drawMain();
          isTXLoRaStr = true; 
        }

      } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
                 Serial.println("CRC error!");  // packet was received, but is malformed
        } else { Serial.printf("failed, code %u\n", state); } // some other error occurred
    }
  }
  //radio.startReceive(); // put module back to listen mode
}

