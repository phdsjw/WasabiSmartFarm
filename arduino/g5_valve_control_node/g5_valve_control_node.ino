/*
 * Wasabi SmartFarm - 밸브 제어 노드
 * 
 * 기능:
 * - 수위 танк 뺄브 on/off 제어
 * - WiFi 연결
 * - MQTT 명령 수신 및 상태 보고
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - Single Channel Relay (5V)
 * 
 * MQTT Topics:
 * - Command: smartfarm/wasabi/g5_water_level_valve/g5-01/valve/cmd
 * - State:   smartfarm/wasabi/g5_water_level_valve/g5-01/valve/state
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "config.h"
#include "valve_control.h"
#include "mqtt_handler.h"

// 전역 객체
ValveControl valveControl;
MQTTHandler mqttHandler(&valveControl);

// 타이머 변수
unsigned long lastHeartbeat = 0;
unsigned long lastStatePublish = 0;
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
  
  // 밸브 제어 초기화
  Serial.println(F("\n[SETUP] Initializing valve control..."));
  valveControl.begin();
  Serial.println(F("[SETUP] Valve control initialized"));
  
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
  mqttHandler.publishState();
  
  // 초기화 완료
  Serial.println(F("\n========================================"));
  Serial.println(F("  Valve Control Node Ready!"));
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
  lastHeartbeat = millis();
  lastStatePublish = millis();
  lastLedBlink = millis();
}

// ============================================
// 메인 루프
// ============================================
void loop() {
  unsigned long currentMillis = millis();
  
  // MQTT 연결 유지
  mqttHandler.loop();
  
  // 상태 전송 (5초마다)
  if (currentMillis - lastStatePublish >= 5000) {
    lastStatePublish = currentMillis;
    mqttHandler.publishState();
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
// 시작 메시지 출력
// ============================================
void printStartupMessage() {
  Serial.println(F("\n"));
  Serial.println(F("========================================"));
  Serial.println(F("  Wasabi SmartFarm"));
  Serial.println(F("  Valve Control Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2026-02-25"));
  Serial.println(F("========================================"));
  Serial.print(F("  Node ID: "));
  Serial.println(NODE_ID);
  Serial.println(F("  MCU: Arduino Uno R4 WiFi"));
  Serial.println(F("  Actuator: Single Channel Relay"));
  Serial.print(F("  WiFi SSID: "));
  Serial.println(WIFI_SSID);
  Serial.print(F("  MQTT Broker: "));
  Serial.print(MQTT_SERVER);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);
  Serial.print(F("  MQTT Command Topic: "));
  Serial.println(MQTT_TOPIC_CMD);
  Serial.print(F("  MQTT State Topic: "));
  Serial.println(MQTT_TOPIC_STATE);
  Serial.print(F("  Heartbeat Interval: "));
  Serial.print(HEARTBEAT_INTERVAL / 1000);
  Serial.println(F(" sec"));
  Serial.println(F("========================================\n"));
}
