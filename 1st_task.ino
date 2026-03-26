#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ==========================
// Wi-Fi credentials
// ==========================
const char* ssid = "KK-Network";
const char* password = "network1";

// ==========================
// MQTT broker settings
// ==========================
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

const char* mqttPa

// ==========================
// MQTT topics
// ==========================
const char* topicSubscribe = "nit/buzzer/14";

// ==========================
// Globals
// ==========================
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;
const unsigned long publishInterval = 5000;

// NodeMCU built-in LED is usually on GPIO2 (D4)
// and is ACTIVE LOW
const int buzzerPin = LED_BUILTIN;

bool ledState = off;
int counter = 0;

// ==========================
// Wi-Fi connect
// ==========================
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ==========================
// MQTT callback
// ==========================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.print("Raw payload: ");
  Serial.println(message);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* state = doc["command"] | "";
  int value = doc["value"] | 0;
  const char* device = doc["device"] | "unknown";

  Serial.print("device: ");
  Serial.println(device);

  Serial.print("command: ");
  Serial.println(command);

  Serial.print("value: ");
  Serial.println(value);

  if (strcmp(command, "led_on") == 0) {
    ledState = true;
    digitalWrite(ledPin, LOW);   // active low
    Serial.println("LED turned ON");
  }
  else if (strcmp(command, "led_off") == 0) {
    ledState = false;
    digitalWrite(ledPin, HIGH);  // active low
    Serial.println("LED turned OFF");
  }
  else if (strcmp(command, "set_counter") == 0) {
    counter = value;
    Serial.print("Counter updated to: ");
    Serial.println(counter);
  }
}

// ==========================
// MQTT reconnect
// ==========================
void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    String clientId = "NodeMCU-";
    clientId += String(ESP.getChipId(), HEX);

    bool connected;
    if (strlen(mqttUser) > 0) {
      connected = client.connect(clientId.c_str(), mqttUser, mqttPassword);
    } else {
      connected = client.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("connected");
      client.subscribe(topicSubscribe);
      Serial.print("Subscribed to: ");
      Serial.println(topicSubscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 3 seconds");
      delay(3000);
    }
  }
}

// ==========================
// Publish JSON
// ==========================
void publishJsonMessage() {
  JsonDocument doc;

  doc["device"] = "nodemcu";
  doc["status"] = ledState ? "on" : "off";
  doc["counter"] = counter;
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  bool ok = client.publish(topicPublish, buffer, n);

  Serial.print("Publishing to ");
  Serial.println(topicPublish);
  Serial.print("Payload: ");
  Serial.println(buffer);

  if (ok) {
    Serial.println("Publish success");
  } else {
    Serial.println("Publish failed");
  }
}

// ==========================
// Setup
// ==========================
void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED OFF initially (active low)

  connectWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

// ==========================
// Loop
// ==========================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectWiFi();
  }

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  if (millis() - lastPublish >= publishInterval) {
    lastPublish = millis();
    counter++;
    publishJsonMessage();
  }
}
