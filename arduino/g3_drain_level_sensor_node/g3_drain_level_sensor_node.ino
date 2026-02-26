/*
 * Wasabi SmartFarm - 배수수위 센서 노드
 * 
 * 기능:
 * - HC-SR04 초음파 센서로 배수糟 수위 측정
 * - WiFi 연결
 * - MQTT를 통한 센서 데이터 전송
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - HC-SR04 초음파 센서
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "config.h"
#include "hcsr04_sensor.h"
#include "mqtt_handler.h"

// 전역 객체
HCSR04Sensor waterLevelSensor;
MQTTHandler mqttHandler;

// 타이머 변수
unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastLedBlink = 0;

// LED 상태
bool ledState = false;

// ============================================
// 초기 설정
// ============================================
void setup() {
  // LED 핀 설정
  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, LOW);
  
  // 시리얼 통신 초기화
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial && millis() < 3000);  // 3초 대기
  
  // 시작 메시지
  printStartupMessage();
  
  // HC-SR04 센서 초기화
  Serial.println(F("\n[SETUP] Initializing HC-SR04 sensor..."));
  waterLevelSensor.begin();
  Serial.println(F("[SETUP] HC-SR04 sensor initialized"));
  
  // MQTT 핸들러 초기화
  Serial.println(F("\n[SETUP] Initializing MQTT handler..."));
  if (!mqttHandler.begin()) {
    Serial.println(F("[ERROR] Failed to initialize MQTT handler!"));
    Serial.println(F("[ERROR] Please check:"));
    Serial.println(F("        - WiFi SSID/Password in config.h"));
    Serial.println(F("        - MQTT Broker IP/Port in config.h"));
    Serial.println(F("[SETUP] System will restart in 5 seconds..."));
    delay(5000);
    NVIC_SystemReset();  // 시스템 재시작
  }
  Serial.println(F("[SETUP] MQTT handler initialized successfully"));
  
  // 초기 상태 메시지
  mqttHandler.publishStatus("initialized");
  
  // 초기화 완료
  Serial.println(F("\n========================================"));
  Serial.println(F("  Drain Level Sensor Node Ready!"));
  Serial.println(F("  Node ID: " NODE_ID));
  Serial.println(F("========================================\n"));
  
  // LED 켜기 (정상 동작)
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN_PIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN_PIN, LOW);
    delay(200);
  }
  
  // 초기 타이머 설정
  lastSensorRead = millis();
  lastHeartbeat = millis();
  lastLedBlink = millis();
}

// ============================================
// 메인 루프
// ============================================
void loop() {
  unsigned long currentMillis = millis();
  
  // MQTT 연결 유지
  mqttHandler.loop();
  
  // 센서 데이터 읽기 및 전송
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    readAndPublishSensorData();
  }
  
  // 하트비트 전송
  if (currentMillis - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = currentMillis;
    mqttHandler.publishHeartbeat();
  }
  
  // LED 상태 표시
  if (currentMillis - lastLedBlink >= 1000) {
    lastLedBlink = currentMillis;
    
    if (mqttHandler.isConnected()) {
      // MQTT 연결됨: 느린 깜빡임
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
    } else {
      // MQTT 연결 안됨: 빠른 깜빡임
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
    }
  }
  
  delay(100);
}

// ============================================
// 센서 데이터 읽기 및 전송
// ============================================
void readAndPublishSensorData() {
  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINTLN(F("[SENSOR] Reading drain level sensor..."));
  
  // 수위 센서 데이터 읽기
  WaterLevelData data = waterLevelSensor.readWaterLevel();
  
  if (data.is_valid) {
    // 시리얼 출력
    Serial.println(F("[DRAIN LEVEL] Data:"));
    Serial.print(F("  Distance: "));
    Serial.print(data.distance_cm, 1);
    Serial.println(F(" cm"));
    Serial.print(F("  Water Level: "));
    Serial.print(data.water_level_percent, 1);
    Serial.println(F(" %"));
    
    // MQTT로 데이터 전송
    if (mqttHandler.publishWaterLevelData(data)) {
      Serial.println(F("[DRAIN LEVEL] Data published successfully"));
    } else {
      Serial.println(F("[DRAIN LEVEL] WARNING: Failed to publish data"));
    }
  } else {
    Serial.println(F("[DRAIN LEVEL] ERROR: Failed to read sensor data"));
    Serial.println(F("[INFO] Possible causes:"));
    Serial.println(F("  - Sensor not connected"));
    Serial.println(F("  - Distance out of range (2-400 cm)"));
    Serial.println(F("  - Timeout occurred"));
  }
  
  DEBUG_PRINTLN(F("========================================\n"));
}

// ============================================
// 시작 메시지 출력
// ============================================
void printStartupMessage() {
  Serial.println(F("\n"));
  Serial.println(F("========================================"));
  Serial.println(F("  Wasabi SmartFarm"));
  Serial.println(F("  Drain Level Sensor Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2026-02-25"));
  Serial.println(F("========================================"));
  Serial.print(F("  Node ID: "));
  Serial.println(NODE_ID);
  Serial.println(F("  MCU: Arduino Uno R4 WiFi"));
  Serial.println(F("  Sensor: HC-SR04 (Ultrasonic)"));
  Serial.print(F("  WiFi SSID: "));
  Serial.println(WIFI_SSID);
  Serial.print(F("  MQTT Broker: "));
  Serial.print(MQTT_SERVER);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);
  Serial.print(F("  Sensor Read Interval: "));
  Serial.print(SENSOR_READ_INTERVAL / 1000);
  Serial.println(F(" sec"));
  Serial.print(F("  Heartbeat Interval: "));
  Serial.print(HEARTBEAT_INTERVAL / 1000);
  Serial.println(F(" sec"));
  Serial.print(F("  Tank Height: "));
  Serial.print(TANK_HEIGHT_CM);
  Serial.println(F(" cm"));
  Serial.println(F("========================================\n"));
}
