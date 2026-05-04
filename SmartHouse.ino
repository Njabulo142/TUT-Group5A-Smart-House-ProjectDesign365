/*
 * GROUP5A SMART HOUSE - TUT 2025
 * Module: Project Design 365 - Programming for IoT
 * Author: Njabulo Msibi
 *
 * AIM: Create a smart homing system that uses Wi-Fi and can control appliances remotely
 * SCHEMATIC: ESP32 + 8-Channel Relay controlling Plugs, Lights, Stove, Heater
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// === HARDWARE CONFIG - MATCHES SCHEMATIC ===
// Schematic shows 8-Channel Relay Module connected to ESP32
const int relayPins[8] = {32, 33, 25, 26, 27, 14, 12, 13}; // GPIO pins to IN1-IN8 on relay
const char* relayLabels[8] = {
  "Plugs", // Relay 1 - matches "PLUGS" in schematic
  "Lights", // Relay 2 - matches "LIGHTS" in schematic
  "Stove", // Relay 3 - matches "STOVE" in schematic - high power load
  "Heater", // Relay 4 - matches "HEATER" in schematic - high power load
  "Spare 5", // Relay 5 - unused channel for future expansion
  "Spare 6", // Relay 6 - unused channel
  "Spare 7", // Relay 7 - unused channel
  "Spare 8" // Relay 8 - unused channel
};
// CONCLUSION: "Proper wiring and protection ensure safe operation" - critical for Stove/Heater
bool relayStates[8] = {false, false, false, false};
bool timerActive[8] = {false, false, false, false};

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // SAFETY: Initialize all relays to OFF - critical for Stove/Heater loads
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // HIGH = OFF for active-low relays
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status()!= WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nSmart House Connected! IP: " + WiFi.localIP().toString());

  // Web routes - "ESP32 acts as both the server and controller"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateHTML());
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("relay")) {
      int relay = request->getParam("relay")->value().toInt();
      if (relay >= 0 && relay < 8) {
        toggleRelay(relay); // "Relays acting as switches based on user commands"
      }
    }
    request->redirect("/");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("relay")) {
      int relay = request->getParam("relay")->value().toInt();
      if (relay >= 0 && relay < 8) {
        relayStates[relay] = false;
        digitalWrite(relayPins[relay], HIGH); // Cut power - critical for safety
        timerActive[relay] = false;
        Serial.println("Safety OFF: " + String(relayLabels[relay]));
      }
    }
    request->redirect("/");
  });

  server.begin();
}

void loop() {
  // Async server handles clients - "low-cost design" objective
}

void toggleRelay(int relay) {
  relayStates[relay] =!relayStates[relay];
  digitalWrite(relayPins[relay], relayStates[relay]? LOW : HIGH);
  Serial.println(String(relayLabels[relay]) + " toggled " + (relayStates[relay]? "ON" : "OFF"));
}

// SIMPLE WEB PAGE: Shows Plugs, Lights, Stove, Heater - matches schematic
String generateHTML() {
  String html = "<html><head><title>Group 5A Smart House</title></head><body>";
  html += "<h1>Group 5A Smart House</h1>";
  html += "<h3>TUT 2025 - Project Design 365</h3>";
  html += "<p><b>Connected Devices:</b></p>";

  for (int i = 0; i < 8; i++) {
    html += "<p><a href='/toggle?relay=" + String(i) + "'>";
    html += "<button>" + String(relayLabels[i]) + " - " + String(relayStates[i]? "ON" : "OFF");
    html += "</button></a></p>";
  }

  html += "<p><small>ESP32 8-Channel Relay Control</small></p>";
  html += "</body></html>";
  return html;
}