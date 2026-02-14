/**
 * @file      boards.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-04-24
 * @last-update 2025-05-26
 * @modified  OE3WAS
 */

#include "LoRaBoards.h"

#include "soc/rtc.h"

#ifdef ENABLE_BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#endif

#if defined(HAS_SDCARD) && !defined(SD_SHARE_SPI_BUS)
SPIClass SDCardSPI(HSPI);
#endif

#if defined(ARDUINO_ARCH_ESP32)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#include "hal/gpio_hal.h"
#endif

#include "driver/gpio.h"
#endif //ARDUINO_ARCH_ESP32

static DevInfo_t  devInfo;
uint32_t deviceOnline = 0x00;
static void enable_slow_clock();

#ifdef HAS_GPS
static bool find_gps = false;
String gps_model = "None";
#endif

// I2S Devices default address
uint8_t  display_address = 0x3c;    // It might be 0x3D

#ifdef DISPLAY_MODEL
DISPLAY_MODEL *disp = NULL;

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
#endif

#ifdef HAS_SDCARD
bool writeFile(const char *path, const char *buffer)
{
    bool rlst = false;
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    if (file.print(buffer)) {
        Serial.println("File written");
        rlst = true;
    } else {
        Serial.println("Write failed");
        rlst = false;
    }
    file.close();
    return  rlst;
}

bool readFile(const char *path, uint8_t *buffer, size_t size)
{
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    file.read(buffer, size);
    file.close();
    return false;
}

bool testSDWriteAndRead()
{
    const char *path = "/test_sd.txt";
    const char *message = "This is a string for reading and writing SD card.";
    uint8_t buffer[128] = {0};

    if (!writeFile(path, message)) {
        Serial.println("SD Text write failed");
        return false;
    }
    delay(100);

    readFile(path, buffer, 128);

    if (memcmp(buffer, message, strlen(message)) != 0) {
        Serial.println("SD verification failed");
        return false;
    }
    Serial.println("SD verification successful");
    return true;
}

bool beginSDCard()
{
#if defined(HAS_SDCARD) && defined(SD_SHARE_SPI_BUS)
    bool rlst = SD.begin(SDCARD_CS);
#else
    bool rlst = SD.begin(SDCARD_CS, SDCardSPI);
#endif

    if (rlst) {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("Sd Card init succeeded. The current available capacity is %.3f GB\n",cardSize / 1024.0);
        deviceOnline |= SDCARD_ONLINE;
        return testSDWriteAndRead();
    } else {
        Serial.println("Warning: Failed to init Sd Card");
    }
    return false;
}
#endif /*HAS_SDCARD*/

/**
 * @brief Start WiFi softAP
 * 
 */
