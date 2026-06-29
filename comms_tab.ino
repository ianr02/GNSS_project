#include <esp_now.h>
#include <WiFi.h>
#include <esp_arduino_version.h>

uint8_t broadcastAddr[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned long lastTx = 0;

// Peers, indexiert nach DEVICE_ID (1..3)
struct Peer {
  bool   active = false;
  double lat = 0, lng = 0;
  int    alarm = 0;
  unsigned long lastSeen = 0;
};
Peer peers[4];

int firstActivePeer() {
  for (int i = 1; i <= 3; i++)
    if (peers[i].active && i != DEVICE_ID) return i;
  return 0;
}

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
  void onDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
#else
  void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  // ---- ab hier bleibt ALLES wie gehabt ----
  char buf[64];
  if (len >= (int)sizeof(buf)) len = sizeof(buf) - 1;
  memcpy(buf, data, len);
  buf[len] = '\0';

  int id, alarm;
  double lat, lng;
  if (sscanf(buf, "%d,%lf,%lf,%d", &id, &lat, &lng, &alarm) == 4) {
    if (id >= 1 && id <= 3 && id != DEVICE_ID) {
      peers[id].active   = true;
      peers[id].lat      = lat;
      peers[id].lng      = lng;
      peers[id].alarm    = alarm;
      peers[id].lastSeen = millis();
    }
  }
} 

void commsBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init FEHLER"); return; }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) Serial.println("Broadcast-Peer FEHLER");
  Serial.println("ESP-NOW bereit");
}

void commsUpdate() {
  // 1x pro Sekunde eigene Position senden (nur bei gültigem Fix)
  if (millis() - lastTx >= TX_INTERVAL) {
    lastTx = millis();
    if (myFix.valid) {
      char buf[64];
      int n = snprintf(buf, sizeof(buf), "%d,%.6f,%.6f,%d",
                       DEVICE_ID, myFix.lat, myFix.lng, myAlarm);
      esp_now_send(broadcastAddr, (uint8_t*)buf, n);
    }
  }
  // Peers ausmustern, die zu lange nichts gesendet haben
  for (int i = 1; i <= 3; i++)
    if (peers[i].active && millis() - peers[i].lastSeen > PEER_TIMEOUT)
      peers[i].active = false;
}