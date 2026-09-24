#include <Arduino.h>
#include <WebServer.h>
#include <husarnet.h>
#include "config.h"
#include "husarnet_handler.h"

HusarnetClient husarnet;
WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Dziala");
}

void husarnetSetup() {
  husarnet.join(HusarnetConfig::husarnet_host_name, HusarnetConfig::husarnet_join_token);
  Serial.println("[HUSAR] Łączenie z siecią Husarnet...");
  while(!husarnet.isJoined()) {
    Serial.println(".");
    delay(1000);
  }
  Serial.println("[HUSAR] Połączono z Husarnetem");

  Serial.print("[HUSAR] Husarnet IP: ");
  Serial.println(husarnet.getIpAddress().c_str());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("[HUSAR] HTTP server started");
}

void husarnetLoop() {
  server.handleClient();
}
