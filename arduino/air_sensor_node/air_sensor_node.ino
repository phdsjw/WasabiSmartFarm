/*
 * Wasabi SmartFarm - 대기 센서 노드
 * 
 * 기능:
 * - SHT30 온습도 센서 데이터 읽기
 * - WiFi 연결
 * - MQTT를 통한 센서 데이터 전송 (10초 주기)
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - SHT30 온습도 센서 (I2C)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "config.h"
#include "sht30_sensor.h"
#include "mqtt_handler.h"

// 전역 객체
SHT30Sensor airSensor;
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
  
  // SHT30 센서 초기화
  Serial.println(F("\n[SETUP] Initializing SHT30 sensor..."));
  if (!airSensor.begin()) {
    Serial.println(F("[ERROR] Failed to initialize SHT30 sensor!"));
    Serial.println(F("[ERROR] Please check I2C connections:"));
    Serial.println(F("        - SDA: A4"));
    Serial.println(F("        - SCL: A5"));
    Serial.println(F("        - VCC: 3.3V or 5V"));
    Serial.println(F("        - GND: GND"));
    
    // 오류 표시 (빠른 깜빡임)
    while (true) {
      digitalWrite(LED_BUILTIN_PIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN_PIN, LOW);
      delay(100);
    }
  }
  Serial.println(F("[SETUP] SHT30 sensor initialized successfully"));
  
  // MQTT 핸들러 초기화
  Serial.println(F("\n[SETUP] Initializing MQTT handler..."));
  if (!mqttHandler.begin()) {
    Serial.println(F("[ERROR] Failed to initialize MQTT handler!"));
    Serial.println(F("[ERROR] Please check:"));
    Serial.println(F("        - WiFi SSID/Password in config.h"));
    Serial.println(F("        - MQTT Broker IP/Port in config.h"));
    
    // 오류 표시 (느린 깜빡임)
    while (true) {
      digitalWrite(LED_BUILTIN_PIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN_PIN, LOW);
      delay(500);
    }
  }
  Serial.println(F("[SETUP] MQTT handler initialized successfully"));
  
  // 초기 상태 메시지
  mqttHandler.publishStatus("initialized");
  
  // 초기화 완료
  Serial.println(F("\n========================================"));
  Serial.println(F("  Air Sensor Node Ready!"));
  Serial.println(F("  Zone: " ZONE_ID));
  Serial.println(F("========================================\n"));
  
  // LED 켜기 (정상 동작)
  digitalWrite(LED_BUILTIN_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN_PIN, LOW);
}

// ============================================
// 메인 루프
// ============================================
void loop() {
  unsigned long currentMillis = millis();
  
  // MQTT 연결 유지
  mqttHandler.loop();
  
  // 센서 데이터 읽기 및 전송 (10초 주기)
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    
    readAndPublishSensorData();
  }
  
  // 하트비트 전송 (60초 주기)
  if (currentMillis - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = currentMillis;
    
    mqttHandler.publishHeartbeat();
  }
  
  // LED 상태 표시 (1초 주기로 깜빡임)
  if (currentMillis - lastLedBlink >= 1000) {
    lastLedBlink = currentMillis;
    
    ledState = !ledState;
    digitalWrite(LED_BUILTIN_PIN, ledState);
  }
  
  // 짧은 대기
  delay(10);
}

// ============================================
// 센서 데이터 읽기 및 전송
// ============================================
void readAndPublishSensorData() {
  DEBUG_PRINTLN(F("\n--- Reading Air Sensor Data ---"));
  
  // 센서 데이터 구조체
  AirSensorData data;
  data.timestamp = millis();
  data.is_valid = false;
  
  // SHT30 센서에서 온습도 읽기
  if (airSensor.readTempHumidity(data.air_temp, data.air_humidity)) {
    data.is_valid = true;
    
    // 시리얼 출력
    Serial.println(F("[SENSOR] Air sensor data:"));
    Serial.print(F("  Zone ID: "));
    Serial.println(ZONE_ID);
    Serial.print(F("  Air Temperature: "));
    Serial.print(data.air_temp, 1);
    Serial.println(F(" °C"));
    Serial.print(F("  Air Humidity: "));
    Serial.print(data.air_humidity, 1);
    Serial.println(F(" %"));
    
    // MQTT로 데이터 전송
    if (mqttHandler.publishSensorData(data)) {
      Serial.println(F("[SENSOR] Data published successfully"));
    } else {
      Serial.println(F("[SENSOR] WARNING: Failed to publish data"));
    }
  } else {
    Serial.println(F("[SENSOR] ERROR: Failed to read sensor data"));
    
    // 센서 재초기화 시도
    Serial.println(F("[SENSOR] Attempting to reinitialize sensor..."));
    if (airSensor.begin()) {
      Serial.println(F("[SENSOR] Sensor reinitialized successfully"));
    } else {
      Serial.println(F("[SENSOR] ERROR: Sensor reinitialization failed"));
    }
  }
  
  DEBUG_PRINTLN(F("-------------------------------\n"));
}

// ============================================
// 시작 메시지 출력
// ============================================
void printStartupMessage() {
  Serial.println(F("\n"));
  Serial.println(F("========================================"));
  Serial.println(F("  Wasabi SmartFarm"));
  Serial.println(F("  Air Sensor Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2024-12-11"));
  Serial.println(F("========================================"));
  Serial.print(F("  Zone ID: "));
  Serial.println(ZONE_ID);
  Serial.print(F("  Sensor: SHT30 (I2C Address: 0x"));
  Serial.print(SHT30_I2C_ADDRESS, HEX);
  Serial.println(F(")"));
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
  Serial.println(F("========================================\n"));
}
