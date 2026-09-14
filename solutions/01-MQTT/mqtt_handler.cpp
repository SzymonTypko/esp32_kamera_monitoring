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
			client.subscribe(MQTTConfig::topic_capture);
      		client.subscribe(MQTTConfig::topic_set_all);
		} else {
			Serial.print(" Błąd: ");
			Serial.print(client.state());
			Serial.println(" Ponowna próba za 4 sekundy...");
			delay(4000);
		}
	}
}
}
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.printf("[MQTT] Odebrano komendę na [%s]: %s\n", topic, message.c_str());
  String top = String(topic);
  if (top.endsWith("/capture")) {
    // zrob zdjecie
  } else if (top.endsWith("/brightness")) {
    // ustaw jasność
  } else if (top.endsWith("/contrast")) {
    // ustaw kontrast
  } else if (top.endsWith("/saturation")) {
    // ustaw nasycenie
  } else if (top.endsWith("/sharpness")) {
    // ustaw ostrość
  } else if (top.endsWith("/quality")) {
    // ustaw jakość
  } else if (top.endsWith("/rotate_left")) {
    // obracaj serwo w lewo
  } else if (top.endsWith("/rotate_right")) {
    // obracaj w prawo
  }
}

void mqttSetup() {
	espClient.setInsecure(); // bez SSL

	client.setServer(MQTTConfig::mqtt_server, MQTTConfig::mqtt_port);
	client.setCallback(callback);

	// pewnie wiekszy bufor dla przesylania zdjec
}

void mqttLoop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}
