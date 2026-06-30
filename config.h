#pragma once
#include <Adafruit_ST7789.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_arduino_version.h>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <TinyGPSPlus.h> 

// ---- Network Information ----
#define PORT 5005
#define BROADCAST "255.255.255.255"
#define SSID ""
#define PASSWORD ""

// ---- Alarm Toggle Switch ----
#define SWITCH_PIN 1   // A free GPIO pin
#define DRAW_INTERVAL 500 

// ---- Device Identity: Change per device (1, 2, 3) ----
#define DEVICE_ID 2
#define NUM_PEERS 3

// ---- ESP-NOW ----
#define TX_INTERVAL   1000   // ms: Send once per second
#define PEER_TIMEOUT  5000   // ms: Device is considered offline after this time

// ---- TFT ----
#define TFT_CS   7
#define TFT_DC   39
#define TFT_RST  40
#define TFT_BL   45
#define SPI_SCK  36
#define SPI_MISO 37
#define SPI_MOSI 35

// ---- GNSS UART ----
#define GPS_RX   17        // LC29H TX (Yellow) -> ESP RX
#define GPS_TX   18        // ESP TX -> LC29H RX (Blue)
#define GPS_BAUD 115200

// ---- Colors ----
#define COL_BG    ST77XX_BLACK
#define COL_OK    ST77XX_GREEN     // Green border = normal operation
#define COL_WARN  ST77XX_YELLOW
#define COL_TEXT  ST77XX_WHITE

// Shared position state (passed along to the network later)
struct GpsFix {
  bool     valid = false;
  double   lat = 0, lng = 0;   // Signed decimal degrees
  double   speedKmh = 0;
  uint32_t sats = 0;
};

struct PeerData {
  bool active = false;
  double lat = 0, lng = 0;
  int alarm = 0;
  unsigned long lastSeen = 0;
};

// Share these objects across all your tabs safely via 'extern'
extern Adafruit_ST7789 tft;
extern TinyGPSPlus     gps;
extern GpsFix          myFix;
extern WiFiUDP         udp;
extern int             myAlarm;
extern PeerData        peers[];
