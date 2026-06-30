/*
   BIP26 Campus Emergency Demo
   Stage 1
   Creates a WiFi hotspot and sends default GPS data
*/

#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* ssid = "Campus Emergency";
const char* password = "A7654321";


void setup()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println("Starting Campus Emergency");

  // Create WiFi Hotspot
  WiFi.softAP(ssid, password);

  Serial.print("SSID: ");
  Serial.println(ssid);

  Serial.print("Password: ");
  Serial.println(password);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/status", handleStatus);

  server.begin();

  Serial.println("Web Server Started");
}

void loop()
{
  server.handleClient();
}