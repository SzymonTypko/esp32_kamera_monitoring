#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <mbedtls/base64.h>

#include "config.h"
#include "mqtt_handler.h"
#include "camera_handler.h"
#include "servo_handler.h"

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

void takeAndSendPhoto() {
  Serial.println("[KAMERA] Robię zdjęcie...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (fb) { esp_camera_fb_return(fb); }
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[KAMERA] Błąd pobierania ramki z kamery");
    return;
  }
  Serial.printf("[KAMERA] Zdjęcie zrobione. Rozmiar JPEG: %u B\n", fb->len);

  String base64Prefix = "data:image/jpeg;base64,";
  size_t prefixLen = base64Prefix.length();
  size_t base64DataLen = 0;
  mbedtls_base64_encode(NULL, 0, &base64DataLen, fb->buf, fb->len);
  size_t totalPayloadLen = prefixLen + base64DataLen;
  size_t maxPayloadLen = 65535 - 5 - 2 - strlen(MQTTConfig::topic_foto);
  Serial.printf("[MQTT] Base64: %u B, limit payloadu: %u B\n", totalPayloadLen, maxPayloadLen);

  char* payloadBuffer = (char*)ps_malloc(totalPayloadLen + 1);
  if (!payloadBuffer) {
    payloadBuffer = (char*)malloc(totalPayloadLen + 1);
  }

  if (payloadBuffer) {
    memcpy(payloadBuffer, base64Prefix.c_str(), prefixLen);
    size_t written = 0;
    mbedtls_base64_encode((unsigned char*)(payloadBuffer + prefixLen), base64DataLen + 1, &written, fb->buf, fb->len);
    payloadBuffer[totalPayloadLen] = '\0';
    Serial.printf("[MQTT] Wysyłam Base64 (Łącznie: %u B)...\n", totalPayloadLen);
    // Wysyłanie w chunkach 1024, aby uniknąć problemów z buforem
    constexpr size_t chunkSize = 1024;
    bool success = client.beginPublish(MQTTConfig::topic_foto, totalPayloadLen, true);
    if (success) {
      const uint8_t* payload = (const uint8_t*)payloadBuffer;
      size_t bytesSent = 0;
      while (bytesSent < totalPayloadLen) {
        size_t bytesToSend = min(chunkSize, totalPayloadLen - bytesSent);
        size_t writtenChunk = client.write(payload + bytesSent, bytesToSend);
        if (writtenChunk != bytesToSend) {
          success = false;
          break;
        }
        bytesSent += writtenChunk;
      }
      if (success) {
        success = client.endPublish();
      }
    }
    if (success) {
      Serial.println("[MQTT] Zdjęcie wysłane pomyślnie");
      unsigned long totalSec = millis() / 1000;
      unsigned long min = (totalSec / 60) % 60;
      unsigned long hrs = totalSec / 3600;
      unsigned long sec = totalSec % 60;
      char timeStr[64];
      snprintf(timeStr, sizeof(timeStr), "Zdjęcie z %02luh %02lum %02lus (Uptime)", hrs, min, sec);
      client.publish(MQTTConfig::topic_timestamp, timeStr);
      Serial.printf("[MQTT] Timestamp wysłany: %s\n", timeStr);
    } else {
      Serial.printf("[MQTT] Publikacja nieudana. Stan klienta: %d, payload: %u/%u B\n",
                    client.state(), totalPayloadLen, maxPayloadLen);
    }
    free(payloadBuffer);
  } else {
    Serial.println("[BŁĄD] Brak pamięci na bufor Base64");
  }
  esp_camera_fb_return(fb);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.printf("[MQTT] Odebrano komendę na [%s]: %s\n", topic, message.c_str());
  String top = String(topic);
  sensor_t* s = esp_camera_sensor_get();

  if (top.endsWith("/capture")) {
    takeAndSendPhoto();
  } else if (top.endsWith("/brightness")) {
    s->set_brightness(s, message.toInt() - 2);  // [0 ; 4] -> [-2 ; 2] // u mnie na cliencie nie da sie ustawic wartosci ujemnych wiec konwertuje
  } else if (top.endsWith("/contrast")) {
    s->set_contrast(s, message.toInt() - 2);  // [0 ; 4] -> [-2 ; 2]
  } else if (top.endsWith("/saturation")) {
    s->set_saturation(s, message.toInt() - 2);  // [0 ; 4] -> [-2 ; 2]
  } else if (top.endsWith("/sharpness")) {
    s->set_sharpness(s, message.toInt() - 2);  // [0 ; 4] -> [-2 ; 2]
  } else if (top.endsWith("/quality")) {
    s->set_quality(s, message.toInt());            // [10 ; 63]
  } else if (top.endsWith("/rotate_left")) {
    move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_left, ServoConfig::rotation_delay);
  } else if (top.endsWith("/rotate_right")) {
    move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_right, ServoConfig::rotation_delay);
  }
}

void mqttSetup() {
	espClient.setInsecure(); // bez SSL

	client.setServer(MQTTConfig::mqtt_server, MQTTConfig::mqtt_port);
	client.setCallback(callback);

	Serial.println(client.setBufferSize(65535) ? "udalo sie ustawic bufor 65K" : "nie udalo sie ustawic bufora 65K");// 65535
}

void mqttLoop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}
