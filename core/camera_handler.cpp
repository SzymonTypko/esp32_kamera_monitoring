#include "camera_handler.h"

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