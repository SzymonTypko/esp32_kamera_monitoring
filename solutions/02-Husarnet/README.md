# Husarnet - szkic rozwiązania

Ten katalog zawiera szkic integracji kamery ESP32-S3 z siecią Husarnet.
Kod jest przygotowany i powinien mieć docelowo strukturę zgodną z PlatformIO, ale aktualna wersja biblioteki Husarnet nie buduje się poprawnie w tym środowisku.

## Dlaczego build obecnie nie działa

Biblioteka:

```ini
husarnet/Husarnet ESP32 @ 2.0.0-beta6
```

wymaga Arduino ESP32 Core w wersji co najmniej `3.0.3`. Przy standardowej platformie PlatformIO projekt używał starszego Core i kompilacja zatrzymywała się na:

```text
This library requires ESP32 Arduino Core version 3.0.3 or newer
```

Po wymuszeniu Arduino Core `3.0.5` warunek wersji został spełniony, ale build zatrzymał się później na etapie linkowania. Pojawiły się między innymi błędy:

```text
undefined reference to `__dso_handle'
undefined reference to `__atomic_fetch_add_4'
undefined reference to `std::__cxx11::basic_string...'
undefined reference to `getentropy'
```

Na ten moment zostawie zarys kodu który działałby najpewniej na PlatformIO i dodam plik ???.ino który będzie zawierał kod który builduje się w ArduinoIDE.