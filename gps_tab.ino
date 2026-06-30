#include "config.h"
// ---- GNSS: read, parse, check fix ----

void gpsBegin() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
}

void gpsUpdate() {
  // Feed all waiting bytes into the parser (non-blocking)
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // NEU: Qualitätswerte holen (vor dem if, damit wir sie in der Bedingung nutzen können)
  double hdop = gps.hdop.isValid()       ? gps.hdop.hdop()        : 99.0;  // 99 = unbekannt/schlecht
  int    sats = gps.satellites.isValid() ? gps.satellites.value() : 0;

  // Accept fix only if position is VALID and FRESH (< 2 s old).
  // age() protects us from showing old ghost coordinates in case of signal loss.
  if (gps.location.isValid() && gps.location.age() < 2000
      && hdop < 5.0 && sats >= 5) {                 // NEU: hdop < 5.0 && sats >= 5
    myFix.valid    = true;
    myFix.lat      = gps.location.lat();
    myFix.lng      = gps.location.lng();
    myFix.sats     = sats;                           // nutzt jetzt die Variable von oben
    if (gps.speed.isValid()) myFix.speedKmh = gps.speed.kmph();
  } else {
    myFix.valid = false;
  }
}

/* TEACHING/REFERENCE FUNCTION: manual conversion ddmm.mmmm -> decimal degrees.
// With TinyGPSPlus we do NOT need this (see explanation), but this way the
// team understands what happens under the hood.
double nmeaToDecimal(double ddmm, char hemi) {
  int    deg     = (int)(ddmm / 100.0);          // Degree component
  double minutes = ddmm - deg * 100.0;           // Minute component
  double dec     = deg + minutes / 60.0;         // -> Decimal degrees
  if (hemi == 'S' || hemi == 'W') dec = -dec;    // South/West negative
  return dec;
}
*/