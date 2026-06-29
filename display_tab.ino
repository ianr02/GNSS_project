// ---- TFT: alles Zeichnen ----

void displayBegin() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init(135, 240);     // Panel 135 x 240
  tft.setRotation(3);     // Querformat: 240 breit x 135 hoch
  tft.fillScreen(COL_BG);
}

// 3 px dicker Rahmen – Farbe signalisiert den Zustand
void drawBorder(uint16_t color) {
  for (int i = 0; i < 3; i++)
    tft.drawRect(i, i, tft.width() - 2 * i, tft.height() - 2 * i, color);
}

// Zeichnet einen gefuellten Pfeil, der in Richtung angleDeg zeigt.
// 0 = oben/Norden auf dem Bildschirm, im Uhrzeigersinn.
void drawArrow(int cx, int cy, int r, float angleDeg, uint16_t color) {
  float a = radians(angleDeg);
  // Spitze
  int tipX = cx + r * sin(a);
  int tipY = cy - r * cos(a);
  // zwei hintere Ecken (Pfeil-Fluegel)
  int lX = cx + (r * 0.6) * sin(a + radians(150));
  int lY = cy - (r * 0.6) * cos(a + radians(150));
  int rX = cx + (r * 0.6) * sin(a - radians(150));
  int rY = cy - (r * 0.6) * cos(a - radians(150));
  tft.fillTriangle(tipX, tipY, lX, lY, rX, rY, color);
}

void displayWaiting() {
  tft.fillScreen(COL_BG);
  drawBorder(COL_WARN);
  tft.setTextColor(COL_WARN);
  tft.setTextSize(2);
  tft.setCursor(12, 20);
  tft.println("Acquiring fix...");
  tft.setTextSize(1);
  tft.setTextColor(COL_TEXT);
  tft.setCursor(12, 60);
  tft.print("Sats visible: ");
  tft.print(gps.satellites.value());
  tft.setCursor(12, 80);
  tft.print("Hold antenna to window");
}


void displayPosition(const GpsFix &f) {
  tft.fillScreen(COL_BG);
  drawBorder(COL_OK);

  tft.setTextColor(COL_OK);
  tft.setTextSize(2);
  tft.setCursor(8, 6);
  tft.print("DEVICE "); tft.print(DEVICE_ID);

  tft.setTextColor(COL_TEXT);
  tft.setTextSize(1);
  tft.setCursor(8, 32);
  tft.print("Lat: "); tft.print(f.lat, 6);
  tft.setCursor(8, 44);
  tft.print("Lon: "); tft.print(f.lng, 6);
  tft.setCursor(8, 56);
  tft.print(f.speedKmh, 1); tft.print(" km/h  Sats:"); tft.print(f.sats);

  // Distance + bearing to the first active device
  int other = firstActivePeer();
  if (other > 0) {
    double d = haversine(f.lat, f.lng, peers[other].lat, peers[other].lng);
    double b = bearing (f.lat, f.lng, peers[other].lat, peers[other].lng);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(8, 80);
    tft.print("-> D"); tft.print(other); tft.print(": ");
    tft.print(d, 0); tft.print("m");
    tft.setTextSize(1);
    tft.setTextColor(COL_TEXT);
    tft.setCursor(8, 110);
    tft.print("Bearing: "); tft.print(b, 0); tft.print(" deg");
  } else {
    tft.setTextColor(COL_WARN);
    tft.setTextSize(1);
    tft.setCursor(8, 90);
    tft.print("Waiting for other device...");
  }
    // ---- Kompass / Nordpfeil (rechts auf dem Display) ----
  float heading = getHeading();
  int cx = 195, cy = 60, r = 38;          // Kreis-Mittelpunkt rechts
  tft.drawCircle(cx, cy, r, COL_TEXT);    // Kompass-Ring

  // Der Pfeil zeigt nach NORDEN, relativ zur Blickrichtung:
  // Pfeilwinkel = 0 (Nord) - Blickrichtung
  drawArrow(cx, cy, r - 6, -heading, ST77XX_RED);

  // "N" an die Stelle malen, wohin der Pfeil zeigt
  float na = radians(-heading);
  tft.setTextColor(COL_TEXT);
  tft.setTextSize(1);
  tft.setCursor(cx + (r + 4) * sin(na) - 3, cy - (r + 4) * cos(na) - 3);
  tft.print("N");
}