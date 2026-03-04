/*
 * Wasabi SmartFarm - MQTT 핸들러 구현 (대기+토양 통합 센서 노드용)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#include "mqtt_handler.h"

// ============================================
// 메모리 체크 함수 (Arduino Uno R4 WiFi용)
// ============================================
int freeMemory() {
  return 0;  // ARM 아키텍처에서는 정확한 측정 불가
}

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler() 
  : _mqttClient(_wifiClient), _lastReconnectAttempt(0), _reconnectAttempts(0) {
  
  // MQTT 클라이언트 ID 생성
  _clientId = String(MQTT_CLIENT_ID_PREFIX) + String(NODE_ID);
  
  // MQTT 토픽 생성
  _airDataTopic = String(MQTT_TOPIC_AIR_DATA);
  _soilDataTopic = String(MQTT_TOPIC_SOIL_DATA);
  _heartbeatTopic = String(MQTT_TOPIC_HEARTBEAT);
  _statusTopic = String(MQTT_TOPIC_STATUS);
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

#if WIFI_USE_STATIC_IP
  IPAddress localIp(WIFI_STATIC_IP_A, WIFI_STATIC_IP_B, WIFI_STATIC_IP_C, WIFI_STATIC_IP_D);
  IPAddress gateway(WIFI_GATEWAY_A, WIFI_GATEWAY_B, WIFI_GATEWAY_C, WIFI_GATEWAY_D);
  IPAddress subnet(WIFI_SUBNET_A, WIFI_SUBNET_B, WIFI_SUBNET_C, WIFI_SUBNET_D);
  IPAddress dns(WIFI_DNS_A, WIFI_DNS_B, WIFI_DNS_C, WIFI_DNS_D);
  WiFi.config(localIp, dns, gateway, subnet);
#endif
  
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
  DEBUG_PRINTLN(_clientId);
  
  // MQTT 연결 시도
  bool connected = false;
  
  if (strlen(MQTT_USER) > 0) {
    connected = _mqttClient.connect(_clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
  } else {
    connected = _mqttClient.connect(_clientId.c_str());
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

bool MQTTHandler::isMQTTConnected() {
  return isConnected();
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
// 대기 센서 데이터 전송
// ============================================
bool MQTTHandler::publishAirData(const AirSensorData &data) {
  if (!_mqttClient.connected()) {
    DEBUG_PRINTLN(F("[MQTT] Not connected. Cannot publish air data"));
    return false;
  }
  
  if (!data.is_valid) {
    DEBUG_PRINTLN(F("[MQTT] Invalid air sensor data. Not publishing"));
    return false;
  }
  
  // JSON 생성
  StaticJsonDocument<256> doc;
  doc["zone_id"] = ZONE_ID;
  doc["air_temp"] = round(data.air_temp * 10) / 10.0;
  doc["air_humidity"] = round(data.air_humidity * 10) / 10.0;
  doc["timestamp"] = data.timestamp;
  
  // JSON 직렬화
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  DEBUG_PRINT(F("[MQTT] Publishing air data to: "));
  DEBUG_PRINTLN(_airDataTopic);
  DEBUG_PRINT(F("[MQTT] Payload: "));
  DEBUG_PRINTLN(jsonBuffer);
  
  bool result = _mqttClient.publish(_airDataTopic.c_str(), jsonBuffer, len);
  
  if (!result) {
    DEBUG_PRINTLN(F("[MQTT] Failed to publish air data"));
  }
  
  return result;
}

// ============================================
// 토양 센서 데이터 전송
// ============================================
bool MQTTHandler::publishSoilData(const SoilSensorData &data) {
  if (!_mqttClient.connected()) {
    DEBUG_PRINTLN(F("[MQTT] Not connected. Cannot publish soil data"));
    return false;
  }
  
  if (!data.valid) {
    DEBUG_PRINTLN(F("[MQTT] Invalid soil sensor data. Not publishing"));
    return false;
  }
  
  // JSON 생성
  StaticJsonDocument<256> doc;
  doc["tank_id"] = TANK_ID;
  doc["soil_temp"] = round(data.soil_temp * 10) / 10.0;
  doc["soil_moisture"] = round(data.soil_moisture * 10) / 10.0;
  doc["soil_ec"] = round(data.soil_ec * 10) / 10.0;
  doc["soil_ph"] = round(data.soil_ph * 100) / 100.0;
  doc["timestamp"] = data.timestamp;
  
  // JSON 직렬화
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  DEBUG_PRINT(F("[MQTT] Publishing soil data to: "));
  DEBUG_PRINTLN(_soilDataTopic);
  DEBUG_PRINT(F("[MQTT] Payload: "));
  DEBUG_PRINTLN(jsonBuffer);
  
  bool result = _mqttClient.publish(_soilDataTopic.c_str(), jsonBuffer, len);
  
  if (!result) {
    DEBUG_PRINTLN(F("[MQTT] Failed to publish soil data"));
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
  doc["zone_id"] = ZONE_ID;
  doc["tank_id"] = TANK_ID;
  doc["node_type"] = "combined";
  doc["status"] = "alive";
  doc["uptime"] = millis();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_memory"] = freeMemory();
  doc["timestamp"] = millis();
  
  // JSON 직렬화
  char jsonBuffer[256];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  DEBUG_PRINT(F("[MQTT] Publishing heartbeat to: "));
  DEBUG_PRINTLN(_heartbeatTopic);
  
  bool result = _mqttClient.publish(_heartbeatTopic.c_str(), jsonBuffer, len);
  
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
  doc["zone_id"] = ZONE_ID;
  doc["tank_id"] = TANK_ID;
  doc["status"] = message;
  doc["timestamp"] = millis();
  
  // JSON 직렬화
  char jsonBuffer[128];
  size_t len = serializeJson(doc, jsonBuffer);
  
  // MQTT 전송
  return _mqttClient.publish(_statusTopic.c_str(), jsonBuffer, len);
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
