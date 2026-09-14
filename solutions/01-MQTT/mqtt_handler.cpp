#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "config.h"
#include "mqtt_handler.h"

namespace {
WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnect() {
	while (!client.connected()) {
		Serial.print("[MQTT] Łączenie z HiveMQ Cloud...");
		String clientId = "XIAO-S3-Sense-" + String(random(0xffff), HEX);
		if (client.connect(clientId.c_str(), MQTTConfig::mqtt_user, MQTTConfig::mqtt_pass)) {
			Serial.println(" POŁĄCZONO BEZPIECZNIE!");
			// tu subskrybcje tematow
		} else {
			Serial.print(" Błąd: ");
			Serial.print(client.state());
			Serial.println(" Ponowna próba za 4 sekundy...");
			delay(4000);
		}
	}
}
}

void mqttSetup() {
	espClient.setInsecure(); // bez SSL

	client.setServer(MQTTConfig::mqtt_server, MQTTConfig::mqtt_port);
	//client.setCallback(callback); // na jakies przyciski itp

	// pewnie wiekszy bufor dla przesylania zdjec
}

void mqttLoop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}
