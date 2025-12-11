/*
 * Wasabi SmartFarm - 수조 센서 노드
 * 
 * 기능:
 * - DS18B20 수온 센서 (1-Wire)
 * - SEN0161 pH 센서 (아날로그)
 * - SEN0244 TDS 센서 (아날로그)
 * - SEN0451 Pro EC 센서 (아날로그)
 * - WiFi 연결
 * - MQTT를 통한 센서 데이터 전송 (10초 주기)
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - DS18B20 (1-Wire, D4)
 * - SEN0161 pH (Analog, A0)
 * - SEN0244 TDS (Analog, A1)
 * - SEN0451 Pro EC (Analog, A2)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "config.h"
#include "water_sensors.h"
#include "mqtt_handler.h"

// 전역 객체
WaterSensors waterSensors;
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
  
  // 수조 센서 초기화
  Serial.println(F("\n[SETUP] Initializing water tank sensors..."));
  if (!waterSensors.begin()) {
    Serial.println(F("[ERROR] Failed to initialize water sensors!"));
    Serial.println(F("[ERROR] Please check sensor connections:"));
    Serial.println(F("        - DS18B20: D4 (1-Wire)"));
    Serial.println(F("        - pH: A0 (Analog)"));
    Serial.println(F("        - TDS: A1 (Analog)"));
    Serial.println(F("        - EC: A2 (Analog)"));
    
    // 오류 표시 (빠른 깜빡임)
    while (true) {
      digitalWrite(LED_BUILTIN_PIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN_PIN, LOW);
      delay(100);
    }
  }
  
  // DS18B20 센서 확인
  int tempSensorCount = waterSensors.getTemperatureSensorCount();
  if (tempSensorCount == 0) {
    Serial.println(F("[WARNING] No DS18B20 temperature sensor found!"));
    Serial.println(F("[WARNING] Water temperature will not be available."));
  } else {
    Serial.print(F("[SETUP] DS18B20 sensor found: "));
    Serial.println(tempSensorCount);
  }
  
  Serial.println(F("[SETUP] Water sensors initialized successfully"));
  
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
  Serial.println(F("  Water Tank Sensor Node Ready!"));
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
  // 센서 데이터 구조체
  WaterTankSensorData data;
  
  // 모든 센서 데이터 읽기
  if (waterSensors.readAllSensors(data)) {
    // 시리얼 출력
    Serial.println(F("[SENSOR] Water tank sensor data:"));
    Serial.print(F("  Water Temperature: "));
    Serial.print(data.water_temp, 1);
    Serial.println(F(" °C"));
    Serial.print(F("  Water pH: "));
    Serial.println(data.water_ph, 2);
    Serial.print(F("  Water TDS: "));
    Serial.print(data.water_tds);
    Serial.println(F(" ppm"));
    Serial.print(F("  Water EC: "));
    Serial.print(data.water_ec, 2);
    Serial.println(F(" mS/cm"));
    
    // MQTT로 데이터 전송
    if (mqttHandler.publishSensorData(data)) {
      Serial.println(F("[SENSOR] Data published successfully"));
    } else {
      Serial.println(F("[SENSOR] WARNING: Failed to publish data"));
    }
  } else {
    Serial.println(F("[SENSOR] ERROR: Failed to read sensor data"));
    
    // 센서 재초기화 시도
    Serial.println(F("[SENSOR] Attempting to reinitialize sensors..."));
    if (waterSensors.begin()) {
      Serial.println(F("[SENSOR] Sensors reinitialized successfully"));
    } else {
      Serial.println(F("[SENSOR] ERROR: Sensor reinitialization failed"));
    }
  }
}

// ============================================
// 시작 메시지 출력
// ============================================
void printStartupMessage() {
  Serial.println(F("\n"));
  Serial.println(F("========================================"));
  Serial.println(F("  Wasabi SmartFarm"));
  Serial.println(F("  Water Tank Sensor Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2024-12-11"));
  Serial.println(F("========================================"));
  Serial.println(F("  Sensors:"));
  Serial.println(F("    - DS18B20 (Water Temp, 1-Wire D4)"));
  Serial.println(F("    - SEN0161 (pH, Analog A0)"));
  Serial.println(F("    - SEN0244 (TDS, Analog A1)"));
  Serial.println(F("    - SEN0451 Pro (EC, Analog A2)"));
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
