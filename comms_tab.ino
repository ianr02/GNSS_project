#include "config.h"
#include <esp_wifi.h> 

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

  // Closer to zero -> Better
  // if -70 at 20 m -> Good 
  // -90 -> bad
  // RSSI -54 : 2,53.350120,-6.298400,0     <- von Gerät 2 (nah, starkes Signal)
  // RSSI -71 : 3,53.349800,-6.301100,0     <- von Gerät 3 (weiter weg
  //                                                        ,schwächer)
  Serial.printf("RSSI %d : %s\n", info->rx_ctrl->rssi, buf);  

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
  // Old commsBegin
  /*
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);     

  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false; 
  esp_now_add_peer(&peerInfo);
  Serial.println("ESP-NOW only");
  */ 

  // New one (01.07. 23:25)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);

  WiFi.setTxPower(WIFI_POWER_19_5dBm);                 // max transmit power
  esp_wifi_set_protocol(WIFI_IF_STA,
      WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G |
      WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);           // Long-Range mode
  esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE);      // all devices on ch 1

  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); return; }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddr, 6);
  peerInfo.channel = 11;                               
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
  Serial.println("ESP-NOW ready (LR, max power, ch1)");
}


void commsUpdate() {
  if (millis() - lastTx >= TX_INTERVAL) {
    lastTx = millis();
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d,%.6f,%.6f,%d",
                     DEVICE_ID, myFix.lat, myFix.lng, myAlarm);
    esp_now_send(broadcastAddr, (uint8_t*)buf, n);
    if (WiFi.status() == WL_CONNECTED) {
      udp.beginPacket(BROADCAST, PORT);
      udp.write((const uint8_t*)buf, n);
      udp.endPacket();
    }
  }

  // Remove peers that haven't sent anything for too long (timeout)
  for (int i = 1; i <= NUM_PEERS; i++)
    if (peers[i].active && millis() - peers[i].lastSeen > PEER_TIMEOUT)
      peers[i].active = false;
}