/*
   RadioLib Transmit with Interrupts Example

   This example transmits packets using SX1276/SX1278/SX1262/SX1268/SX1280/LR1121 LoRa radio module.
   Each packet contains up to 256 bytes of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
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

//=======================================================================================
#ifndef CONFIG_RADIO_OUTPUT_POWER
    #define CONFIG_RADIO_OUTPUT_POWER   10
#endif

#if defined(USING_SX1262)
    SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
#endif // USING_SX1262

#if     defined(USING_SX1276)
    SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);
#endif

static int transmissionState = RADIOLIB_ERR_NONE;  // save transmission state between loops
static volatile bool transmittedFlag = false;  // flag to indicate that a packet was sent
static uint32_t counter = 0;
static String payload;

// this function is called when a complete packet is transmitted by the module
// IMPORTANT: this function MUST be 'void' type and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) { transmittedFlag = true; } // we sent a packet, set the flag

//=======================================================================================
uint8_t  display_address = 0x3c;    // It might be 0x3D
DISPLAY_MODEL *disp = NULL;
#define U8G2_HOR_ALIGN_CENTER(t)    ((disp->getDisplayWidth() -  (disp->getUTF8Width(t))) / 2)
#define U8G2_HOR_ALIGN_RIGHT(t)     ( disp->getDisplayWidth()  -  disp->getUTF8Width(t))

bool beginDisplay()
{
    Wire.beginTransmission(display_address);
    if (Wire.endTransmission() == 0) {
        disp = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        Serial.printf("Find Display model at 0x%X address\n", display_address);
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
    Serial.printf("Warning: Failed to find Display at 0x%0X address\n", display_address);
    return false;
}

//=======================================================================================
void drawMain()
{
    if (disp) {
        disp->clearBuffer();
        disp->drawRFrame(0, 0, 128, 64, 5);
        disp->setFont(u8g2_font_pxplusibmvga8_mr);
        disp->setCursor(5, 20);   disp->print("TX:");
        disp->setCursor(5, 35);   disp->print("STATE:");

        disp->setFont(u8g2_font_crox1h_tr);
        disp->setCursor( U8G2_HOR_ALIGN_RIGHT(payload.c_str()) - 5, 20 );
        disp->print(payload);

        String state = transmissionState == RADIOLIB_ERR_NONE ? "OK" : String(transmissionState);
        disp->setCursor( U8G2_HOR_ALIGN_RIGHT(state.c_str()) -  21, 35 );
        disp->print(state);
        disp->sendBuffer();
    }
}


//=======================================================================================
void setup()
{
    timerSerial.start(2000);
    Serial.begin(115200);
    while (!Serial && !timerSerial.time_over());
    if (Serial) { for (int i=0;i<10;i++) { Serial.println("."); delay(1000); } }

    #ifdef I2C_SDA
        Wire.begin(I2C_SDA, I2C_SCL);
    #endif

    Serial.println("\nSetup Board");
    
    beginDisplay();

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
        // T-BEAM-1W LoRa RX/TX Control. RADIO_CTRL controls the LNA, not the PA.
        // Only when RX DATA set to 1 to turn on LNA.
        // When TX DATA set to 0 to turn off LNA
        pinMode(RADIO_CTRL, OUTPUT);
        digitalWrite(RADIO_CTRL, HIGH);  // RX Mode
        delay(500);
    #endif

    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

    Serial.println("... radio.begin ...");
    int state = radio.begin(CONFIG_RADIO_FREQ);    // initialize radio with default settings
    Serial.printf("... radio state: %u\n", state);

    //radio.setTCXO(3.0);

    Serial.printf("[%s]:", RADIO_TYPE_STR);
    Serial.print("Radio Initializing ... ");
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("success!");
    } else {
        Serial.printf("failed, code %u >>>>>> stopped", state);
        while (true);
    }

    // set the function that will be called when packet transmission is finished
    radio.setPacketSentAction(setFlag);

    // Sets carrier frequency.
    // SX1268/SX1262 : Allowed values are in range from 150.0 to 960.0 MHz
    if (radio.setFrequency(CONFIG_RADIO_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY) {
        Serial.println("Selected frequency is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Sets LoRa link bandwidth.
    // SX1268/SX1262 : Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz
    if (radio.setBandwidth(CONFIG_RADIO_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
        Serial.println("Selected bandwidth is invalid for this module! >>>>> stopped");
        while (true);
    }

    // SX1262 :  LoRa link spreading factor allowed values range from 5 to 12
    if (radio.setSpreadingFactor(CONFIG_RADIO_SF) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
        Serial.println("Selected spreading factor is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Sets LoRa coding rate denominator.
    // SX1278/SX1276/SX1268/SX1262 : Allowed values range from 5 to 8. Only available in LoRa mode
    if (radio.setCodingRate(CONFIG_RADIO_CR) == RADIOLIB_ERR_INVALID_CODING_RATE) {
        Serial.println("Selected coding rate is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Sets LoRa sync word.
    // SX1278/SX1276/SX1268/SX1262/SX1280 : Sets LoRa sync word. Only available in LoRa mode
    if (radio.setSyncWord(CONFIG_RADIO_SW) != RADIOLIB_ERR_NONE) {
        Serial.println("Unable to set sync word! >>>>> stopped");
        while (true);
    }

    // Sets transmission output power.
    // SX1262 :  Allowed values are in range from -9 to 22 dBm.
    // This method is virtual to allow override from the SX1261 class
    if (radio.setOutputPower(CONFIG_RADIO_OUTPUT_POWER) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println("Selected output power is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Sets current limit for over current protection at transmitter amplifier.
    // SX1262/SX1268 : Allowed values range from 45 to 120 mA in 2.5 mA steps and 120 to 240 mA in 10 mA steps
    // NOTE: set value to 0 to disable overcurrent protection
    if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
        Serial.println("Selected current limit is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Sets preamble length for LoRa or FSK modem.
    // SX1262/SX1268 : Allowed values range from 1 to 65535.
    if (radio.setPreambleLength(CONFIG_RADIO_PRE) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
        Serial.println("Selected preamble length is invalid for this module! >>>>> stopped");
        while (true);
    }

    // Enables or disables CRC check of received packets.
    if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
        Serial.println("Selected CRC is invalid for this module! >>>>> stopped");
        while (true);
    }


#ifdef RADIO_CTRL
    Serial.println("Turn off LNA, Turn on PA, Enter Tx mode.");
    digitalWrite(RADIO_CTRL, LOW);
#endif /*RADIO_CTRL*/

    Serial.println("Setup finished");

    payload = "HELLO";
    Serial.printf("Radio Sending packet: %s ... ", payload.c_str());
    transmissionState = radio.startTransmit(payload);
    drawMain();
}

//=======================================================================================
void loop()
{
    if (transmittedFlag) {    // check if the previous transmission finished
        transmittedFlag = false;  // reset flag

        if (transmissionState == RADIOLIB_ERR_NONE) {
            Serial.println("transmission finished!");  // packet was successfully sent
            // NOTE: when using interrupt-driven transmit method,
            // it is not possible to automatically measure transmission data rate using getDataRate()
        } else {
            Serial.printf("failed, code %u", transmissionState);
        }

        delay(5000);  // wait a second before transmitting again

        payload = "TB-1W #" + String(counter++);
        drawMain();

        Serial.printf("Radio Sending packet: %s ... ", payload.c_str());
        transmissionState = radio.startTransmit(payload);
    }
}
