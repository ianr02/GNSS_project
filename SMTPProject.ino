#include "config.h"

unsigned long lastDraw = 0;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TinyGPSPlus     gps;
GpsFix          myFix;

unsigned long lastDraw    = 0;             // fuer das Warte-Bild
unsigned long lastText    = 0;             // Textbereich (langsam)
unsigned long lastCompass = 0;             // Kompass (schnell)
const unsigned long DRAW_INTERVAL    = 500; // ms, Warte-Bild
const unsigned long TEXT_INTERVAL    = 400; // ms, Text ~2-3x/s
const unsigned long COMPASS_INTERVAL = 50;  // ms, Kompass ~20x/s
bool  normalActive   = false;              // sind wir gerade im Normal-Layout?
float currentHeading = 0;                  // aktuelle Blickrichtung (Gyro)

int myAlarm = 0;   // wird in Stufe 5 vom Knopf gesetzt, vorerst 0
WiFiUDP         udp;
int             myAlarm = 0; // Starts at 0 (Safe peacetime)
PeerData peers[NUM_PEERS + 1];

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_PIN, INPUT_PULLUP);     // NEW
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
  displayBegin();
  gpsBegin();
  commsBegin();
  compassBegin();
  displayWaiting();
  Serial.println("Stage 3 + switch started");
}

void loop() {
  gpsUpdate();

  // Switch position = alarm state. LOW = flipped (to GND) = alarm.
  myAlarm = (digitalRead(SWITCH_PIN) == LOW) ? 1 : 0;   // NEW

  commsUpdate();

  // Gyro JEDE Runde abfragen -> gleichmaessiges, genaues Integral
  currentHeading = getHeading();

  if (myFix.valid) {
    // Moduswechsel: Grundlayout genau einmal malen
    if (!normalActive) {
      normalActive = true;
      drawNormalLayout();
      lastText = 0;          // erzwingt sofortiges erstes Zeichnen
      lastCompass = 0;
    }
    // Kompass oft aktualisieren (fluessiger Pfeil)
    if (millis() - lastCompass >= COMPASS_INTERVAL) {
      lastCompass = millis();
      drawCompass(currentHeading);
    }
    // Text selten aktualisieren (aendert sich langsam)
    if (millis() - lastText >= TEXT_INTERVAL) {
      lastText = millis();
      drawNormalText(myFix);
    }
  } else {
    normalActive = false;   // beim naechsten Fix wird Layout neu gemalt
    if (millis() - lastDraw >= DRAW_INTERVAL) {
      lastDraw = millis();
      displayWaiting();
    }
  }
}