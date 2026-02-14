#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "configuration.h"

SPIClass SD_SPIHandle(HSPI);

/**
 * @brief List a Directory
 * 
 * @param fs 
 * @param dirname 
 * @param levels 
 */
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("\n> dir %s\n", dirname);  // dir | ls

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s\n", file.name());
            if (levels) {
                listDir(fs, file.path(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

/**
 * @brief Create a Dir object
 * 
 * @param fs 
 * @param path 
 */
void createDir(fs::FS &fs, const char *path)
{
    Serial.printf("\n> md %s\n", path);  // md | mkdir
    if (fs.mkdir(path)) { Serial.println("Dir created");  }
    else                { Serial.println("mkdir failed"); }
}

/**
 * @brief Remove a Dir
 * 
 * @param fs 
 * @param path 
 */
void removeDir(fs::FS &fs, const char *path)
{
    Serial.printf("\n> rd %s\n", path);  // rs | rmdir
    if (fs.rmdir(path)) { Serial.println("Dir removed");  }
    else                { Serial.println("rmdir failed"); }
}

/**
 * @brief Read a File
 * 
 * @param fs 
 * @param path 
 */
void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("\nReading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) { Serial.write(file.read()); }
    file.close();
}

/**
 * @brief Write a File
 * 
 * @param fs 
 * @param path 
 * @param message 
 */
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("\nWriting file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) { Serial.println("File written"); }
    else                     { Serial.println("Write failed"); }
    file.close();
}

/**
 * @brief Append a File
 * 
 * @param fs 
 * @param path 
 * @param message 
 */
void appendFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("\nAppending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) { Serial.println("Message appended"); }
    else                     { Serial.println("Append failed");    }
    file.close();
}

/**
 * @brief Rename a File
 * 
 * @param fs 
 * @param path1 
 * @param path2 
 */
void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    Serial.printf("\nRenaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) { Serial.println("File renamed");  }
    else                         { Serial.println("Rename failed"); }
}

/**
 * @brief Delete a File
 * 
 * @param fs 
 * @param path 
 */
void deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("\n> del %s\n", path);
    if (fs.remove(path)) { Serial.println("File deleted");  }
    else                 { Serial.println("Delete failed"); }
}

/**
 * @brief Test File IO
 * 
 * @param fs 
 * @param path 
 */
void testFileIO(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) { toRead = 512; }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

#if defined(HAS_PMU)&&defined(HAS_SDCARD)
#include <XPowersLib.h>
XPowersLibInterface *PMU = NULL;
void beginPower()
{
#ifdef I2C_SDA
    Wire.begin(I2C_SDA, I2C_SCL);
#endif

#ifdef I2C1_SDA
    Wire1.begin(I2C1_SDA, I2C1_SCL);
#endif

    if (!PMU) {
        PMU = new XPowersAXP2101(PMU_WIRE_PORT);
        if (!PMU->init()) {
            Serial.println("Warning: Failed to find AXP2101 power management");
            delete PMU;
            PMU = NULL;
        } else {
            Serial.println("AXP2101 PMU init succeeded, using AXP2101 PMU");
        }
    }
    if (PMU->getChipModel() == XPOWERS_AXP2101) {
#if defined(T_BEAM_S3_SUPREME)
        PMU->setPowerChannelVoltage(XPOWERS_BLDO1, 3300);
        PMU->enablePowerOutput(XPOWERS_BLDO1);
#elif defined(T_BEAM_S3_BPF)
        PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO2);
#endif
    }
}
#else
void beginPower()
{
    Serial.println("This board does not have a PMU.");
}
#endif



//=======================================================================================
/**
 * @brief 
 * 
 */
void setup()
{
    Serial.begin(115200);
    delay(10000);
    Serial.println("SD-Card Test");
    beginPower();

    // SPI initialization, reserved for Radio use
    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

    // Initialize the second SPI for use with the SD card
    SD_SPIHandle.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);

    if (!SD.begin(SDCARD_CS, SD_SPIHandle)) {
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("\nSD Card Type: ");
    if (cardType == CARD_MMC)       { Serial.println("MMC");     }
    else if (cardType == CARD_SD)   { Serial.println("SDSC");    }
    else if (cardType == CARD_SDHC) { Serial.println("SDHC");    }
    else                            { Serial.println("UNKNOWN"); }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %llu MB\n", cardSize);

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    Serial.printf("\nTotal space: %llu MB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %llu MB\n", SD.usedBytes() / (1024 * 1024));
}

//=======================================================================================
void loop()
{
}
