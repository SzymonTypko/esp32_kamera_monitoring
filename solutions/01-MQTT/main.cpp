  #include <Arduino.h>

  void setup() {
    Serial.begin(115200);
    delay(2000); 
    Serial.println("Sprawdzanie dostępności PSRAM...");
    if (psramFound()) {
      Serial.println("PSRAM została pomyślnie wykryta!");
      Serial.printf("Całkowity rozmiar PSRAM: %d bajtów\n", ESP.getPsramSize());
      Serial.printf("Wolny PSRAM: %d bajtów\n", ESP.getFreePsram());
    } else {
      Serial.println("BŁĄD: PSRAM nie jest dostępna. Sprawdź konfigurację platformio.ini!");
    }

    pinMode(LED_BUILTIN, OUTPUT);
  }

  void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

