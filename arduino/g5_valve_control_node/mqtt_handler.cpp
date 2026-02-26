/*
 * Wasabi SmartFarm - MQTT 핸들러 구현 (밸브 제어 노드용)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "mqtt_handler.h"

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler(ValveControl* valveControl) 
  : _mqttClient(_wifiClient), _lastReconnectAttempt(0), _valveControl(valveControl) {
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
  
  // MQTT 콜백 등록
  _mqttClient.setCallback(mqttCallback);
  
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
  
  // MQTT 명령 토픽 구독
  if (_mqttClient.connect(_clientId.c_str())) {
    DEBUG_PRINTLN(F("[MQTT] Connected to broker!"));
    
    // 명령 토픽 구독
    _mqttClient.subscribe(MQTT_TOPIC_CMD);
    DEBUG_PRINT(F("[MQTT] Subscribed to: "));
    DEBUG_PRINTLN(MQTT_TOPIC_CMD);
    
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
// MQTT 콜백
// ============================================
void MQTTHandler::mqttCallback(char* topic, byte* payload, unsigned int length) {
  // 문자열 종료
  payload[length] = '\0';
  String command = String((char*)payload);
  
  DEBUG_PRINT(F("[MQTT] Received on topic: "));
  DEBUG_PRINTLN(topic);
  DEBUG_PRINT(F("[MQTT] Command: "));
  DEBUG_PRINTLN(command);
  
  // 명령 처리 (밸브 제어)
  if (_valveControl != nullptr) {
    _valveControl->handleCommand(command.c_str());
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
// 상태 전송
// ============================================
bool MQTTHandler::publishState() {
  if (!isConnected()) {
    return false;
  }
  
  if (_valveControl == nullptr) {
    return false;
  }
  
  // JSON 문서 생성
  StaticJsonDocument<256> doc;
  
  ValveState state = _valveControl->getState();
  doc["node_id"] = NODE_ID;
  doc["valve"] = state.valve_open ? "on" : "off";
  
  char payload[256];
  serializeJson(doc, payload);
  
  bool success = _mqttClient.publish(MQTT_TOPIC_STATE, payload);
  
  if (success) {
    DEBUG_PRINTLN(F("[MQTT] Valve state published"));
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
