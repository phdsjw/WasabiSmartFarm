/*
 * Wasabi SmartFarm - 대기+토양 통합 센서 노드
 * 
 * 기능:
 * - SHT30 온습도 센서 데이터 읽기 (대기)
 * - SEN0604 토양 센서 데이터 읽기 (토양)
 * - WiFi 연결
 * - MQTT를 통한 센서 데이터 전송 (10초 주기)
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - SHT30 온습도 센서 (I2C)
 * - RS485 확장보드 (DFR0259)
 * - SEN0604 4-in-1 토양 센서
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#include "config.h"
#include "sht30_sensor.h"
#include "sen0604_modbus.h"
#include "mqtt_handler.h"

// 전역 객체
SHT30Sensor airSensor;
SEN0604Modbus soilSensor;
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
    Serial.println(F("[SETUP] Continuing without air sensor..."));
  } else {
    Serial.println(F("[SETUP] SHT30 sensor initialized successfully"));
  }
  
  // SEN0604 센서 초기화
  Serial.println(F("\n[SETUP] Initializing SEN0604 sensor..."));
  if (!soilSensor.begin()) {
    Serial.println(F("[ERROR] Warning: SEN0604 initialization failed or sensor not responding"));
    Serial.println(F("[SETUP] Continuing... Will retry on first read"));
  } else {
    Serial.println(F("[SETUP] SEN0604 sensor initialized successfully"));
  }
  
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
  Serial.println(F("  Air+Soil Combined Node Ready!"));
  Serial.println(F("  Zone ID: " ZONE_ID));
  Serial.println(F("  Tank ID: " TANK_ID));
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
  if (currentMillis - lastLedBlink >= LED_BLINK_INTERVAL) {
    lastLedBlink = currentMillis;
    
    if (mqttHandler.isConnected()) {
      // MQTT 연결됨: 느린 깜빡임
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
    } else {
      // MQTT 연결 안됨: 빠른 깜빡임 (500ms)
      if (currentMillis - lastLedBlink >= 500) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN_PIN, ledState);
      }
    }
  }
  
  // 짧은 대기
  delay(100);
}

// ============================================
// 센서 데이터 읽기 및 전송
// ============================================
void readAndPublishSensorData() {
  DEBUG_PRINTLN(F("\n========================================"));
  DEBUG_PRINT(F("[SENSOR] Reading combined sensor data (Zone "));
  DEBUG_PRINT(ZONE_ID);
  DEBUG_PRINT(F(", Tank "));
  DEBUG_PRINT(TANK_ID);
  DEBUG_PRINTLN(F(")..."));
  
  // 1. SHT30 대기 센서 데이터 읽기
  AirSensorData airData;
  airData.timestamp = millis();
  airData.is_valid = false;
  
  if (airSensor.readTempHumidity(airData.air_temp, airData.air_humidity)) {
    airData.is_valid = true;
    
    // 시리얼 출력
    Serial.println(F("[AIR SENSOR] Data:"));
    Serial.print(F("  Zone ID: "));
    Serial.println(ZONE_ID);
    Serial.print(F("  Air Temperature: "));
    Serial.print(airData.air_temp, 1);
    Serial.println(F(" °C"));
    Serial.print(F("  Air Humidity: "));
    Serial.print(airData.air_humidity, 1);
    Serial.println(F(" %"));
    
    // MQTT로 대기 데이터 전송
    if (mqttHandler.publishAirData(airData)) {
      Serial.println(F("[AIR SENSOR] Data published successfully"));
    } else {
      Serial.println(F("[AIR SENSOR] WARNING: Failed to publish data"));
    }
  } else {
    Serial.println(F("[AIR SENSOR] ERROR: Failed to read sensor data"));
  }
  
  // 2. SEN0604 토양 센서 데이터 읽기
  SoilSensorData soilData = soilSensor.readSensorData();
  
  if (soilData.valid) {
    // 시리얼 출력
    Serial.println(F("[SOIL SENSOR] Data:"));
    Serial.print(F("  Tank ID: "));
    Serial.println(TANK_ID);
    Serial.print(F("  Soil Temperature: "));
    Serial.print(soilData.soil_temp, 1);
    Serial.println(F(" °C"));
    Serial.print(F("  Soil Moisture: "));
    Serial.print(soilData.soil_moisture, 1);
    Serial.println(F(" %"));
    Serial.print(F("  Soil EC: "));
    Serial.print(soilData.soil_ec, 1);
    Serial.println(F(" μS/cm"));
    Serial.print(F("  Soil pH: "));
    Serial.println(soilData.soil_ph, 2);
    
    // MQTT로 토양 데이터 전송
    if (mqttHandler.publishSoilData(soilData)) {
      Serial.println(F("[SOIL SENSOR] Data published successfully"));
    } else {
      Serial.println(F("[SOIL SENSOR] WARNING: Failed to publish data"));
    }
  } else {
    Serial.println(F("[SOIL SENSOR] ERROR: Failed to read sensor data"));
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
  Serial.println(F("  Air+Soil Combined Sensor Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2025-12-27"));
  Serial.println(F("========================================"));
  Serial.print(F("  Zone ID: "));
  Serial.println(ZONE_ID);
  Serial.print(F("  Tank ID: "));
  Serial.println(TANK_ID);
  Serial.println(F("  Sensors:"));
  Serial.print(F("    - SHT30 (I2C Address: 0x"));
  Serial.print(SHT30_I2C_ADDRESS, HEX);
  Serial.println(F(")"));
  Serial.print(F("    - SEN0604 (Modbus Slave ID: "));
  Serial.print(MODBUS_SLAVE_ID);
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
