#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "camera_handler.h"
#include <WebServer.h>

WebServer server(80);

void handleStream() {
    WiFiClient client = server.client();
    client.setTimeout(2);
    
    String head = "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Access-Control-Allow-Origin: *\r\n\r\n";
    client.print(head);
    
    Serial.println("Rozpoczęto przesyłanie strumienia wideo do klienta: " + client.remoteIP().toString());

    while (client.connected()) {
        camera_fb_t * fb = esp_camera_fb_get();
        if (!fb) {
            break;
        }
        
        if (fb->format == PIXFORMAT_JPEG) {
            String header = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fb->len) + "\r\n\r\n";
            if (client.print(header) == 0) { 
                // poszlo cos nie tak z wysylaniem naglowka - przerwij stream
                esp_camera_fb_return(fb);
                break; 
            }
            client.write(fb->buf, fb->len);
            client.print("\r\n");
        }
        
        esp_camera_fb_return(fb);
        delay(30); // ograniczenie fps 
    }
    Serial.println("Zakończono przesyłanie strumienia");
    client.stop();
}

void setup() {
    Serial.begin(115200);
    delay(1000); 
    
    SprawdzPSRAM();
    
    // Inicjalizacja kamery
    if (!initCamera()) {
        Serial.println("Nie udalo sie zainicjalizowac kamery");
        while (true) { delay(1000); }
    }
    
    // Połączenie Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WifiConfig::ssid, WifiConfig::password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Połączone. IP: " + WiFi.localIP().toString()); // to ip jest bardzo wazne bo go2rtc potrzebuje go do przekazania streama
    
    server.on("/stream", handleStream);
    server.begin();
}

void loop() {
    server.handleClient();
}

