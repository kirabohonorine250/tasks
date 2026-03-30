#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

// ===== WiFi credentials =====
const char* ssid = "mancity_26";
const char* password = "inyoniiguruka";

// ===== PHP API URL =====
const char* serverURL = "http://yourserver/api/insert.php"; // replace with your server URL

// ===== DHT sensor setup =====
#define DHTPIN 4          // GPIO where DHT is connected
#define DHTTYPE DHT22     // DHT22 or DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== Transmission interval =====
const unsigned long interval = 10000; // 10 seconds
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // 1. Read sensor values
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // 2. Validate readings
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // 3. Format as JSON
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    String jsonData;
    serializeJson(jsonDoc, jsonData);

    // 4. Send HTTP POST
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverURL);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(jsonData);
      String response = http.getString();

      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Server response: " + response);

      http.end();
    } else {
      Serial.println("WiFi disconnected!");
    }
  }
}
