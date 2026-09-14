#pragma once
#include <Arduino.h>

namespace WifiConfig {
    // Wifi dane
    constexpr char* ssid = "zastąp_to_loginem";
    constexpr char* password = "zastąp_to_hasłem";
}

namespace MQTTConfig {
    // MQTT dane
    constexpr char mqtt_server[] = "zastąp_to_adresem_serwera"; // np 34734fdf89fdsfd.s1.eu.hivemq.cloud 
    constexpr int mqtt_port = 8883; // lub inny port, jeśli twój broker używa innego
    constexpr char mqtt_user[] = "zastąp_to_loginem"; // jezeli nie masz hasla zostaw puste
    constexpr char mqtt_pass[] = "zastąp_to_hasłem";
}