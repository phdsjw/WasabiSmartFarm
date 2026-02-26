/*
 * Wasabi SmartFarm - MQTT 핸들러 구현 (수위 센서 노드용)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "mqtt_handler.h"

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler() 
  : _mqttClient(_wifiClient), _lastReconnectAttempt(0) {
  // MQTT 클라이언트 ID 생성
  _clientId = String(MQTT_CLIENT_ID_PREFIX) + String(NODE_ID);
}

// ============================================
// 초기화
// ============================================
bool MQTTHandler::begin() {
  DEBUG_PRINTLN(F("\n[MQTT] Initializing MQTT Handler..."));
  
  // WiFi 연결
  if (!connectWiFi()) {
    return false;
  }
  
  // MQTT 서버 설정
  _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  
  // MQTT 연결
  if (!connectMQTT()) {
    return false;
  }
  
  DEBUG_PRINTLN(F("[MQTT] MQTT Handler initialized successfully"));
  return true;
}

// ============================================
// WiFi 연결
// ============================================
bool MQTTHandler::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  
  DEBUG_PRINT(F("[WiFi] Connecting to: "));
  DEBUG_PRINTLN(WIFI_SSID);
  
  int retryCount = 0;
  
  while (retryCount < WIFI_MAX_RETRY) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > WIFI_TIMEOUT) {
        retryCount++;
        DEBUG_PRINT(F("\n[WiFi] Timeout. Retry "));
        DEBUG_PRINT(retryCount);
        DEBUG_PRINT(F("/"));
        DEBUG_PRINTLN(WIFI_MAX_RETRY);
        break;
      }
      delay(500);
      DEBUG_PRINT(F("."));
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PRINTLN(F("\n[WiFi] Connected!"));
      DEBUG_PRINT(F("[WiFi] IP Address: "));
      DEBUG_PRINTLN(WiFi.localIP());
      DEBUG_PRINT(F("[WiFi] RSSI: "));
      DEBUG_PRINT(WiFi.RSSI());
      DEBUG_PRINTLN(F(" dBm"));
      return true;
    }
    
    if (retryCount < WIFI_MAX_RETRY) {
      DEBUG_PRINT(F("[WiFi] Waiting "));
      DEBUG_PRINT(WIFI_RETRY_INTERVAL / 1000);
      DEBUG_PRINTLN(F("s before retry..."));
      delay(WIFI_RETRY_INTERVAL);
    }
  }
  
  DEBUG_PRINTLN(F("[WiFi] ERROR: Max retry reached. Continuing without WiFi..."));
  return false;
}

// ============================================
// MQTT 연결
// ============================================
bool MQTTHandler::connectMQTT() {
  if (_mqttClient.connected()) {
    return true;
  }
  
  DEBUG_PRINT(F("[MQTT] Connecting to broker: "));
  DEBUG_PRINT(MQTT_SERVER);
  DEBUG_PRINT(F(":"));
  DEBUG_PRINTLN(MQTT_PORT);
  DEBUG_PRINT(F("[MQTT] Client ID: "));
  DEBUG_PRINTLN(_clientId);
  
  // MQTT 연결 시도
  if (_mqttClient.connect(_clientId.c_str())) {
    DEBUG_PRINTLN(F("[MQTT] Connected to broker!"));
    
    // 연결 상태 메시지 전송
    publishStatus("online");
    
    return true;
  } else {
    DEBUG_PRINT(F("[MQTT] ERROR: Connection failed, rc="));
    DEBUG_PRINTLN(_mqttClient.state());
    return false;
  }
}

// ============================================
// WiFi 연결 상태 확인
// ============================================
bool MQTTHandler::isWiFiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

// ============================================
// MQTT 연결 상태 확인
// ============================================
bool MQTTHandler::isConnected() {
  return _mqttClient.connected();
}

// ============================================
// 연결 유지 및 재연결
// ============================================
void MQTTHandler::loop() {
  // WiFi 재연결
  if (!isWiFiConnected()) {
    DEBUG_PRINTLN(F("[WiFi] Connection lost. Reconnecting..."));
    connectWiFi();
  }
  
  // MQTT 재연결 (5초마다 시도)
  if (!isConnected()) {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt > 5000) {
      _lastReconnectAttempt = now;
      DEBUG_PRINTLN(F("[MQTT] Connection lost. Reconnecting..."));
      
      if (connectMQTT()) {
        _lastReconnectAttempt = 0;
      }
    }
  }
  
  // MQTT loop 처리
  _mqttClient.loop();
}

// ============================================
// 수위 데이터 전송
// ============================================
bool MQTTHandler::publishWaterLevelData(const WaterLevelData &data) {
  if (!isConnected()) {
    DEBUG_PRINTLN(F("[MQTT] ERROR: Not connected, cannot publish data"));
    return false;
  }
  
  // JSON 문서 생성
  StaticJsonDocument<512> doc;
  
  doc["node_id"] = NODE_ID;
  doc["distance_cm"] = round(data.distance_cm * 10.0) / 10.0;
  doc["water_level_percent"] = round(data.water_level_percent * 10.0) / 10.0;
  doc["timestamp"] = data.timestamp;
  doc["rssi"] = WiFi.RSSI();
  
  // JSON 직렬화
  char payload[512];
  size_t len = serializeJson(doc, payload);
  
  // MQTT 전송
  bool success = _mqttClient.publish(MQTT_TOPIC_DATA, payload, len);
  
  if (success) {
    DEBUG_PRINTLN(F("[MQTT] Water level data published:"));
    DEBUG_PRINT(F("  Topic: "));
    DEBUG_PRINTLN(MQTT_TOPIC_DATA);
    DEBUG_PRINT(F("  Payload: "));
    DEBUG_PRINTLN(payload);
  } else {
    DEBUG_PRINTLN(F("[MQTT] ERROR: Failed to publish water level data"));
  }
  
  return success;
}

// ============================================
// 하트비트 전송
// ============================================
bool MQTTHandler::publishHeartbeat() {
  if (!isConnected()) {
    return false;
  }
  
  // JSON 문서 생성
  StaticJsonDocument<256> doc;
  
  doc["node_id"] = NODE_ID;
  doc["status"] = "alive";
  doc["uptime"] = millis();
  doc["rssi"] = WiFi.RSSI();
  
  char payload[256];
  serializeJson(doc, payload);
  
  bool success = _mqttClient.publish(MQTT_TOPIC_HEARTBEAT, payload);
  
  if (success) {
    DEBUG_PRINTLN(F("[MQTT] Heartbeat sent"));
  }
  
  return success;
}

// ============================================
// 상태 정보 전송
// ============================================
bool MQTTHandler::publishStatus(const char* message) {
  if (!isConnected()) {
    return false;
  }
  
  StaticJsonDocument<128> doc;
  doc["node_id"] = NODE_ID;
  doc["status"] = message;
  doc["timestamp"] = millis();
  
  char payload[128];
  serializeJson(doc, payload);
  
  return _mqttClient.publish(MQTT_TOPIC_STATUS, payload);
}

// ============================================
// WiFi RSSI 가져오기
// ============================================
int MQTTHandler::getWiFiRSSI() {
  return WiFi.RSSI();
}