void beginWiFi()
{
#ifdef ARDUINO_ARCH_ESP32
    if (!WiFi.softAP(BOARD_VARIANT_NAME)) {
        log_e("Soft AP creation failed.");
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
#endif
}


void printWakeupReason()
{
#ifdef ARDUINO_ARCH_ESP32
    Serial.print("Reset reason:");
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        Serial.println(" In case of deep sleep, reset was not caused by exit from deep sleep");
        break;
    case ESP_SLEEP_WAKEUP_ALL :
        break;
    case ESP_SLEEP_WAKEUP_EXT0 :
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1 :
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER :
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD :
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP :
        Serial.println("Wakeup caused by ULP program");
        break;
    default :
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
#endif
}


void getChipInfo()
{
#if defined(ARDUINO_ARCH_ESP32)

    Serial.println("-----------------------------------");
    printWakeupReason();

    if (psramFound()) {
        uint32_t psram = ESP.getPsramSize();
        devInfo.psramSize = psram / 1024.0 / 1024.0;
        Serial.printf("PSRAM is enable! PSRAM: %.2f MB\n", devInfo.psramSize);
        deviceOnline |= PSRAM_ONLINE;
    } else {
        Serial.println("PSRAM is disable!");
        devInfo.psramSize = 0;
    }

    devInfo.flashSize       = ESP.getFlashChipSize() / 1024.0 / 1024.0;
    devInfo.flashSpeed      = ESP.getFlashChipSpeed() / 1000 / 1000;
    devInfo.chipModel       = ESP.getChipModel();
    devInfo.chipModelRev    = ESP.getChipRevision();
    devInfo.chipFreq        = ESP.getCpuFreqMHz();

    Serial.printf("Flash: %u MB\n", devInfo.flashSize);
    Serial.printf("Flash speed: %u M\n", devInfo.flashSpeed);
    Serial.printf("Model: %s\n", devInfo.chipModel);
    Serial.printf("Chip Revision: %u\n", devInfo.chipModelRev);
    Serial.printf("Freq: %u MHz\n", devInfo.chipFreq);
    Serial.printf("SDK Ver: %s", ESP.getSdkVersion());
    Serial.printf("DATE: %s\n", __DATE__);
    Serial.printf("TIME: %s\n", __TIME__);

    uint8_t mac[6];
    char macStr[18] = { 0 };
    esp_efuse_mac_get_default(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("EFUSE MAC: %s\n", macStr);
    Serial.println("-----------------------------------");

#elif defined(ARDUINO_ARCH_STM32)
    uint32_t uid[3];
    uid[0] = HAL_GetUIDw0();
    uid[1] = HAL_GetUIDw1();
    uid[2] = HAL_GetUIDw2();
    Serial.print("STM UID: 0X");
    Serial.print( uid[0], HEX);
    Serial.print( uid[1], HEX);
    Serial.print( uid[2], HEX);
    Serial.println();
#endif
}


/**
 * @brief Setup Boards
 * 
 */
void setupBoards()
{
    Serial.begin(115200);
    while (!Serial);
    for (int i=0;i<10;i++) {
        Serial.println(".");
        delay(1000);
    }
    Serial.println("setupBoards");

    getChipInfo();

#if defined(ARDUINO_ARCH_ESP32)
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
#elif defined(ARDUINO_ARCH_STM32)
    SPI.setMISO(RADIO_MISO_PIN);
    SPI.setMOSI(RADIO_MOSI_PIN);
    SPI.setSCLK(RADIO_SCLK_PIN);
    SPI.begin();
#endif

#if defined(HAS_SDCARD)
#if defined(SD_SHARE_SPI_BUS)
    // Share spi bus with lora , set lora cs,rst to high
    pinMode(RADIO_CS_PIN, OUTPUT);
    pinMode(RADIO_RST_PIN, OUTPUT);
    digitalWrite(RADIO_CS_PIN, HIGH);
    digitalWrite(RADIO_RST_PIN, HIGH);
#else
    SDCardSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
#endif /*SD_SHARE_SPI_BUS*/
#endif /*HAS_SDCARD*/

#ifdef I2C1_SDA
    Wire1.begin(I2C1_SDA, I2C1_SCL);
    Serial.println("Scan Wire1...");
    scanDevices(&Wire1);
#endif

#ifdef HAS_GPS
#ifdef GPS_EN_PIN
    pinMode(GPS_EN_PIN, OUTPUT);
    digitalWrite(GPS_EN_PIN, HIGH);
#endif /*GPS_EN_PIN*/

#ifdef GPS_PPS_PIN
    pinMode(GPS_PPS_PIN, INPUT);
#endif

#if defined(ARDUINO_ARCH_ESP32)
    SerialGPS.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
#elif defined(ARDUINO_ARCH_STM32)
    SerialGPS.setRx(GPS_RX_PIN);
    SerialGPS.setTx(GPS_TX_PIN);
    SerialGPS.begin(GPS_BAUD_RATE);
#endif // ARDUINO_ARCH_
#endif // HAS_GPS

#if OLED_RST
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH); delay(20);
    digitalWrite(OLED_RST, LOW);  delay(20);
    digitalWrite(OLED_RST, HIGH); delay(20);
#endif

#ifdef BOARD_LED
    // T-Beam LED defaults to low level as turn on, so it needs to be forced to pull up
#if LED_ON == LOW
#if defined(ARDUINO_ARCH_ESP32)
    gpio_hold_dis((gpio_num_t)BOARD_LED);
#endif //ARDUINO_ARCH_ESP32
#endif
    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LED_ON);
#endif

#ifdef GPS_RST_PIN
    pinMode(GPS_RST_PIN, OUTPUT);
    digitalWrite(GPS_RST_PIN, HIGH);
#endif

#ifdef RADIO_LDO_EN
    // 1W and BPF LoRa LDO enable , Control SX1262 , LNA
    // 1W and BPF Radio version must set LDO_EN to HIGH to initialize the Radio
    pinMode(RADIO_LDO_EN, OUTPUT);
    digitalWrite(RADIO_LDO_EN, HIGH);
#endif

#ifdef RADIO_CTRL
    // 1W and BPF LoRa RX TX Control
    // CTRL controls the LNA, not the PA.
    // Only when RX DATA is on, set to 1 to turn on LNA.
    // When TX DATA is on, CTL is set to 0 and LNA is turned off.
    pinMode(RADIO_CTRL, OUTPUT);
    digitalWrite(RADIO_CTRL, HIGH);
#endif

#ifdef RADIO_DIO2_PIN
    pinMode(RADIO_DIO2_PIN, INPUT);
#endif

    beginPower();

    // Perform an I2C scan after power-on operation
#ifdef I2C_SDA
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("Scan Wire...");
    scanDevices(&Wire);
#endif

    beginSDCard();

    beginDisplay();

    scanWiFi();
    beginWiFi();

#ifdef FAN_CTRL
    pinMode(FAN_CTRL, OUTPUT);
#endif

#ifdef HAS_GPS

#if defined(T_BEAM_S3_SUPREME) || defined(T_BEAM_1W) || defined(T_BEAM_S3_BPF)
    // T-Beam v1.2 skips L76K
    find_gps = beginGPS();
#endif
    uint32_t baudrate[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 4800};
    if (!find_gps) {
        // Restore factory settings
        for ( int i = 0; i < sizeof(baudrate) / sizeof(baudrate[0]); ++i) {
            Serial.printf("Update baudrate : %u\n", baudrate[i]);
            SerialGPS.updateBaudRate(baudrate[i]);
            if (recoveryGPS()) {
                Serial.println("UBlox GNSS init succeeded, using UBlox GNSS Module\n");
                gps_model = "UBlox";
                find_gps = true;
                break;
            }
        }
    } else {
        gps_model = "L76K";
    }

    if (find_gps) {
        deviceOnline |= GPS_ONLINE;
    }

#ifdef T_BEAM_S3_SUPREME
    enable_slow_clock();
#endif

#endif
    Serial.println("init done . ");
}

