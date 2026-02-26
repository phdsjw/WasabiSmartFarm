/*
 * Wasabi SmartFarm - MQTT 핸들러 구현
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "mqtt_handler.h"

// 싱글톤 인스턴스 초기화
MQTTHandler* MQTTHandler::instance = nullptr;

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler(ActuatorControl* actuatorControl) 
  : _mqttClient(_wifiClient), 
    _actuatorControl(actuatorControl),
    _lastReconnectAttempt(0) {
  
  instance = this;  // 싱글톤 설정
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
  _mqttClient.setCallback(mqttCallback);
  
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
  DEBUG_PRINTLN(MQTT_CLIENT_ID);
  
  // MQTT 연결 시도
  if (_mqttClient.connect(MQTT_CLIENT_ID)) {
    DEBUG_PRINTLN(F("[MQTT] Connected to broker!"));
    
    // 명령 토픽 구독
    DEBUG_PRINTLN(F("[MQTT] Subscribing to command topics..."));
    _mqttClient.subscribe(MQTT_TOPIC_IRRIGATION);
    _mqttClient.subscribe(MQTT_TOPIC_DRAINAGE);
    _mqttClient.subscribe(MQTT_TOPIC_FAN);
    _mqttClient.subscribe(MQTT_TOPIC_LED);
    _mqttClient.subscribe(MQTT_TOPIC_EMERGENCY_STOP);
    _mqttClient.subscribe(MQTT_TOPIC_EMERGENCY_RELEASE);
    _mqttClient.subscribe(MQTT_TOPIC_RESET);
    
    DEBUG_PRINTLN(F("[MQTT] Subscribed to all command topics"));
    
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
// MQTT 콜백 (정적)
// ============================================
void MQTTHandler::mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (instance == nullptr) return;
  
  // 메시지 변환
  char message[256];
  if (length >= sizeof(message)) {
    length = sizeof(message) - 1;
  }
  memcpy(message, payload, length);
  message[length] = '\0';
  
  DEBUG_PRINT(F("[MQTT] Message received: "));
  DEBUG_PRINT(topic);
  DEBUG_PRINT(F(" -> "));
  DEBUG_PRINTLN(message);
  
  // 명령 처리
  instance->handleCommand(topic, message);
}

// ============================================
// 명령 처리
// ============================================
void MQTTHandler::handleCommand(const char* topic, const char* message) {
  // 관수 펌프
  if (strcmp(topic, MQTT_TOPIC_IRRIGATION) == 0) {
    if (strcmp(message, "off") == 0) {
      if (_actuatorControl->stopIrrigationPump()) {
        publishStatus("irrigation_pump_stopped");
      }
    } else {
      if (_actuatorControl->startIrrigationPump()) {
        publishStatus("irrigation_pump_started");
      } else {
        publishStatus("irrigation_pump_start_failed");
      }
    }
  }
  // 배수 펌프
  else if (strcmp(topic, MQTT_TOPIC_DRAINAGE) == 0) {
    if (strcmp(message, "off") == 0) {
      if (_actuatorControl->stopDrainagePump()) {
        publishStatus("drainage_pump_stopped");
      }
    } else {
      if (_actuatorControl->startDrainagePump()) {
        publishStatus("drainage_pump_started");
      } else {
        publishStatus("drainage_pump_start_failed");
      }
    }
  }
  // 팬
  else if (strcmp(topic, MQTT_TOPIC_FAN) == 0) {
    if (strcmp(message, "off") == 0) {
      if (_actuatorControl->stopFan()) {
        publishStatus("fan_stopped");
      }
    } else {
      if (_actuatorControl->startFan()) {
        publishStatus("fan_started");
      }
    }
  }
  // LED
  else if (strcmp(topic, MQTT_TOPIC_LED) == 0) {
    if (strcmp(message, "off") == 0) {
      if (_actuatorControl->stopLED()) {
        publishStatus("led_stopped");
      }
    } else {
      if (_actuatorControl->startLED()) {
        publishStatus("led_started");
      }
    }
  }
  // 긴급 정지
  else if (strcmp(topic, MQTT_TOPIC_EMERGENCY_STOP) == 0) {
    _actuatorControl->emergencyStop();
    publishStatus("emergency_stop_activated");
  }
  // 긴급 정지 해제
  else if (strcmp(topic, MQTT_TOPIC_EMERGENCY_RELEASE) == 0) {
    DEBUG_PRINTLN(F("[MQTT] Emergency release command received"));
    _actuatorControl->resetEmergencyStop();
    publishStatus("emergency_stop_released");
    DEBUG_PRINTLN(F("[ACTUATOR] ✅ 긴급 정지 해제 - 시스템 대기 상태"));
  }
  // 리셋 (하위 호환성 유지)
  else if (strcmp(topic, MQTT_TOPIC_RESET) == 0) {
    _actuatorControl->resetEmergencyStop();
    publishStatus("emergency_stop_reset");
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
bool MQTTHandler::isMQTTConnected() {
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
  if (!isMQTTConnected()) {
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
// 상태 리포트 전송
// ============================================
bool MQTTHandler::publishStateReport(const ActuatorState &state) {
  if (!isMQTTConnected()) {
    return false;
  }
  
  // JSON 문서 생성 (512바이트)
  StaticJsonDocument<512> doc;
  
  doc["irrigation_pump"] = state.irrigation_pump;
  doc["drainage_pump"] = state.drainage_pump;
  doc["fan"] = state.fan;
  doc["led"] = state.led;
  doc["emergency_stop"] = state.emergency_stop;
  
  // 통계
  doc["total_irrigation_time"] = state.total_irrigation_time / 1000;  // 초 단위
  doc["total_drainage_time"] = state.total_drainage_time / 1000;
  doc["irrigation_count"] = state.irrigation_count;
  doc["drainage_count"] = state.drainage_count;
  
  doc["uptime"] = millis();
  doc["rssi"] = WiFi.RSSI();
  
  // JSON 직렬화
  char payload[512];
  serializeJson(doc, payload);
  
  // MQTT 전송
  return _mqttClient.publish(MQTT_TOPIC_STATE, payload);
}

// ============================================
// 하트비트 전송
// ============================================
bool MQTTHandler::publishHeartbeat(const ActuatorState &state) {
  if (!isMQTTConnected()) {
    return false;
  }
  
  // JSON 문서 생성
  StaticJsonDocument<256> doc;
  
  doc["status"] = "alive";
  doc["emergency_stop"] = state.emergency_stop;
  doc["active_count"] = (int)state.irrigation_pump + (int)state.drainage_pump + 
                        (int)state.fan + (int)state.led;
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
// 상태 메시지 전송
// ============================================
bool MQTTHandler::publishStatus(const char* message) {
  if (!isMQTTConnected()) {
    return false;
  }
  
  StaticJsonDocument<128> doc;
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
