#include "async_webserver_handler.h"
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "camera_handler.h"
#include "config.h"
#include "servo_handler.h"

namespace {

AsyncWebServer server(80);
bool ledState = false;
volatile bool streamActive = false;

const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace; boundary=frame";
const char *FIRST_BOUNDARY = "--frame\r\n";
const char *NEXT_BOUNDARY = "\r\n--frame\r\n";
const char *FRAME_HEADER = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

class AsyncMjpegResponse : public AsyncAbstractResponse {
public:
    AsyncMjpegResponse() {
        _code = 200;
        _contentType = STREAM_CONTENT_TYPE;
        _contentLength = 0;
        _sendContentLength = false;
        _chunked = true;
        _callback = nullptr;
    }

    ~AsyncMjpegResponse() override {
        releaseFrame();
        streamActive = false;
    }

    bool _sourceValid() const override {
        return true;
    }

    size_t _fillBuffer(uint8_t *buffer, size_t maxLen) override {
        if (maxLen < 128) {
            return RESPONSE_TRY_AGAIN;
        }

        if (frameData != nullptr && frameIndex == frameLength) {
            releaseFrame();
        }

        if (frameData == nullptr) {
            // tu ewentualnie opoznienie aby zmnieszyc fps
            camera_fb_t *frame = esp_camera_fb_get();

            if (frame == nullptr) {
                Serial.println("Nie udalo sie pobrac klatki");
                return 0;
            }

            if (frame->format != PIXFORMAT_JPEG) {
                Serial.println("Oczekiwano formatu JPEG");
                esp_camera_fb_return(frame);
                return 0;
            }

            frameData = static_cast<uint8_t *>(ps_malloc(frame->len));
            if (frameData == nullptr) {
                frameData = static_cast<uint8_t *>(malloc(frame->len));
            }

            if (frameData == nullptr) {
                Serial.println("Nie udalo sie zaalokowac bufora klatki");
                esp_camera_fb_return(frame);
                return 0;
            }

            frameLength = frame->len;
            memcpy(frameData, frame->buf, frameLength);
            esp_camera_fb_return(frame);
        }

        size_t copied = 0;

        if (frameIndex == 0) {
            const char *boundary = firstFrame ? FIRST_BOUNDARY : NEXT_BOUNDARY;
            const size_t boundaryLength = strlen(boundary);

            char header[64];
            int headerLength = snprintf(
                header,
                sizeof(header),
                FRAME_HEADER,
                static_cast<unsigned int>(frameLength)
            );

            if (headerLength <= 0 ||
                static_cast<size_t>(headerLength) >= sizeof(header)) {
                releaseFrame();
                return 0;
            }

            size_t prefixLength = boundaryLength + headerLength;
            if (maxLen < prefixLength) {
                return RESPONSE_TRY_AGAIN;
            }

            memcpy(buffer, boundary, boundaryLength);
            memcpy(buffer + boundaryLength, header, headerLength);
            copied = prefixLength;
            firstFrame = false;
        }

        size_t remaining = frameLength - frameIndex;
        size_t jpegBytes = min(remaining, maxLen - copied);
        memcpy(buffer + copied, frameData + frameIndex, jpegBytes);

        frameIndex += jpegBytes;
        return copied + jpegBytes;
    }

private:
    uint8_t *frameData = nullptr;
    size_t frameLength = 0;
    size_t frameIndex = 0;
    bool firstFrame = true;