/**
 * @brief Print Module Status
 * 
 * @param radio_online 
 */
void printResult(bool radio_online)
{
    Serial.printf("Radio        : %s\n", (radio_online) ? "+" : "-");

#if defined(CONFIG_IDF_TARGET_ESP32)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)
    Serial.printf("PSRAM        : %s\n", (psramFound()) ? "+" : "-");

#ifdef DISPLAY_MODEL
    Serial.printf("Display      : %s\n", (disp) ? "+" : "-");
#endif

#ifdef HAS_SDCARD
    Serial.printf("Sd Card      : %s\n", (SD.cardSize() != 0) ? "+" : "-");
#endif

#ifdef HAS_PMU
    Serial.printf("Power        : %s\n", ( PMU ) ? "+" : "-");
#endif

#ifdef HAS_GPS
    Serial.printf("GPS          : %s\n", ( find_gps ) ? "+" : "-");
#endif

#ifdef DISPLAY_MODEL
    if (disp) {

        disp->clearBuffer();
        disp->setFont(u8g2_font_NokiaLargeBold_tf );
        uint16_t str_w =  disp->getStrWidth(BOARD_VARIANT_NAME);
        disp->drawStr((disp->getWidth() - str_w) / 2, 16, BOARD_VARIANT_NAME);
        disp->drawHLine(5, 21, disp->getWidth() - 5);

        disp->drawStr( 0, 38, "Disp:");     disp->drawStr( 45, 38, ( disp) ? "+" : "-");

#ifdef HAS_SDCARD
        disp->drawStr( 0, 54, "SD :");      disp->drawStr( 45, 54, (SD.cardSize() != 0) ? "+" : "-");
#endif

        disp->drawStr( 62, 38, "Radio:");    disp->drawStr( 120, 38, ( radio_online ) ? "+" : "-");

#ifdef HAS_PMU
        disp->drawStr( 62, 54, "Power:");    disp->drawStr( 120, 54, ( PMU ) ? "+" : "-");
#endif

        disp->sendBuffer();
        delay(5000);
    }
#endif
#endif /*DISPLAY_MODEL*/
}


