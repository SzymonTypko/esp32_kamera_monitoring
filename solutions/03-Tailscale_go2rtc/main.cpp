#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "camera_handler.h"
#include "async_webserver_handler.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    SprawdzPSRAM();

    // Inicjalizacja kamery
    if (!initCamera()) {
        Serial.println("Nie udalo sie zainicjalizowac kamery");
        while (true) { delay(1000); }
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WifiConfig::ssid, WifiConfig::password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi Połączone. IP: " + WiFi.localIP().toString());

    asyncWebServerSetup();
}

void loop() {
    delay(10);
}