    void releaseFrame() {
        if (frameData != nullptr) {
            free(frameData);
            frameData = nullptr;
        }

        frameLength = 0;
        frameIndex = 0;
    }
};

void handleStream(AsyncWebServerRequest *request) {
    if (streamActive) {
        request->send(503, "text/plain", "Stream already active");
        return;
    }

    streamActive = true;
    AsyncMjpegResponse *response = new AsyncMjpegResponse();

    if (response == nullptr) {
        streamActive = false;
        request->send(500, "text/plain", "Out of memory");
        return;
    }

    response->addHeader("Cache-Control", "no-cache, no-store");
    response->addHeader("Access-Control-Allow-Origin", "*");

    Serial.println("Uruchomiono stream");
    request->send(response);
}

void handleGetStatus(AsyncWebServerRequest *request) {
    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    String json = "{\"temperature_c\":" + String(temperatureRead(), 2);
    json += ",\"wifi_connected\":";
    json += wifiConnected ? "true" : "false";
    json += ",\"wifi_rssi_dbm\":";
    if (wifiConnected) {
        json += String(WiFi.RSSI());
    } else {
        json += "null";
    }
    json += ",\"uptime_seconds\":";
    json += String(millis() / 1000UL);
    json += ",\"free_heap_bytes\":";
    json += String(ESP.getFreeHeap());
    json += ",\"free_psram_bytes\":";
    json += String(ESP.getFreePsram());
    json += ",\"stream_active\":";
    json += streamActive ? "true}" : "false}";

    request->send(200, "application/json", json);
}

void handleCameraSetting(AsyncWebServerRequest *request) {
    if (!request->hasParam("setting") || !request->hasParam("value")) {
        request->send(400, "text/plain", "Missing setting or value");
        return;
    }

    const String settingName = request->getParam("setting")->value();
    const String valueText = request->getParam("value")->value();
    char *end = nullptr;
    const long requestedValue = strtol(valueText.c_str(), &end, 10);
    if (valueText.isEmpty() || end == valueText.c_str() || *end != '\0') {
        request->send(400, "text/plain", "Value must be an integer");
        return;
    }

    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        Serial.println("Sensor kamery niedostepny");
        request->send(503, "text/plain", "Camera sensor unavailable");
        return;
    }

    int minValue;
    int maxValue;
    int sensorOffset = 0;
    int (*setValue)(sensor_t *, int);

    if (settingName == "brightness") {
        minValue = 0;
        maxValue = 4;
        sensorOffset = 2;
        setValue = sensor->set_brightness;
    } else if (settingName == "contrast") {
        minValue = 0;
        maxValue = 4;
        sensorOffset = 2;
        setValue = sensor->set_contrast;
    } else if (settingName == "saturation") {
        minValue = 0;
        maxValue = 4;
        sensorOffset = 2;
        setValue = sensor->set_saturation;
    } else if (settingName == "sharpness") {
        minValue = 0;
        maxValue = 4;
        sensorOffset = 2;
        setValue = sensor->set_sharpness;
    } else if (settingName == "quality") {
        minValue = 10;
        maxValue = 63;
        setValue = sensor->set_quality;
    } else {
        request->send(400, "text/plain", "Unknown camera setting");
        return;
    }

    if (requestedValue < minValue || requestedValue > maxValue) {
        Serial.printf( "Odrzucono %s: wartosc %ld poza zakresem %d-%d\n", settingName.c_str(), requestedValue, minValue, maxValue);
        request->send(400, "text/plain", "Camera setting is out of range");
        return;
    }

    if (setValue == nullptr) {
        Serial.printf("Nie obslugiwane ustawienie kamery: %s\n", settingName.c_str());
        request->send(500, "text/plain", "Failed to update camera setting");
        return;
    }

    const int result = setValue(sensor, static_cast<int>(requestedValue) - sensorOffset);
    if (result != 0) {
        Serial.printf("Blad zmiany %s na %ld, kod: %d\n", settingName.c_str(), requestedValue, result);
        request->send(500, "text/plain", "Failed to update camera setting");
        return;
    }

    Serial.printf("Ustawiono %s na %ld\n", settingName.c_str(), requestedValue);
    request->send(200, "text/plain", "OK");
}

void handleRotateLeft(AsyncWebServerRequest *request) {
    move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_left, ServoConfig::rotation_delay);
    request->send(200, "text/plain", "OK");
}

void handleRotateRight(AsyncWebServerRequest *request) {
    move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_right, ServoConfig::rotation_delay);
    request->send(200, "text/plain", "OK");
}

void handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

}  // namespace

void asyncWebServerSetup() {
    server.on("/stream", HTTP_GET, handleStream);
    server.on("/get_status", HTTP_GET, handleGetStatus);
    server.on("/set_camera", HTTP_GET, handleCameraSetting);
    server.on("/rotate_left", HTTP_GET, handleRotateLeft);
    server.on("/rotate_right", HTTP_GET, handleRotateRight);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Asynchroniczny serwer uruchomiony");
}
