// ===================================================================
//  alarm_tab - Alarm screen
//  RECEIVER: another device raised the alarm -> big arrow toward the
//            person in danger + distance, red blinking border.
//  SENDER:   THIS device raised the alarm -> red border + "Aid is on its way".
// ===================================================================
#include "config.h"

// Find the first active peer that has raised an alarm (0 = none).
int findAlarmPeer() {
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && peers[i].alarm == 1) return i;
  return 0;
}

// Red blinking border (toggles ~every 400 ms). Cheap; call it often.
void drawAlarmBorder() {
  bool on = (millis() / 400) % 2;
  uint16_t c = on ? ST77XX_RED : COL_BG;
  for (int i = 0; i < 4; i++)
    tft.drawRect(i, i, tft.width() - 2 * i, tft.height() - 2 * i, c);
}

// ---------- RECEIVER (someone else is in danger) ----------

void drawAlarmLayout(int who) {
  tft.fillScreen(COL_BG);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.setCursor(8, 8);
  tft.print("ALARM");
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(150, 8);
  tft.print("D"); tft.print(who);
}

void drawAlarmArrow(int who, float heading) {
  double brg   = bearing(myFix.lat, myFix.lng, peers[who].lat, peers[who].lng);
  float  angle = brg - heading;            // screen angle = target bearing - own heading

  int cx = 120, cy = 85, r = 40;
  tft.fillRect(78, 43, 84, 84, COL_BG);    // clear only the arrow area
  drawArrow(cx, cy, r, angle, ST77XX_RED);
}

void drawAlarmInfo(int who) {
  double dist = haversine(myFix.lat, myFix.lng, peers[who].lat, peers[who].lng);
  tft.fillRect(148, 38, 84, 22, COL_BG);   // clear only the distance field
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(150, 40);
  tft.print(dist, 0); tft.print(" m");
}

// ---------- SENDER (this device is in danger) ----------

void drawSendingLayout() {
  tft.fillScreen(COL_BG);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.setCursor(15, 18);
  tft.print("ALARM");
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(15, 65);
  tft.print("Aid is on");
  tft.setCursor(15, 90);
  tft.print("its way");
}