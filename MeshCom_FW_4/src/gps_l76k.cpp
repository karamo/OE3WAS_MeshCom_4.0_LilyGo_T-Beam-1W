#include <Arduino.h>
#include <SoftwareSerial.h>

#include <configuration.h>

#include <clock.h>

#include <loop_functions.h>
#include <loop_functions_extern.h>

#if defined(GPS_L76K)

#include <gps_l76k.h>

#if defined(BOARD_T_DECK) || defined(BOARD_T_DECK_PLUS) || defined(BOARD_TRACKER) || defined(BOARD_TBEAM_1W)
    HardwareSerial gpsSerial(1);
#else
  #include "SoftwareSerial.h"
  extern SoftwareSerial gpsSerial;
#endif

#include <TinyGPSPlus.h>

// TinyGPS
extern TinyGPSPlus tinyGPSPlus;

bool l76kProbe()
{
    bool result = false;
    uint32_t startTimeout ;
    gpsSerial.write("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02\r\n");
    delay(500);
    // Get version information
    startTimeout = millis() + 4000;
    Serial.printf("\n[GPSL]...Try to init GNSS @ %lu Wait stop ...\n", meshcom_settings.node_gpsbaud);

    while (gpsSerial.available() > 0)
    {
        char c = gpsSerial.read();
        if(bGPSDEBUG) Serial.print(c);

        if (millis() > startTimeout)
        {
            Serial.println("[GPSL]...Wait GNSS stop NMEA timeout!");
            return false;
        }
    };
    Serial.println();
    
    gpsSerial.flush();
    delay(200);

    gpsSerial.write("$PCAS06,0*1B\r\n");
    startTimeout = millis() + 500;
    String ver = "";

    while (gpsSerial.available() <= 0)
    {
        if (millis() > startTimeout)
        {
            Serial.println("[GPSL]...Get GNSS-Probe timeout!");
            return false;
        }
    }
    gpsSerial.setTimeout(40);
    ver = gpsSerial.readStringUntil('\n');
    if(bGPSDEBUG) Serial.printf("\nProbe: %s\n", ver.c_str());
    if (ver.startsWith("$GPTXT,01,01,02") || ver.startsWith("$GNTXT,01,01,01,PCAS"))
    {
        Serial.print("\n[GPSL]...GNSS init succeeded, using GNSS Module: ");

        if(ver.indexOf("PCAS inv format") > 0) {
            Serial.println("UBLOX GNSS");
        } else { 
          Serial.println("L76K");
          result = true;
        }
    }
    delay(500);

    // Initialize the GNSS Chip, use GPS + GLONASS
    gpsSerial.write("$PCAS04,5*1C\r\n");
    delay(250);
    // only ask for RMC and GGA
    gpsSerial.write("$PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0*02\r\n");
    delay(250);
    // Switch to Vehicle Mode, since SoftRF enables Aviation < 2g
    gpsSerial.write("$PCAS11,3*1E\r\n");

    // hier noch Baudrate auf 38400 umschalten
    // ...

    return result;
}

static TaskHandle_t gpsInitTaskHandle = NULL;

void gpsInitTask(void *parameter) {

    bool result = false;

    if(meshcom_settings.node_gpsbaud > 0 && meshcom_settings.node_gpsbaud < 150000)
    {
        if(bGPSDEBUG)
            Serial.printf("[GPS ]...check %lubaud\n", meshcom_settings.node_gpsbaud);

        gpsSerial.begin(meshcom_settings.node_gpsbaud);
        for ( int i = 0; i < 3; ++i)
        {
            result = l76kProbe();
            if (result)
            {
                break;
            }
        }
    }

    if (!result) {
        if(bGPSDEBUG)
            Serial.println("[GPS ]...check 38400baud");

        gpsSerial.begin(38400);
        for ( int i = 0; i < 3; ++i)
        {
            result = l76kProbe();
            if (result)
            {
                meshcom_settings.node_gpsbaud = 38400;
                save_settings();
                break;
            }
        }
    }

    if (!result) {
        if(bGPSDEBUG)
            Serial.println("[GPS ]...check 115200baud");

        gpsSerial.begin(115200);
        for ( int i = 0; i < 3; ++i)
        {
            result = l76kProbe();
            if (result)
            {
                meshcom_settings.node_gpsbaud = 115200;
                save_settings();
                break;
            }
        }
    }
    
    if(!result) {
        if(bGPSDEBUG)
            Serial.println("[GPS ]...check 9600baud");

        gpsSerial.begin(9600);
        for ( int i = 0; i < 3; ++i)
        {
            result = l76kProbe();
            if (result)
            {
                meshcom_settings.node_gpsbaud = 9600;
                save_settings();
                break;
            }
        }
    }

    gpsInitTaskHandle = NULL;
    vTaskDelete(NULL);
}

