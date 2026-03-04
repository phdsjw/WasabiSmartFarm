/*
 * Wasabi SmartFarm - MQTT 핸들러 구현 (ESP8266 수위 센서 노드용)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#include "mqtt_handler.h"

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler() 
  : _mqttClient(_wifiClient), _lastReconnectAttempt(0), _reconnectAttempts(0) {
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
  _mqttClient.setKeepAlive(60);
  _mqttClient.setSocketTimeout(10);
  
  // MQTT 연결
  if (!connectMQTT()) {
    return false;
  }
  
  DEBUG_PRINTLN(F("[MQTT] MQTT Handler initialized successfully"));
  return true;
}

// ============================================
// WiFi 연결 (개선된 재시도 로직)
// ============================================
bool MQTTHandler::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  
  DEBUG_PRINT(F("[WiFi] Connecting to: "));
  DEBUG_PRINTLN(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int retryCount = 0;
  
  while (WiFi.status() != WL_CONNECTED && retryCount < WIFI_MAX_RETRY) {
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
      DEBUG_PRINTLN(F(" seconds before retry..."));
      delay(WIFI_RETRY_INTERVAL);
    }
  }
  
  DEBUG_PRINTLN(F("[WiFi] Failed to connect after max retries"));
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
  DEBUG_PRINTLN(MQTT_CLIENT_ID);
  
  // MQTT 연결 시도
  bool connected = false;
  
  if (strlen(MQTT_USER) > 0) {
    connected = _mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
  } else {
    connected = _mqttClient.connect(MQTT_CLIENT_ID);
  }
  
  if (connected) {
    DEBUG_PRINTLN(F("[MQTT] Connected to broker"));
    _reconnectAttempts = 0;
    
    // 연결 성공 메시지 전송
    publishStatus("connected");
    
    return true;
  } else {
    DEBUG_PRINT(F("[MQTT] Connection failed, rc="));
    DEBUG_PRINTLN(_mqttClient.state());
    _reconnectAttempts++;
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
// 연결 유지 (loop에서 호출)
// ============================================
void MQTTHandler::loop() {
  // WiFi 재연결
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN(F("[WiFi] Connection lost. Reconnecting..."));
    if (!connectWiFi()) {
      DEBUG_PRINTLN(F("[WiFi] Reconnection failed"));
    }
  }
  
  // MQTT 재연결
  if (!_mqttClient.connected()) {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt > 5000) {  // 5초마다 재연결 시도
      _lastReconnectAttempt = now;
      
      DEBUG_PRINTLN(F("[MQTT] Connection lost. Reconnecting..."));
      if (connectMQTT()) {
        DEBUG_PRINTLN(F("[MQTT] Reconnected successfully"));
      }
    }
  }
  
  // MQTT 루프 처리
  _mqttClient.loop();
}

// ============================================
// 수위 센서 데이터 전송
// ============================================
bool MQTTHandler::publishWaterLevelData(const WaterLevelData &data) {
  if (!_mqttClient.connected()) {
    DEBUG_PRINTLN(F("[MQTT] Not connected. Cannot publish water level data"));
    return false;
  }
  
  if (!data.is_valid) {
    DEBUG_PRINTLN(F("[MQTT] Invalid water level data. Not publishing"));
    return false;
  }
  
  // JSON 생성
  StaticJsonDocument<256> doc;
  doc["node_id"] = NODE_ID;
  doc["distance_cm"] = round(data.distance_cm * 10) / 10.0;
  doc["water_level_percent"] = round(data.water_level_percent * 10) / 10.0;
  doc["timestamp"] = data.timestamp;
  
  // JSON 직렬화
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  DEBUG_PRINT(F("[MQTT] Publishing data to: "));
  DEBUG_PRINTLN(MQTT_DATA_TOPIC);
  DEBUG_PRINT(F("[MQTT] Payload: "));
  DEBUG_PRINTLN(jsonBuffer);
  
  bool result = _mqttClient.publish(MQTT_DATA_TOPIC, jsonBuffer, len);
  
  if (!result) {
    DEBUG_PRINTLN(F("[MQTT] Failed to publish data"));
  }
  
  return result;
}

// ============================================
// 하트비트 전송
// ============================================
bool MQTTHandler::publishHeartbeat() {
  if (!_mqttClient.connected()) {
    DEBUG_PRINTLN(F("[MQTT] Not connected. Cannot publish heartbeat"));
    return false;
  }
  
  // JSON 생성
  StaticJsonDocument<256> doc;
  doc["node_id"] = NODE_ID;
  doc["node_type"] = "water_level";
  doc["status"] = "alive";
  doc["uptime"] = millis();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["timestamp"] = millis();
  
  // JSON 직렬화
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  DEBUG_PRINT(F("[MQTT] Publishing heartbeat to: "));
  DEBUG_PRINTLN(MQTT_HEARTBEAT_TOPIC);
  
  bool result = _mqttClient.publish(MQTT_HEARTBEAT_TOPIC, jsonBuffer, len);
  
  if (result) {
    DEBUG_PRINTLN(F("[MQTT] Heartbeat sent successfully"));
  } else {
    DEBUG_PRINTLN(F("[MQTT] Failed to send heartbeat"));
  }
  
  return result;
}

// ============================================
// 상태 정보 전송
// ============================================
bool MQTTHandler::publishStatus(const char* message) {
  if (!_mqttClient.connected()) {
    return false;
  }
  
  // JSON 생성
  StaticJsonDocument<128> doc;
  doc["node_id"] = NODE_ID;
  doc["status"] = message;
  doc["timestamp"] = millis();
  
  // JSON 직렬화
  char jsonBuffer[128];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  return _mqttClient.publish(MQTT_STATUS_TOPIC, jsonBuffer, len);
}

// ============================================
// WiFi RSSI 가져오기
// ============================================
int MQTTHandler::getWiFiRSSI() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI();
  }
  return -999;  // 연결 안됨
}
