/*
 * Wasabi SmartFarm Controller
 * Step 1: 기초 통신 인프라 구축 및 센서 데이터 수집
 * 
 * Hardware: Arduino Uno R4 WiFi
 * Author: Wasabi SmartFarm Team
 * Version: 1.0.0 - Step 1
 */

#include "config.h"
#include "sensors.h"
#include "mqtt_handler.h"

// 타이머 변수
unsigned long lastSensorRead = 0;
unsigned long lastWaterLevelRead = 0;
unsigned long lastHeartbeat = 0;

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // 3초 대기
  
  Serial.println(F("\n=== Wasabi SmartFarm Controller ==="));
  Serial.println(F("Step 1: Sensor Data Collection"));
  Serial.println(F("Version: 1.0.0"));
  Serial.println(F("===================================\n"));
  
  // WiFi 연결
  Serial.println(F("[SETUP] Connecting to WiFi..."));
  connectWiFi();
  
  // MQTT 연결
  Serial.println(F("[SETUP] Connecting to MQTT Broker..."));
  setupMQTT();
  connectMQTT();
  
  // 센서 초기화
  Serial.println(F("[SETUP] Initializing sensors..."));
  initSensors();
  
  Serial.println(F("[SETUP] Setup complete!\n"));
  
  // 초기 센서 읽기
  lastSensorRead = millis();
  lastWaterLevelRead = millis();
  lastHeartbeat = millis();
}

void loop() {
  // WiFi 연결 확인
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[ERROR] WiFi disconnected! Reconnecting..."));
    connectWiFi();
  }
  
  // MQTT 연결 확인 및 유지
  if (!mqttClient.connected()) {
    Serial.println(F("[ERROR] MQTT disconnected! Reconnecting..."));
    connectMQTT();
  }
  mqttClient.loop();
  
  // 환경 센서 데이터 수집 (10초마다)
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
    Serial.println(F("\n[SENSOR] Reading environment sensors..."));
    SensorData data = readEnvironmentSensors();
    publishEnvironmentData(data);
    lastSensorRead = millis();
  }
  
  // 수위 센서 데이터 수집 (3초마다)
  if (millis() - lastWaterLevelRead >= WATER_LEVEL_READ_INTERVAL) {
    Serial.println(F("[SENSOR] Reading water level sensors..."));
    readAndPublishWaterLevels();
    lastWaterLevelRead = millis();
  }
  
  // 하트비트 전송 (1분마다)
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    publishHeartbeat();
    lastHeartbeat = millis();
  }
  
  // 짧은 딜레이 (CPU 부하 감소)
  delay(100);
}
