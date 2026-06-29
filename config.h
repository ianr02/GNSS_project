#pragma once
#include <Adafruit_ST7789.h>

// ---- Alarm-Kippschalter ----
#define SWITCH_PIN 1   // ein freier GPIO (siehe Hinweis unten)

// ---- Geräte-Identität: pro Gerät ändern (1, 2, 3) ----
#define DEVICE_ID 2

// ---- ESP-NOW ----
#define TX_INTERVAL   1000   // ms: 1x pro Sekunde senden
#define PEER_TIMEOUT  5000   // ms: danach gilt ein Gerät als offline

// ---- TFT ----
#define TFT_CS   7
#define TFT_DC   39
#define TFT_RST  40
#define TFT_BL   45
#define SPI_SCK  36
#define SPI_MISO 37
#define SPI_MOSI 35

// ---- GNSS UART ----
#define GPS_RX   17        // LC29H TX (Gelb) -> ESP RX
#define GPS_TX   18        // ESP TX -> LC29H RX (Blau)
#define GPS_BAUD 115200

// ---- Farben ----
#define COL_BG    ST77XX_BLACK
#define COL_OK    ST77XX_GREEN     // grüner Rand = Normalbetrieb
#define COL_WARN  ST77XX_YELLOW
#define COL_TEXT  ST77XX_WHITE

// Gemeinsamer Positionszustand (wird später ans Netzwerk weitergegeben)
struct GpsFix {
  bool     valid = false;
  double   lat = 0, lng = 0;   // Dezimalgrad, vorzeichenbehaftet
  double   speedKmh = 0;
  uint32_t sats = 0;
};