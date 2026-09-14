#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "camera_handler.h"
#include "mqtt_handler.h"

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  SprawdzPSRAM();

  // tu pewnie inicjacja kamery

  // ps_malloc dla wykrywania ruchu w przyszlosci

  // Połączenie Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiConfig::ssid, WifiConfig::password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Połączone. IP: " + WiFi.localIP().toString());

  mqttSetup();
}

void loop() {
  mqttLoop();

}

