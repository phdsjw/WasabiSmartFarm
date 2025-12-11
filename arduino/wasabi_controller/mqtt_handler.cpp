/*
 * MQTT Communication Implementation
 */

#include "mqtt_handler.h"

// ============================================
// 전역 객체 정의
// ============================================
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ============================================
// WiFi 연결
// ============================================
void connectWiFi() {
  DEBUG_PRINT(F("Connecting to WiFi: "));
  DEBUG_PRINTLN(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    DEBUG_PRINT(F("."));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN(F("\n[OK] WiFi connected!"));
    DEBUG_PRINT(F("IP Address: "));
    DEBUG_PRINTLN(WiFi.localIP());
  } else {
    DEBUG_PRINTLN(F("\n[ERROR] WiFi connection failed!"));
  }
}

// ============================================
// MQTT 설정
// ============================================
void setupMQTT() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);  // JSON 메시지를 위한 버퍼 크기 증가
}

// ============================================
// MQTT 연결
// ============================================
void connectMQTT() {
  while (!mqttClient.connected()) {
    DEBUG_PRINT(F("Connecting to MQTT Broker... "));
    
    String clientId = getClientId();
    
    // MQTT 연결 시도
    bool connected;
    if (strlen(MQTT_USER) > 0) {
      // 인증 사용
      connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
      // 인증 없음
      connected = mqttClient.connect(clientId.c_str());
    }
    
    if (connected) {
      DEBUG_PRINTLN(F("[OK] MQTT connected!"));
      
      // Subscribe to control topics (Step 2 이후 활성화)
      // mqttClient.subscribe("watering/+/+/on");
      // mqttClient.subscribe("watering/+/+/off");
      // mqttClient.subscribe("emergency/stop");
      
      // 연결 성공 메시지 발행
      publishHeartbeat();
      
    } else {
      DEBUG_PRINT(F("[ERROR] MQTT connection failed, rc="));
      DEBUG_PRINTLN(mqttClient.state());
      DEBUG_PRINTLN(F("Retrying in 5 seconds..."));
      delay(5000);
    }
  }
}

// ============================================
// MQTT 콜백 (Subscribe 메시지 수신)
// ============================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Step 1에서는 Subscribe 없음
  // Step 2 이후 제어 명령 처리
  
  DEBUG_PRINT(F("[MQTT] Message received: "));
  DEBUG_PRINTLN(topic);
  
  // payload를 문자열로 변환
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  DEBUG_PRINT(F("Payload: "));
  DEBUG_PRINTLN(message);
}

// ============================================
// 환경 센서 데이터 발행
// ============================================
void publishEnvironmentData(const SensorData& data) {
  // JSON 문서 생성 (512 바이트)
  StaticJsonDocument<512> doc;
  
  // 대기 환경
  if (data.air_valid) {
    doc["air_temp"] = data.air_temp;
    doc["air_humidity"] = data.air_humidity;
  }
  
  // 물탱크
  if (data.water_valid) {
    doc["water_temp"] = data.water_temp;
    doc["water_ph"] = data.water_ph;
    doc["water_tds"] = data.water_tds;
    doc["water_ec"] = data.water_ec;
  }
  
  // 토양 센서 (배열)
  JsonArray soil_temps = doc.createNestedArray("soil_temp");
  JsonArray soil_moistures = doc.createNestedArray("soil_moisture");
  JsonArray soil_ecs = doc.createNestedArray("soil_ec");
  JsonArray soil_phs = doc.createNestedArray("soil_ph");
  
  for (uint8_t i = 0; i < TANK_COUNT; i++) {
    if (data.soil_valid[i]) {
      soil_temps.add(data.soil_temp[i]);
      soil_moistures.add(data.soil_moisture[i]);
      soil_ecs.add(data.soil_ec[i]);
      soil_phs.add(data.soil_ph[i]);
    } else {
      soil_temps.add(-999.0);
      soil_moistures.add(-999.0);
      soil_ecs.add(-999.0);
      soil_phs.add(-999.0);
    }
  }
  
  // JSON 직렬화
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  
  // MQTT 발행
  if (mqttClient.publish(TOPIC_ENVIRONMENT, jsonBuffer)) {
    DEBUG_PRINTLN(F("[MQTT] Environment data published"));
    DEBUG_PRINTLN(jsonBuffer);
  } else {
    DEBUG_PRINTLN(F("[ERROR] Failed to publish environment data"));
  }
  
  // 수위 데이터 개별 발행
  for (uint8_t i = 1; i <= TANK_COUNT; i++) {
    int level = readWaterLevel(i);
    publishWaterLevel(i, level);
  }
}

// ============================================
// 수위 데이터 발행
// ============================================
void publishWaterLevel(uint8_t tankNum, int level) {
  char topic[50];
  snprintf(topic, sizeof(topic), TOPIC_WATER_LEVEL, tankNum);
  
  char payload[10];
  snprintf(payload, sizeof(payload), "%d", level);
  
  mqttClient.publish(topic, payload);
}

// ============================================
// 하트비트 발행
// ============================================
void publishHeartbeat() {
  StaticJsonDocument<128> doc;
  doc["client_id"] = MQTT_CLIENT_ID;
  doc["uptime"] = millis() / 1000;  // 초 단위
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_memory"] = freeMemory();  // 함수 구현 필요
  
  char jsonBuffer[128];
  serializeJson(doc, jsonBuffer);
  
  if (mqttClient.publish(TOPIC_HEARTBEAT, jsonBuffer)) {
    DEBUG_PRINTLN(F("[MQTT] Heartbeat sent"));
  }
}

// ============================================
// 유틸리티 함수
// ============================================
String getClientId() {
  // MAC 주소 기반 고유 ID 생성
  byte mac[6];
  WiFi.macAddress(mac);
  
  String clientId = String(MQTT_CLIENT_ID);
  clientId += "_";
  for (int i = 0; i < 6; i++) {
    clientId += String(mac[i], HEX);
  }
  
  return clientId;
}

// 사용 가능한 메모리 확인 (Arduino Uno R4)
int freeMemory() {
  // Uno R4는 32KB SRAM
  // 정확한 측정을 위해서는 추가 라이브러리 필요
  return 0;  // 임시
}
