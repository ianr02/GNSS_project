#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <TinyGPSPlus.h>
#include <WiFi.h>                
#include <esp_now.h>             
#include <esp_arduino_version.h>  
#include "config.h"

// Globale Objekte (in den anderen Tabs sichtbar)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TinyGPSPlus     gps;
GpsFix          myFix;

unsigned long lastDraw = 0;
const unsigned long DRAW_INTERVAL = 500;   // ms, nicht-blockierend

int myAlarm = 0;   // wird in Stufe 5 vom Knopf gesetzt, vorerst 0

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_PIN, INPUT_PULLUP);     // NEU
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
  displayBegin();
  gpsBegin();
  commsBegin();
  displayWaiting();
  Serial.println("Stufe 3 + switch started");
}

void loop() {
  gpsUpdate();

  // Schalterstellung = Alarmzustand. LOW = umgelegt (gegen GND) = Alarm.
  myAlarm = (digitalRead(SWITCH_PIN) == LOW) ? 1 : 0;   // NEU

  commsUpdate();
  if (millis() - lastDraw >= DRAW_INTERVAL) {
    lastDraw = millis();
    if (myFix.valid) displayPosition(myFix);
    else             displayWaiting();
  }
}