//=======================================================================================
const int SAMPLE_COUNT = 50;      // Anzahl der zu messenden Signalflanken
const int SAMPLE_DURATION = 5000; // max. Messdauer 5s
volatile unsigned long pulseTimes[SAMPLE_COUNT];
volatile int pulseIndex = 0;
volatile unsigned long lastMicros = 0;
volatile unsigned long currentMicros = 0;
volatile unsigned long duration = 0;
volatile unsigned long startWait = 0;

/**
 * @brief ISR für detectBaudrate
 * 
 */
void IRAM_ATTR handleRxInterrupt() {
  currentMicros = micros();
  duration = currentMicros - lastMicros;
  
  if (pulseIndex < SAMPLE_COUNT && duration > 2) {
    pulseIndex = pulseIndex+1;
    pulseTimes[pulseIndex] = duration;
  }
  lastMicros = currentMicros;
}

/**
 * @brief detect Baudrate durch Messung der Zeit zwischen RX-Flanken
 * 
 * @return long = detected Baudrate
 */
long detectBaudrate() {
  pulseIndex = 0;
  lastMicros = micros();

  // Messung: warten, bis genügend Flanken gemessen wurden oder Timeout 5s
  attachInterrupt(GPS_RX_PIN, handleRxInterrupt, CHANGE);
  startWait = millis();
  while (pulseIndex < SAMPLE_COUNT && (millis() - startWait < SAMPLE_DURATION)) { delay(10); }
  detachInterrupt(GPS_RX_PIN);

  // Auswertung
  if (pulseIndex < 5) return -1; // Zu wenig Daten empfangen
  unsigned long minDuration = 1000000;
  for (int i = 1; i < pulseIndex; i++) {  // Suche nach dem kürzesten Puls (entspricht 1 Bit)
    if (pulseTimes[i] < minDuration && pulseTimes[i] > 2) { // Rauschfilter > 2µs, ist zwar schon in der Erfassung
      minDuration = pulseTimes[i];
    }
  }
  USBSerial.printf("[GPS] gemessene Flanken %u\n", pulseIndex);
  long calculatedBaud = 1000000 / minDuration;

  // Mapping auf Standard-Baudraten
  long standardBauds[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
  long bestMatch = 0;
  long minDiff = 1000000;
  for (long b : standardBauds) {
    long diff = abs(calculatedBaud - b);
    if (diff < minDiff) {
      minDiff = diff;
      bestMatch = b;
    }
  }
  return bestMatch;
}