bool beginGPS()
{
    if (gpsInitTaskHandle != NULL) return true;
    
    xTaskCreate(gpsInitTask, "GPSInit", 4096, NULL, 1, &gpsInitTaskHandle);
    return true;
}

void stopL76KGPS()
{
    gpsSerial.end();

    posinfo_fix = false;
    posinfo_satcount = 0;
    posinfo_hdop = 0;
}

bool newData = false;

unsigned int loopL76KGPS()
{
  if(bGPSDEBUG) Serial.println("-----------check L76K GPS-----------");

  newData = false;
  unsigned long start = millis();
  unsigned long GPStimeout = millis();
  bool BurstStart = false;
  char c = '0';
  char ziel[200];
  int j = 0;
  ziel[j] = '\0';

  gpsSerial.flush(); // Empfangs-Buffer (=64 Zeichen) löschen

  while ((millis() - start) < 10000)  // weil nur 9600 verbunden ist
  {
    while (gpsSerial.available() > 0)
    {
      BurstStart = true;
      GPStimeout = millis();
      c = gpsSerial.read();

      if(((c>=0x20) && (c<0x7f)) || (c==0x0A) || (c==0x0D))
      {
        ziel[j] = c; j++; ziel[j] = '\0'; // Zeichen akkumulieren
        if (tinyGPSPlus.encode(c)) { newData = true; }
      }
    }
    if (BurstStart && ((GPStimeout+100) < millis()) ) break;
  }
  if(bGPSDEBUG) { Serial.printf("Rec:%s\n", ziel); }

  newData = newData; // um compiler ruhig zu stellen
  return displayInfo();
}
 
 
unsigned int displayInfo()
 {
    bool gps_time_update = tinyGPSPlus.time.isUpdated();
    bool gps_loc_update  = tinyGPSPlus.location.isUpdated();
    posinfo_satcount = tinyGPSPlus.satellites.value();
    posinfo_hdop = tinyGPSPlus.hdop.value();

    if(bGPSDEBUG)
    {
        Serial.printf("\n[GPS ]...sat_count:%i hdop:%i time_update:%i location_update:%i\n", posinfo_satcount, posinfo_hdop, gps_time_update, gps_loc_update);

        Serial.print("[GPS ]...Time <UTC>: ");
        if (tinyGPSPlus.time.hour() < 10) Serial.print(F("0"));
        Serial.print(tinyGPSPlus.time.hour());
        Serial.print(F(":"));
        if (tinyGPSPlus.time.minute() < 10) Serial.print(F("0"));
        Serial.print(tinyGPSPlus.time.minute());
        Serial.print(F(":"));
        if (tinyGPSPlus.time.second() < 10) Serial.print(F("0"));
        Serial.print(tinyGPSPlus.time.second());

        Serial.print(F(" / Date: "));
        Serial.print(tinyGPSPlus.date.year());
        Serial.print(F("."));
        if (tinyGPSPlus.date.month() < 10) Serial.print(F("0"));
        Serial.print(tinyGPSPlus.date.month());
        Serial.print(F("."));
        if (tinyGPSPlus.date.day() < 10) Serial.print(F("0"));
        Serial.print(tinyGPSPlus.date.day());
    }

    if (tinyGPSPlus.location.isValid())
    {
        if(bGPSDEBUG) 
        {
            Serial.printf(" / Pos: ");
            Serial.print(tinyGPSPlus.location.lat(), 6);
            Serial.print((tinyGPSPlus.location.rawLat().negative?"S":"N"));
            Serial.print(F(","));
            Serial.print(tinyGPSPlus.location.lng(), 6);
            Serial.print((tinyGPSPlus.location.rawLng().negative?"W":"E"));
        }

        if(bGPSDEBUG)
            Serial.print(F(" "));

        if (tinyGPSPlus.time.isValid())
        {
            if(tinyGPSPlus.date.year() > 2023)
            {
                MyClock.setCurrentTime(meshcom_settings.node_utcoff, tinyGPSPlus.date.year(), tinyGPSPlus.date.month(), tinyGPSPlus.date.day(), tinyGPSPlus.time.hour(), tinyGPSPlus.time.minute(), tinyGPSPlus.time.second());
                snprintf(cTimeSource, sizeof(cTimeSource), (char*)"GPS");
            }
                
            meshcom_settings.node_date_year = MyClock.Year();
            meshcom_settings.node_date_month = MyClock.Month();
            meshcom_settings.node_date_day = MyClock.Day();
        
            meshcom_settings.node_date_hour = MyClock.Hour();
            meshcom_settings.node_date_minute = MyClock.Minute();
            meshcom_settings.node_date_second = MyClock.Second();
    
            // valid GPS data
            if(tinyGPSPlus.location.isValid() && tinyGPSPlus.hdop.isValid() && tinyGPSPlus.hdop.value() < 5000)
            {
                double dlat, dlon;

                dlat = cround4abs(tinyGPSPlus.location.lat());
                dlon = cround4abs(tinyGPSPlus.location.lng());

                double slat = 100.0;
                slat = dlat*slat;
                double slon = 100.0;
                slon=dlon*slon;
                
                double slatr=60.0;
                double slonr=60.0;
                
                slat = (int)dlat;
                slatr = (dlat - slat) * slatr;
                slat = (slat * 100.) + slatr;
                
                slon = (int)dlon;
                slonr = (dlon - slon) * slonr;
                slon = (slon * 100.) + slonr;
            
                meshcom_settings.node_lat = cround4(dlat);
                meshcom_settings.node_lon = cround4(dlon);

                if(tinyGPSPlus.location.rawLng().negative)
                    meshcom_settings.node_lon_c='W';
                else
                    meshcom_settings.node_lon_c='E';

                if(tinyGPSPlus.location.rawLat().negative)
                    meshcom_settings.node_lat_c='S';
                else
                    meshcom_settings.node_lat_c='N';

                meshcom_settings.node_alt = (int)tinyGPSPlus.altitude.meters();
                if(meshcom_settings.node_alt < 0)
                    meshcom_settings.node_alt = 0;

                posinfo_satcount = tinyGPSPlus.satellites.value();
                posinfo_hdop = tinyGPSPlus.hdop.value();
                posinfo_fix = true;

                posinfo_age = tinyGPSPlus.satellites.age();

                if(bGPSDEBUG)
                {
                    Serial.println(F("...VALID"));
                }

                return setSMartBeaconing(dlat, dlon);

            }
            else
            {
                posinfo_fix = false;
                posinfo_direction = 0;
                posinfo_distance = 0;
                posinfo_age = 0;
        
                if(bGPSDEBUG)
                    Serial.println(F(" INVALID"));
            }
        }
        else
        {
            posinfo_fix = false;
            posinfo_direction = 0;
            posinfo_distance = 0;

            if(bGPSDEBUG)
                Serial.println(F(" INVALID"));
        }

        if(bGPSDEBUG)
            Serial.println();
    }
    else
    {
        posinfo_fix = false;

        if(bGPSDEBUG)
            Serial.println(F(" INVALID"));   
    }

    return POSINFO_INTERVAL;
}
 
#endif