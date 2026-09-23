#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "camera_handler.h"
#include "mqtt_handler.h"
#include "utility.h"

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  SprawdzPSRAM();

  // Inicjalizacja kamery
  if (!initCamera()) {
    Serial.println("Nie udalo sie zainicjalizowac kamery");
    while (true) { delay(1000); }
  }

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

  setInterval(TempConfig::temp_send_interval, getTempAndSend); // wysylaj temperaturę co 5 minut
}

void loop() {
  mqttLoop();
  runIntervals();
}

