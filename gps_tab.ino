// ---- GNSS: lesen, parsen, Fix prüfen ----

void gpsBegin() {
  Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
}

void gpsUpdate() {
  // alle wartenden Bytes in den Parser geben (nicht-blockierend)
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Fix nur akzeptieren, wenn Position GÜLTIG und FRISCH (< 2 s alt) ist.
  // age() schützt davor, dass wir bei Signalverlust alte Geisterkoordinaten zeigen.
  if (gps.location.isValid() && gps.location.age() < 2000) {
    myFix.valid = true;
    myFix.lat   = gps.location.lat();   // siehe Hinweis unten zur Umrechnung
    myFix.lng   = gps.location.lng();
    if (gps.speed.isValid())      myFix.speedKmh = gps.speed.kmph();
    if (gps.satellites.isValid()) myFix.sats     = gps.satellites.value();
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