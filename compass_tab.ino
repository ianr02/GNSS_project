// ============================================================
//  Kompass / Heading (Blickrichtung des Geraets)
//
//  ACHTUNG - PLATZHALTER!
//  Solange das QMC5883L noch nicht da ist, liefert getHeading()
//  eine SIMULIERTE, sich langsam drehende Richtung, damit wir
//  den Pfeil schon zeichnen und testen koennen.
//  Mittwoch ersetzen wir NUR den Inhalt von getHeading() durch
//  echte Sensorwerte (+ Kalibrierung). Alles andere bleibt gleich.
// ============================================================

// true  = simulierte Drehung (zum Testen OHNE Sensor)
// false = echter Magnetometer-Sensor (ab Mittwoch)
#define COMPASS_SIMULATION true

void compassBegin() {
  // Spaeter (Mittwoch): hier QMC5883L starten + Kalibrierung laden.
  // Jetzt noch nichts zu tun.
}

// Blickrichtung in Grad: 0 = Nord, 90 = Ost, im Uhrzeigersinn.
float getHeading() {
#if COMPASS_SIMULATION
  // Simulation: volle Drehung ca. alle 18 Sekunden
  return fmod(millis() / 50.0, 360.0);
#else
  // Spaeter: echter Wert vom Magnetometer
  return 0.0;
#endif
}