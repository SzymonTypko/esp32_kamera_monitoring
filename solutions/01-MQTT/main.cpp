#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "camera_handler.h"

WiFiClientSecure espClient;

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  SprawdzPSRAM();

  // Połączenie Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiConfig::ssid, WifiConfig::password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Połączone. IP: " + WiFi.localIP().toString());

  espClient.setInsecure(); // bez SSL

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

}

