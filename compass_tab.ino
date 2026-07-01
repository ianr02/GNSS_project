// ============================================================
//  Kompass / Heading (Blickrichtung des Geraets)
//
//  ZWISCHENLOESUNG MIT GYROSKOP (QMI8658C, onboard):
//  Wir setzen beim Start einen KUENSTLICHEN NORDEN (= Richtung,
//  in die das Board beim Einschalten zeigt) und verfolgen mit dem
//  Gyroskop, um wie viel sich das Board seitdem gedreht hat.
//  -> Der Pfeil bleibt auf diesem Punkt kleben, wenn man dreht.
//
//  Mittwoch ersetzen wir das durch das echte Magnetometer (QMC5883L),
//  das einen ABSOLUTEN Norden ohne Drift liefert. Dann nur
//  COMPASS_USE_GYRO auf false setzen und Magnetometer-Code einsetzen.
// ============================================================

#include <SensorQMI8658.hpp>

#define QMI_ADDR 0x6B

// true  = kuenstlicher Norden ueber Gyroskop (jetzt)
// false = (spaeter) echter Magnetometer-Norden
#define COMPASS_USE_GYRO true

SensorQMI8658 qmi;
IMUdata gyr;

float headingDeg  = 0.0;      // Blickrichtung relativ zum kuenstlichen Norden
float gyroBiasZ   = 0.0;      // Ruhe-Offset des Gyros (gegen Drift)
unsigned long lastGyroUs = 0;

// Vorzeichen der Drehung. Falls der Pfeil dem Board FOLGT statt auf dem
// Punkt zu bleiben: hier von 1.0 auf -1.0 aendern (oder umgekehrt).
#define GYRO_SIGN 1.0

void compassBegin() {
#if COMPASS_USE_GYRO
  if (!qmi.begin(Wire, QMI_ADDR, I2C_SDA, I2C_SCL)) {
    Serial.println("QMI8658 nicht gefunden! (Gyro)");
    return;
  }
  qmi.configGyroscope(SensorQMI8658::GYR_RANGE_512DPS,   // grosser Bereich = schnelle Drehung ok
                      SensorQMI8658::GYR_ODR_896_8Hz,
                      SensorQMI8658::LPF_MODE_3);
  qmi.enableGyroscope();

  // --- Gyro-Bias messen: Board ~1s voellig ruhig halten ---
  // Ein Gyro misst auch im Stillstand einen kleinen Wert. Den mitteln
  // wir und ziehen ihn spaeter ab, sonst "wandert" der Norden langsam.
  Serial.println("Gyro-Bias: Board 1s ruhig halten...");
  delay(300);
  double sum = 0; int n = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < 1000) {
    if (qmi.getDataReady() && qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
      sum += gyr.z; n++;
    }
  }
  if (n > 0) gyroBiasZ = sum / n;
  Serial.print("Gyro-Bias Z = "); Serial.println(gyroBiasZ, 4);

  headingDeg  = 0.0;          // HIER ist jetzt der kuenstliche Norden
  lastGyroUs  = micros();
#endif
}

// Blickrichtung in Grad: 0 = (kuenstlicher) Norden, im Uhrzeigersinn.
float getHeading() {
#if COMPASS_USE_GYRO
  // Neue Gyro-Messung holen und ueber die Zeit aufsummieren (integrieren)
  if (qmi.getDataReady() && qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
    unsigned long now = micros();
    float dt = (now - lastGyroUs) / 1000000.0;     // vergangene Zeit in Sekunden
    lastGyroUs = now;

    float rate = GYRO_SIGN * (gyr.z - gyroBiasZ);  // Drehrate (deg/s), Bias entfernt
    headingDeg += rate * dt;                        // Winkel = Rate * Zeit, aufsummiert

    // auf 0..360 halten
    headingDeg = fmod(headingDeg, 360.0);
    if (headingDeg < 0) headingDeg += 360.0;
  }
  return headingDeg;
#else
  // Spaeter: echter Wert vom Magnetometer (absoluter Norden)
  return 0.0;
#endif
}