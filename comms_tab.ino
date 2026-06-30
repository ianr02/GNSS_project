#include "config.h"

uint8_t broadcastAddr[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned long lastTx = 0;
bool wifiConnected = false;

typedef struct message {
    char device_id[16];
    float latitude;
    float longitude;
} message;


message myLocationData = {"NODE_01", 37.7749, -122.4194}; 
message incomingAlert;


int firstActivePeer() {
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && i != DEVICE_ID) return i;
  return 0;
}

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3,0,0)
  void onDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
#else
  void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
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
  // Connect to Wifi
WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);              // keep radio awake so ESP-NOW is reliable

  if (strlen(SSID) > 0) {            // only try if an SSID is configured
    WiFi.begin(SSID, PASSWORD);
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
      delay(250);                    // give up after 8 s -> setup never freezes
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("WiFi connected, IP: ");
      Serial.println(WiFi.localIP());
      udp.begin(PORT);
      wifiConnected = true;
    } else {
      Serial.println("WiFi NOT connected (continuing, no PC link)");
    }
  }


  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init ERROR"); return; }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) Serial.println("Broadcast Peer ERROR");
  Serial.println("ESP-NOW ready");
  
}

void commsUpdate() {
  if (myAlarm) {
    if (millis() - lastTx >= 200) {
      lastTx = millis();
      char buf[64];
      int n = snprintf(buf, sizeof(buf), "%d,%.6f,%.6f,%d",
                      DEVICE_ID, myFix.lat, myFix.lng, myAlarm);
      if (wifiConnected) {
        udp.beginPacket(BROADCAST, PORT);
        udp.write((uint8_t*)buf, n);
        udp.endPacket();
      }
      esp_now_send(broadcastAddr, (uint8_t*)buf, n);
    }
  }
  /*
  // Send own position once per second (only with a valid fix)
  if (millis() - lastTx >= TX_INTERVAL) {
    lastTx = millis();
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d,%.6f,%.6f,%d",
                     DEVICE_ID, myFix.lat, myFix.lng, myAlarm);
    esp_now_send(broadcastAddr, (uint8_t*)buf, n);
  }
  */
  // Remove peers that haven't sent anything for too long (timeout)
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && millis() - peers[i].lastSeen > PEER_TIMEOUT)
      peers[i].active = false;
}