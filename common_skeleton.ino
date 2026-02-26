#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define MQTT_SERVER     "192.168.0.100"
#define MQTT_PORT       1883

#define NODE_ID         "g2-01"
#define TOPIC_BASE      "smartfarm/wasabi/g2_soil/" NODE_ID

#define HEARTBEAT_INTERVAL 60000UL
#define RECONNECT_INTERVAL 5000UL

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastHeartbeat = 0;
unsigned long lastReconnectAttempt = 0;

void publishStatus(const char *status) {
  StaticJsonDocument<128> doc;
  doc["node_id"] = NODE_ID;
  doc["status"] = status;
  doc["timestamp"] = millis();
  char payload[128];
  serializeJson(doc, payload);
  mqttClient.publish(TOPIC_BASE "/status", payload, true);
}

void publishHeartbeat() {
  StaticJsonDocument<128> doc;
  doc["node_id"] = NODE_ID;
  doc["status"] = "alive";
  doc["uptime"] = millis();
  doc["rssi"] = WiFi.RSSI();
  char payload[128];
  serializeJson(doc, payload);
  mqttClient.publish(TOPIC_BASE "/heartbeat", payload);
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000UL) {
    delay(500);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool connectMQTT() {
  if (mqttClient.connected()) return true;
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  if (mqttClient.connect(NODE_ID)) {
    publishStatus("online");
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  connectMQTT();
  publishStatus("initialized");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      connectMQTT();
    }
  } else {
    mqttClient.loop();
  }

  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    publishHeartbeat();
  }

  // Sensor read + publish here
  // mqttClient.publish(TOPIC_BASE "/data", payload);
}
