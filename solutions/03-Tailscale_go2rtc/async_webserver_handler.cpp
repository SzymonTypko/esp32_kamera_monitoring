#include "async_webserver_handler.h"
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <cstdio>
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

enum class CameraSetting {
    Brightness,
    Contrast,
    Saturation,
    Sharpness,
    Quality
};

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

        if (frame != nullptr && frameIndex == frame->len) {
            releaseFrame();
        }

        if (frame == nullptr) {
            frame = esp_camera_fb_get();

            if (frame == nullptr) {
                Serial.println("Nie udalo sie pobrac klatki");
                return 0;
            }

            if (frame->format != PIXFORMAT_JPEG) {
                Serial.println("Oczekiwano formatu JPEG");
                releaseFrame();
                return 0;
            }
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
                static_cast<unsigned int>(frame->len)
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

        size_t remaining = frame->len - frameIndex;
        size_t jpegBytes = min(remaining, maxLen - copied);
        memcpy(buffer + copied, frame->buf + frameIndex, jpegBytes);

        frameIndex += jpegBytes;
        return copied + jpegBytes;
    }

private:
    camera_fb_t *frame = nullptr;
    size_t frameIndex = 0;
    bool firstFrame = true;

    void releaseFrame() {
        if (frame != nullptr) {
            esp_camera_fb_return(frame);
            frame = nullptr;
        }

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

void handleToggleLED(AsyncWebServerRequest *request) {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);

    String json = "{\"ledState\":" + String(ledState ? "true" : "false") + "}";
    request->send(200, "application/json", json);
}

void handleCameraSetting(AsyncWebServerRequest *request, CameraSetting setting, int direction) {
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        Serial.println("Nie mozna zmienic ustawienia: sensor kamery niedostepny");
        request->send(503, "text/plain", "Camera sensor unavailable");
        return;
    }

    int currentValue;
    int minValue;
    int maxValue;
    int sensorOffset = 0;
    int (*setValue)(sensor_t *, int);
    const char *settingName;

    switch (setting) {
        case CameraSetting::Brightness:
            currentValue = sensor->status.brightness + 2;
            minValue = 0;
            maxValue = 4;
            sensorOffset = 2;
            setValue = sensor->set_brightness; // bierze funkcje
            settingName = "brightness";
            break;
        case CameraSetting::Contrast:
            currentValue = sensor->status.contrast + 2;
            minValue = 0;
            maxValue = 4;
            sensorOffset = 2;
            setValue = sensor->set_contrast;
            settingName = "contrast";
            break;
        case CameraSetting::Saturation:
            currentValue = sensor->status.saturation + 2;
            minValue = 0;
            maxValue = 4;
            sensorOffset = 2;
            setValue = sensor->set_saturation;
            settingName = "saturation";
            break;
        case CameraSetting::Sharpness:
            currentValue = sensor->status.sharpness + 2;
            minValue = 0;
            maxValue = 4;
            sensorOffset = 2;
            setValue = sensor->set_sharpness;
            settingName = "sharpness";
            break;
        case CameraSetting::Quality:
            currentValue = sensor->status.quality;
            minValue = 10;
            maxValue = 63;
            setValue = sensor->set_quality;
            settingName = "quality";
            break;
    }

    Serial.printf("Zadanie zmiany %s (%+d)\n", settingName, direction);

    const int nextValue = currentValue + direction;
    if (nextValue < minValue || nextValue > maxValue) {
        Serial.printf("Odrzucono %s: wartosc %d poza zakresem %d-%d\n", settingName, nextValue, minValue, maxValue);
        request->send(400, "text/plain", "Camera setting is out of range");
        return;
    }

    if (setValue == nullptr) {
        Serial.printf("Nie obslugiwane ustawienie kamery: %s\n", settingName);
        request->send(500, "text/plain", "Failed to update camera setting");
        return;
    }

    const int result = setValue(sensor, nextValue - sensorOffset);
    if (result != 0) {
        Serial.printf(
            "Blad zmiany %s na %d, kod: %d\n",
            settingName,
            nextValue,
            result
        );
        request->send(500, "text/plain", "Failed to update camera setting");
        return;
    }

    Serial.printf("Ustawiono %s na %d\n", settingName, nextValue);
    request->send(200, "text/plain", "OK");
}

void registerCameraSettingRoutes(const char *name, CameraSetting setting) { // dodaje server.on dla up/down dla ustawień kamery
    String increasePath = String("/") + name + "_up";
    String decreasePath = String("/") + name + "_down";

    server.on(increasePath.c_str(), HTTP_GET, [setting](AsyncWebServerRequest *request) {
        handleCameraSetting(request, setting, 1);
    });
    server.on(decreasePath.c_str(), HTTP_GET, [setting](AsyncWebServerRequest *request) {
        handleCameraSetting(request, setting, -1);
    });
}

void handleRotateLeft(AsyncWebServerRequest *request) {
    //move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_left, ServoConfig::rotation_delay);
    request->send(200, "text/plain", "OK");
}

void handleRotateRight(AsyncWebServerRequest *request) {
    //move_servo(ServoConfig::pan_servo_pin, ServoConfig::rotate_right, ServoConfig::rotation_delay);
    request->send(200, "text/plain", "OK");
}

void handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

}  // namespace

void asyncWebServerSetup() {
    server.on("/stream", HTTP_GET, handleStream);
    server.on("/toggle_led", HTTP_GET, handleToggleLED);
    server.on("/rotate_left", HTTP_GET, handleRotateLeft);
    server.on("/rotate_right", HTTP_GET, handleRotateRight);
    registerCameraSettingRoutes("brightness", CameraSetting::Brightness);
    registerCameraSettingRoutes("contrast", CameraSetting::Contrast);
    registerCameraSettingRoutes("saturation", CameraSetting::Saturation);
    registerCameraSettingRoutes("sharpness", CameraSetting::Sharpness);
    registerCameraSettingRoutes("quality", CameraSetting::Quality);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Asynchroniczny serwer uruchomiony");
}
