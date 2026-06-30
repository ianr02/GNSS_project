#include "config.h"
// ---- Geo-calculations ----

double toRad(double deg) { return deg * PI / 180.0; }
double toDeg(double rad) { return rad * 180.0 / PI; }

// Distance between two points in meters (Haversine)
double haversine(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000.0;                 // Earth radius in m
  double dLat = toRad(lat2 - lat1);
  double dLon = toRad(lon2 - lon1);
  double a = sin(dLat/2) * sin(dLat/2) +
             cos(toRad(lat1)) * cos(toRad(lat2)) * sin(dLon/2) * sin(dLon/2);
  return R * 2 * atan2(sqrt(a), sqrt(1 - a));
}

// Bearing from Point 1 -> Point 2, in degrees relative to North (0..360)
double bearing(double lat1, double lon1, double lat2, double lon2) {
  double dLon = toRad(lon2 - lon1);
  double y = sin(dLon) * cos(toRad(lat2));
  double x = cos(toRad(lat1)) * sin(toRad(lat2)) -
             sin(toRad(lat1)) * cos(toRad(lat2)) * cos(dLon);
  double b = toDeg(atan2(y, x));
  return fmod(b + 360.0, 360.0);
}