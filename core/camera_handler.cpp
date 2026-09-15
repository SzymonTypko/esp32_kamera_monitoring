#include "camera_handler.h"

bool initCamera() {
  config_camera.ledc_channel = LEDC_CHANNEL_0;
  config_camera.ledc_timer = LEDC_TIMER_0;
  config_camera.pin_d0 = Y2_GPIO_NUM;
  config_camera.pin_d1 = Y3_GPIO_NUM;
  config_camera.pin_d2 = Y4_GPIO_NUM;
  config_camera.pin_d3 = Y5_GPIO_NUM;
  config_camera.pin_d4 = Y6_GPIO_NUM;
  config_camera.pin_d5 = Y7_GPIO_NUM;
  config_camera.pin_d6 = Y8_GPIO_NUM;
  config_camera.pin_d7 = Y9_GPIO_NUM;
  config_camera.pin_xclk = XCLK_GPIO_NUM;
  config_camera.pin_pclk = PCLK_GPIO_NUM;
  config_camera.pin_vsync = VSYNC_GPIO_NUM;
  config_camera.pin_href = HREF_GPIO_NUM;
  config_camera.pin_sccb_sda = SIOD_GPIO_NUM;
  config_camera.pin_sccb_scl = SIOC_GPIO_NUM;
  config_camera.pin_pwdn = PWDN_GPIO_NUM;
  config_camera.pin_reset = RESET_GPIO_NUM;
  config_camera.xclk_freq_hz = 20000000;
  config_camera.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config_camera.frame_size = static_cast<framesize_t>(CameraConfig::default_frame_size);
    config_camera.jpeg_quality = 10;  //0 - 63 nie zchodzic ponizej 10 bo bedzie za duzo danych do wyslania
    config_camera.fb_count = 2;
    config_camera.grab_mode = CAMERA_GRAB_LATEST;
    config_camera.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    config_camera.frame_size = static_cast<framesize_t>(CameraConfig::FRAMESIZE_QVGA);  // 320 x 240 bez PSRAM
    config_camera.jpeg_quality = 15;
    config_camera.fb_count = 1;
    config_camera.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config_camera.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config_camera);
  if (err != ESP_OK) {
    Serial.printf("[KAMERA] Błąd inicjalizacji: 0x%x\n", err);
    return false;
  }

  sensor_t* s = esp_camera_sensor_get();
  // domyslne ustawienia
  if (s != NULL) {
    if (s->id.PID == CameraConfig::OV3660_PID) {
      s->set_vflip(s, CameraConfig::vflip);
      s->set_brightness(s, 0);
      s->set_saturation(s, 0);
      s->set_contrast(s,0);
      s->set_sharpness(s,0);
    }
  }

  Serial.println("[KAMERA] Inicjalizacja udana");
  return true;
}

void SprawdzPSRAM(){
    Serial.println("Sprawdzanie dostępności PSRAM...");
    if (psramFound()) {
        Serial.println("PSRAM została pomyślnie wykryta!");
        Serial.printf("Całkowity rozmiar PSRAM: %d bajtów\n", ESP.getPsramSize());
        Serial.printf("Wolny PSRAM: %d bajtów\n", ESP.getFreePsram());
    } else {
        Serial.println("BŁĄD: PSRAM nie jest dostępna. Sprawdź konfigurację platformio.ini!");
    }
}