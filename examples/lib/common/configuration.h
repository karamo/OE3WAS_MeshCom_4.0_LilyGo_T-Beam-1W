#pragma once

#define UNUSED_PIN                   (0)

///< ----- T3 V1.6.1 ----- https://lilygo.cc/products/lora3
#if defined(T3_V1_6_SX1276) || defined(T3_V1_6_SX1278)

#if   defined(T3_V1_6_SX1276)
#ifndef USING_SX1276
#define USING_SX1276
#endif
#elif defined(T3_V1_6_SX1278)
#ifndef USING_SX1278
#define USING_SX1278
#endif
#endif // T3_V1_6_SX1276 || T3_V1_6_SX1278

#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN

#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
// SX1276/78
#define RADIO_DIO2_PIN              32
// SX1262
#define RADIO_BUSY_PIN              32

#define BOARD_LED                   25
#define LED_ON                      HIGH
#define LED_OFF                     LOW

#define HAS_SDCARD
#define SDCARD_MOSI                 15
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13

#define HAS_DISPLAY
#define DISPLAY_MODEL_SSD_LIB       SSD1306Wire
#define DISPLAY_MODEL               U8G2_SSD1306_128X64_NONAME_F_HW_I2C

#define BOARD_VARIANT_NAME          "T3 V1.6"

#define ADC_PIN                     35
#define BAT_ADC_PULLUP_RES          (100000.0)
#define BAT_ADC_PULLDOWN_RES        (100000.0)
#define BAT_MAX_VOLTAGE             (4.2)
#define BAT_VOL_COMPENSATION        (0.0)

///< ----- T-Beam 1W ----- LoRa 1W
#elif defined(T_BEAM_1W)

#ifndef USING_SX1262
#define USING_SX1262
#endif

#define I2C_SDA                     (8)
#define I2C_SCL                     (9)

#define GPS_RX_PIN                  (5)
#define GPS_TX_PIN                  (6)
#define GPS_PPS_PIN                 (7)
#define GPS_EN_PIN                  (16)

#define BUTTON_PIN                  (0)          /*BUTTON 1 = GPIO0 (BOOT)*/
#define BUTTON2_PIN                 (17)         /*BUTTON 2 = GPIO17*/

#define BUTTON_PIN_MASK             GPIO_SEL_0
#define BUTTON_COUNT                (2)
#define BUTTON_ARRAY                {BUTTON_PIN,BUTTON2_PIN}

#define SPI_MOSI                    (11)
#define SPI_SCK                     (13)
#define SPI_MISO                    (12)
#define SPI_CS                      (10)

#define RADIO_SCLK_PIN              (SPI_SCK)
#define RADIO_MISO_PIN              (SPI_MISO)
#define RADIO_MOSI_PIN              (SPI_MOSI)

#define RADIO_CS_PIN                (15)
#define RADIO_RST_PIN               (3)
#define RADIO_LDO_EN                (40)
#define RADIO_CTRL                  (21)
#define RADIO_DIO1_PIN              (1)
#define RADIO_BUSY_PIN              (38)

#define BOARD_LED                   18
#define LED_ON                      HIGH
#define LED_OFF                     LOW

#define NTC_PIN                     (14)
#define FAN_CTRL                    (41)
#define ADC_PIN                     (4)

#define BAT_ADC_PULLUP_RES          (300000.0)
#define BAT_ADC_PULLDOWN_RES        (150000.0)
#define BAT_MAX_VOLTAGE             (7.4)
#define BAT_VOL_COMPENSATION        (0.25)
#define GPS_SLEEP_HOLD_ON_LOW

#define GPS_BAUD_RATE               9600

#define HAS_SDCARD
#define SDCARD_MOSI                 SPI_MOSI
#define SDCARD_MISO                 SPI_MISO
#define SDCARD_SCLK                 SPI_SCK
#define SDCARD_CS                   SPI_CS
#define SD_SHARE_SPI_BUS           // SD-CARD AND RADIO SHARE SPI BUS

#define HAS_GPS

#define HAS_DISPLAY
#define DISPLAY_MODEL               U8G2_SH1106_128X64_NONAME_F_HW_I2C
#define DISPLAY_MODEL_SSD_LIB       SH1106Wire

#define __HAS_SPI1__

#define BOARD_VARIANT_NAME          "LoRa 1W"

#endif

#if  defined(USING_SX1262)
#define RADIO_TYPE_STR  "SX1262"
#elif defined(USING_SX1276)
#define RADIO_TYPE_STR  "SX1276"
#elif defined(USING_SX1278)
#define RADIO_TYPE_STR  "SX1278"
#endif





