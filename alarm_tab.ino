// ===================================================================
//  alarm_tab - Alarm screen
//  RECEIVER: another device raised the alarm -> big arrow toward the
//            person in danger (left) + distance (right), red blinking border.
//  SENDER:   THIS device raised the alarm -> red border, "Aid is on its way",
//            and how many devices are on the way.
// ===================================================================
#include "config.h"

// First active peer that has raised an alarm (0 = none).
int findAlarmPeer() {
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && peers[i].alarm == 1) return i;
  return 0;
}

// How many other devices are currently online (= helpers on the way).
int countActivePeers() {
  int n = 0;
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && i != DEVICE_ID) n++;
  return n;
}

// Red blinking border (toggles ~every 400 ms). Cheap; call it often.
void drawAlarmBorder() {
  bool on = (millis() / 400) % 2;
  uint16_t c = on ? ST77XX_RED : COL_BG;
  for (int i = 0; i < 4; i++)
    tft.drawRect(i, i, tft.width() - 2 * i, tft.height() - 2 * i, c);
}

// ---------- RECEIVER (someone else is in danger) ----------

// Static parts, drawn once on entering alarm mode.
void drawAlarmLayout(int who) {
  tft.fillScreen(COL_BG);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.setCursor(8, 8);
  tft.print("ALARM");
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(128, 42);
  tft.print("To D"); tft.print(who);
}

// Big arrow on the LEFT half. Fast updates.
void drawAlarmArrow(int who, float heading) {
  double brg   = bearing(myFix.lat, myFix.lng, peers[who].lat, peers[who].lng);
  float  angle = brg - heading;            // screen angle = target bearing - own heading

  int cx = 68, cy = 82, r = 42;
  tft.fillRect(22, 38, 92, 90, COL_BG);    // clear only the (left) arrow area
  drawArrow(cx, cy, r, angle, ST77XX_RED);
}

// Distance on the RIGHT half, below the "To Dx" label. Slow updates.
void drawAlarmInfo(int who) {
  double dist = haversine(myFix.lat, myFix.lng, peers[who].lat, peers[who].lng);
  tft.fillRect(126, 66, 110, 34, COL_BG);  // clear only the distance field (right side)
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(3);
  tft.setCursor(128, 70);
  tft.print(dist, 0); tft.print("m");
}

// ---------- SENDER (this device is in danger) ----------

void drawSendingLayout() {
  tft.fillScreen(COL_BG);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.setCursor(15, 12);
  tft.print("ALARM");
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(2);
  tft.setCursor(15, 52);
  tft.print("Aid is on");
  tft.setCursor(15, 74);
  tft.print("its way");
}

// Number of devices on the way. Slow updates.
void drawSendingInfo() {
  int n = countActivePeers();
  tft.fillRect(15, 104, 215, 22, COL_BG);
  tft.setTextColor(n > 0 ? COL_OK : COL_WARN);
  tft.setTextSize(2);
  tft.setCursor(15, 106);
  tft.print(n); tft.print(" on the way");
}