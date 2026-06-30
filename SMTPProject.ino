#include "config.h"
#include <Wire.h>

// ---- Global objects (defined here, declared 'extern' in config.h) ----
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
TinyGPSPlus     gps;
GpsFix          myFix;
WiFiUDP         udp;                  // owned by comms - keep defined only once
PeerData        peers[NUM_PEERS + 1]; // owned by comms - keep defined only once
int             myAlarm = 0;          // 0 = normal, 1 = THIS device raised the alarm

// ---- Display timing ----
unsigned long lastDraw    = 0;        // waiting screen
unsigned long lastText    = 0;        // slow text area
unsigned long lastCompass = 0;        // fast compass / arrow
// DRAW_INTERVAL comes from config.h
const unsigned long TEXT_INTERVAL    = 400;  // ms
const unsigned long COMPASS_INTERVAL = 80;   // ms
float currentHeading = 0;

// ---- Display state machine ----
enum DisplayMode { MODE_WAIT, MODE_NORMAL, MODE_ALARM, MODE_SENDING };
DisplayMode currentMode = MODE_WAIT;
DisplayMode lastMode    = (DisplayMode)-1;   // force first draw

void setup() {
  Serial.begin(115200);

  // Alarm toggle switch: pin 12 supplies HIGH, pin 10 senses it.
  pinMode(SWITCH_PWR_PIN, OUTPUT);
  digitalWrite(SWITCH_PWR_PIN, HIGH);
  pinMode(SWITCH_PIN, INPUT_PULLDOWN);     // LOW when open, HIGH when closed

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
  Wire.begin(I2C_SDA, I2C_SCL);
  displayBegin();
  gpsBegin();
  commsBegin();
  compassBegin();
  displayWaiting();
  Serial.println("Started");
}

void loop() {
  gpsUpdate();

  // Switch closed (pin 10 reads HIGH) = this device raises the alarm
  myAlarm = (digitalRead(SWITCH_PIN) == HIGH) ? 1 : 0;

  // --- TEMP DEBUG: prints once whenever the switch changes ---
  static int lastDbg = -1;
  if (myAlarm != lastDbg) { Serial.print("Switch -> myAlarm = "); Serial.println(myAlarm); lastDbg = myAlarm; }

  commsUpdate();

  // Poll the gyro every loop -> smooth, accurate heading integral
  currentHeading = getHeading();

  // ---- Decide which screen to show ----
  int alarmPeer = findAlarmPeer();
  if      (myAlarm == 1)                 currentMode = MODE_SENDING;  // WE raised it -> always show
  else if (alarmPeer > 0 && myFix.valid) currentMode = MODE_ALARM;    // someone else in danger
  else if (!myFix.valid)                 currentMode = MODE_WAIT;
  else                                   currentMode = MODE_NORMAL;
  // ---- On mode change: draw the static layout once ----
  if (currentMode != lastMode) {
    switch (currentMode) {
      case MODE_NORMAL:  drawNormalLayout();         break;
      case MODE_ALARM:   drawAlarmLayout(alarmPeer); break;
      case MODE_SENDING: drawSendingLayout();        break;
      case MODE_WAIT:    /* displayWaiting draws itself */ break;
    }
    lastMode = currentMode;
    lastDraw = lastText = lastCompass = 0;   // force immediate refresh
  }

  // ---- Per-frame updates ----
  switch (currentMode) {
    case MODE_WAIT:
      if (millis() - lastDraw >= DRAW_INTERVAL) { lastDraw = millis(); displayWaiting(); }
      break;

    case MODE_NORMAL:
      if (millis() - lastCompass >= COMPASS_INTERVAL) { lastCompass = millis(); drawCompass(currentHeading); }
      if (millis() - lastText    >= TEXT_INTERVAL)    { lastText    = millis(); drawNormalText(myFix); }
      break;

    case MODE_ALARM:
      if (millis() - lastCompass >= COMPASS_INTERVAL) {
        lastCompass = millis();
        drawAlarmArrow(alarmPeer, currentHeading);
        drawAlarmBorder();                 // blinking red border
      }
      if (millis() - lastText >= TEXT_INTERVAL) { lastText = millis(); drawAlarmInfo(alarmPeer); }
      break;

       case MODE_SENDING:
      if (millis() - lastCompass >= COMPASS_INTERVAL) { lastCompass = millis(); drawAlarmBorder(); }
      if (millis() - lastText    >= TEXT_INTERVAL)    { lastText    = millis(); drawSendingInfo(); }
      break;
  }
}