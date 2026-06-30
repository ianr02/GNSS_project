#include "config.h"

unsigned long lastDraw = 0;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TinyGPSPlus     gps;
GpsFix          myFix;
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
  
  if (millis() - lastDraw >= DRAW_INTERVAL) {
    lastDraw = millis();
    if (myFix.valid) displayPosition(myFix);
    else             displayWaiting();
  }
}