#ifdef BOARD_LED
static uint8_t ledState = LOW;
static const uint32_t debounceDelay = 500;
static uint32_t lastDebounceTime = 0;
#endif


#ifdef BOARD_LED
void flashLed()
{
    if ((millis() - lastDebounceTime) > debounceDelay) {
        ledState = !ledState;
        if (ledState) {
            digitalWrite(BOARD_LED, LED_ON);
        } else {
            digitalWrite(BOARD_LED, !LED_ON);
        }
        lastDebounceTime = millis();
    }
}
#endif


void scanDevices(TwoWire *w)
{
    uint8_t err, addr;
    int nDevices = 0;
    uint32_t start = 0;

    Serial.println("I2C Devices scanning");
    for (addr = 1; addr < 127; addr++) {
        start = millis();
        w->beginTransmission(addr); delay(2);
        err = w->endTransmission();
        if (err == 0) {
            nDevices++;
            switch (addr) {
            case 0x20:
                Serial.printf("\t0x%X found MCP23017 I/O-Expander\n", addr);
                break;
            case 0x34:
                Serial.printf("\t0x%X found AXP192/AXP2101 PMU!\n", addr);
                deviceOnline |= POWERMANAGE_ONLINE;
                break;
            case 0x38:
                Serial.printf("\t0x%X found AHT10/AHT20 Sensor!\n", addr);
                break;
            case 0x3C:
            case 0x3D: 
                Serial.printf("\tFound OLED display at address 0x%02X\n", addr);
                display_address = addr;
                deviceOnline |= DISPLAY_ONLINE;
                break;
            case 0x51:
                Serial.printf("\t0x%X found PCF8563 RTC!\n", addr);
                deviceOnline |= PCF8563_ONLINE;
                break;
            case 0x5A:
                Serial.printf("\t0x%X found CO2 Gas Sensor!\n", addr);
                break;
            case 0x76:
            case 0x77:
                Serial.printf("\t0x%X found BMx280/680... Sensor!\n", addr);
                deviceOnline |= BME280_ONLINE;
                break;
            default:
                Serial.printf("\tother I2C device found at address 0x%X\n", addr);
                break;
            }
        } else if (err == 4) {
            Serial.printf("Unknow error at address 0x%02X\n", addr);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");

    Serial.println("Scan devices done.\n");
}


#ifdef HAS_GPS

bool l76kProbe()
{
    bool result = false;
    uint32_t startTimeout ;
    SerialGPS.write("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02\r\n");
    delay(5);
    // Get version information
    startTimeout = millis() + 3000;
    Serial.print("Try to init L76K . Wait stop .");
    // SerialGPS.flush();
    while (SerialGPS.available()) {
        int c = SerialGPS.read();
        // Serial.write(c);
        // Serial.print(".");
        // Serial.flush();
        // SerialGPS.flush();
        if (millis() > startTimeout) {
            Serial.println("Wait L76K stop NMEA timeout!");
            return false;
        }
    };
    Serial.println();
    SerialGPS.flush();
    delay(200);

    SerialGPS.write("$PCAS06,0*1B\r\n");
    startTimeout = millis() + 500;
    String ver = "";
    while (!SerialGPS.available()) {
        if (millis() > startTimeout) {
            Serial.println("Get L76K timeout!");
            return false;
        }
    }
    SerialGPS.setTimeout(10);
    ver = SerialGPS.readStringUntil('\n');
    if (ver.startsWith("$GPTXT,01,01,02")) {
        Serial.println("L76K GNSS init succeeded, using L76K GNSS Module\n");
        result = true;
    }
    delay(500);

    // Initialize the L76K Chip, use GPS + GLONASS
    SerialGPS.write("$PCAS04,5*1C\r\n");
    delay(250);
    // only ask for RMC and GGA
    SerialGPS.write("$PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0*02\r\n");
    delay(250);
    // Switch to Vehicle Mode, since SoftRF enables Aviation < 2g
    SerialGPS.write("$PCAS11,3*1E\r\n");
    return result;
}

bool beginGPS()
{
    SerialGPS.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    bool result = false;
    for ( int i = 0; i < 3; ++i) {
        result = l76kProbe();
        if (result) {
            return result;
        }
    }
    return result;
}


static int getAck(uint8_t *buffer, uint16_t size, uint8_t requestedClass, uint8_t requestedID)
{
    uint16_t    ubxFrameCounter = 0;
    bool        ubxFrame = 0;
    uint32_t    startTime = millis();
    uint16_t    needRead;

    while (millis() - startTime < 800) {
        while (SerialGPS.available()) {
            int c = SerialGPS.read();
            switch (ubxFrameCounter) {
            case 0:
                if (c == 0xB5) { ubxFrameCounter++; }
                break;
            case 1:
                if (c == 0x62) { ubxFrameCounter++; }
                else           { ubxFrameCounter = 0; }
                break;
            case 2:
                if (c == requestedClass) { ubxFrameCounter++; }
                else                     { ubxFrameCounter = 0; }
                break;
            case 3:
                if (c == requestedID) { ubxFrameCounter++; }
                else                  { ubxFrameCounter = 0; }
                break;
            case 4:
                needRead = c;
                ubxFrameCounter++;
                break;
            case 5:
                needRead |=  (c << 8);
                ubxFrameCounter++;
                break;
            case 6:
                if (needRead >= size) {
                    ubxFrameCounter = 0;
                    break;
                }
                if (SerialGPS.readBytes(buffer, needRead) != needRead) {
                    ubxFrameCounter = 0;
                } else {
                    return needRead;
                }
                break;

            default: break;
            }
        }
    }
    return 0;
}

bool recoveryGPS()
{
    uint8_t buffer[256];
    uint8_t cfg_clear1[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x1C, 0xA2};
    uint8_t cfg_clear2[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1B, 0xA1};
    uint8_t cfg_clear3[] = {0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x1D, 0xB3};
    SerialGPS.write(cfg_clear1, sizeof(cfg_clear1));

    if (getAck(buffer, 256, 0x05, 0x01)) {
        Serial.println("Get ack successes!");
    }
    SerialGPS.write(cfg_clear2, sizeof(cfg_clear2));
    if (getAck(buffer, 256, 0x05, 0x01)) {
        Serial.println("Get ack successes!");
    }
    SerialGPS.write(cfg_clear3, sizeof(cfg_clear3));
    if (getAck(buffer, 256, 0x05, 0x01)) {
        Serial.println("Get ack successes!");
    }
    // UBX-CFG-RATE, Size 8, 'Navigation/measurement rate settings'
    uint8_t cfg_rate[] = {0xB5, 0x62, 0x06, 0x08, 0x00, 0x00, 0x0E, 0x30};
    SerialGPS.write(cfg_rate, sizeof(cfg_rate));
    if (getAck(buffer, 256, 0x06, 0x08)) {
        Serial.println("Get ack successes!");
    } else {
        return false;
    }
    return true;
}
#endif


#if defined(ARDUINO_ARCH_ESP32)

//NCP18XH103F03RB: https://item.szlcsc.com/14214.html
// #define NTC_PIN 14 // NTC connection pins
#define SERIES_RESISTOR 10000      // Series resistance value (10kΩ)
#define B_COEFFICIENT 3950         // B value, set according to the NTC specification
#define ROOM_TEMP 298.15           // 25°C absolute temperature (K)
#define ROOM_TEMP_RESISTANCE 10000 // Resistance of NTC at 25°C (10kΩ)

float getTempForNTC()
{
    static float temperature = 0.0f;
#ifdef NTC_PIN
    static uint32_t check_temperature = 0;
    if (millis() > check_temperature) {
        float voltage = analogReadMilliVolts(NTC_PIN) / 1000.0;
        float resistance = SERIES_RESISTOR * ((3.3 / voltage) - 1); // Calculate the resistance of NTC

        // Calculate temperature using the Steinhart-Hart equation
        temperature = (1.0 / (log(resistance / ROOM_TEMP_RESISTANCE) / B_COEFFICIENT + 1.0 / ROOM_TEMP)) - 273.15;

        Serial.printf("Temperature: %.1f °C\n", temperature);

        check_temperature  = millis() + 10000;
    }
#endif
    return temperature;
}

#ifdef ENABLE_BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setupBLE()
{
    uint8_t mac[6];
    char macStr[18] = { 0 };
    esp_efuse_mac_get_default(mac);
    sprintf(macStr, "%02X:%02X", mac[0], mac[1]);
    String dev = BOARD_VARIANT_NAME;
    dev.concat('-');
    dev.concat(macStr);
    Serial.printf("Starting BLE: %s", dev.c_str());

    BLEDevice::init(dev.c_str());
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setValue("Hello World");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
#endif

#define CALIBRATE_ONE(cali_clk) calibrate_one(cali_clk, #cali_clk)

static uint32_t calibrate_one(rtc_cal_sel_t cal_clk, const char *name)
{
    const uint32_t cal_count = 1000;
    const float factor = (1 << 19) * 1000.0f;
    uint32_t cali_val;
    for (int i = 0; i < 5; ++i) {
        cali_val = rtc_clk_cal(cal_clk, cal_count);
    }
    return cali_val;
}

static void enable_slow_clock()
{
    rtc_clk_32k_enable(true);
    CALIBRATE_ONE(RTC_CAL_RTC_MUX);
    uint32_t cal_32k = CALIBRATE_ONE(RTC_CAL_32K_XTAL);
    if (cal_32k == 0) {
        Serial.printf("32K XTAL OSC has not started up");
    } else {
        rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
        Serial.println("Switching RTC Source to 32.768Khz succeeded, using 32K XTAL");
        CALIBRATE_ONE(RTC_CAL_RTC_MUX);
        CALIBRATE_ONE(RTC_CAL_32K_XTAL);
    }
    CALIBRATE_ONE(RTC_CAL_RTC_MUX);
    CALIBRATE_ONE(RTC_CAL_32K_XTAL);
    if (rtc_clk_slow_freq_get() != RTC_SLOW_FREQ_32K_XTAL) {
        Serial.println("Warning: Failed to set rtc clk to 32.768Khz !!! "); return;
    }
    deviceOnline |= OSC32768_ONLINE;
}


void scanWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("WiFi Scan start");
    int n = WiFi.scanNetworks();  // returns the number of networks found
    Serial.println("WiFi Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.printf("%u networks found", n);
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.printf("%2d | ", i + 1);
            Serial.printf("%-32.32s | ", WiFi.SSID(i).c_str());
            Serial.printf("%4ld | ", WiFi.RSSI(i));
            Serial.printf("%2ld | ", WiFi.channel(i));
            switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:            Serial.print("open"); break;
            case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
            case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
            case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
            case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
            case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
            case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
            case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
            case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
            default:                        Serial.print("unknown");
            }
            Serial.println();
            delay(10);
        }
    }
    Serial.println();

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
}

#endif /*ARDUINO_ARCH_ESP32*/
