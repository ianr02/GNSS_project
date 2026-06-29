// ---- GNSS: lesen, parsen, Fix prüfen ----

void gpsBegin() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
}

void gpsUpdate() {
  // alle wartenden Bytes in den Parser geben (nicht-blockierend)
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // NEU: Qualitätswerte holen (vor dem if, damit wir sie in der Bedingung nutzen können)
  double hdop = gps.hdop.isValid()       ? gps.hdop.hdop()        : 99.0;  // 99 = unbekannt/schlecht
  int    sats = gps.satellites.isValid() ? gps.satellites.value() : 0;

  // Fix nur akzeptieren, wenn: gültig UND frisch (<2s) UND gute Qualität
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

// LEHR-/REFERENZFUNKTION: manuelle Umrechnung ddmm.mmmm -> Dezimalgrad.
// Mit TinyGPSPlus brauchen wir das NICHT (siehe Erklärung), aber so versteht
// das Team, was unter der Haube passiert.
double nmeaToDecimal(double ddmm, char hemi) {
  int    deg     = (int)(ddmm / 100.0);          // Grad-Anteil
  double minutes = ddmm - deg * 100.0;           // Minuten-Anteil
  double dec     = deg + minutes / 60.0;         // -> Dezimalgrad
  if (hemi == 'S' || hemi == 'W') dec = -dec;    // Süd/West negativ
  return dec;
}