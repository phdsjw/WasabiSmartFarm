/*
 * Wasabi SmartFarm - 액추에이터 노드
 * 
 * 기능:
 * - 4채널 릴레이를 통한 액추에이터 제어
 * - 관수 펌프 (2HP) - CH1
 * - 배수 펌프 (1HP) - CH2
 * - 천장 팬 (예비) - CH3
 * - LED 조명 (예비) - CH4
 * - MQTT 명령 수신 및 실행
 * - 안전 기능 (타임아웃, 긴급 정지, 인터록)
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - 4채널 릴레이 모듈
 * - SSR × 2 (40A)
 * - 전자접촉기 (MC) × 2 (LS MC-18b, MC-12b)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "config.h"
#include "actuator_control.h"
#include "mqtt_handler.h"

// 전역 객체
ActuatorControl actuatorControl;
MQTTHandler mqttHandler(&actuatorControl);

// 타이머 변수
unsigned long lastStateReport = 0;
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
  
  // 액추에이터 제어 초기화
  Serial.println(F("\n[SETUP] Initializing actuator control..."));
  actuatorControl.begin();
  Serial.println(F("[SETUP] Actuator control initialized successfully"));
  
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
  Serial.println(F("  Actuator Node Ready!"));
  Serial.println(F("  Waiting for MQTT commands..."));
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
  
  // MQTT 연결 유지 및 명령 수신
  mqttHandler.loop();
  
  // 타임아웃 체크
  actuatorControl.checkTimeouts();
  
  // 상태 리포트 전송 (5초 주기)
  if (currentMillis - lastStateReport >= STATE_REPORT_INTERVAL) {
    lastStateReport = currentMillis;
    
    ActuatorState state = actuatorControl.getState();
    mqttHandler.publishStateReport(state);
  }
  
  // 하트비트 전송 (10초 주기)
  if (currentMillis - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = currentMillis;
    
    ActuatorState state = actuatorControl.getState();
    mqttHandler.publishHeartbeat(state);
  }
  
  // LED 상태 표시
  updateLEDStatus();
  
  // 짧은 대기
  delay(10);
}

// ============================================
// LED 상태 업데이트
// ============================================
void updateLEDStatus() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();
  
  // 긴급 정지 시: 빠른 깜빡임 (200ms)
  if (actuatorControl.isEmergencyStopped()) {
    if (currentMillis - lastUpdate >= 200) {
      lastUpdate = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
    }
  }
  // 액추에이터 작동 중: 느린 깜빡임 (500ms)
  else if (actuatorControl.isIrrigationPumpRunning() || 
           actuatorControl.isDrainagePumpRunning() ||
           actuatorControl.isFanRunning() ||
           actuatorControl.isLEDRunning()) {
    if (currentMillis - lastUpdate >= 500) {
      lastUpdate = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
    }
  }
  // 대기 중: 매우 느린 깜빡임 (2초)
  else {
    if (currentMillis - lastUpdate >= 2000) {
      lastUpdate = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN_PIN, ledState);
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
  Serial.println(F("  Actuator Control Node"));
  Serial.println(F("========================================"));
  Serial.println(F("  Version: v1.0.0"));
  Serial.println(F("  Author: 서준원"));
  Serial.println(F("  Date: 2024-12-11"));
  Serial.println(F("========================================"));
  Serial.println(F("  Actuators:"));
  Serial.println(F("    - CH1: Irrigation Pump (2HP)"));
  Serial.println(F("    - CH2: Drainage Pump (1HP)"));
  Serial.println(F("    - CH3: Ceiling Fan (Reserve)"));
  Serial.println(F("    - CH4: LED Light (Reserve)"));
  Serial.print(F("  WiFi SSID: "));
  Serial.println(WIFI_SSID);
  Serial.print(F("  MQTT Broker: "));
  Serial.print(MQTT_SERVER);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);
  Serial.println(F("  Safety Features:"));
  Serial.print(F("    - Irrigation Timeout: "));
  Serial.print(IRRIGATION_TIMEOUT / 1000);
  Serial.println(F(" sec"));
  Serial.print(F("    - Drainage Timeout: "));
  Serial.print(DRAINAGE_TIMEOUT / 1000);
  Serial.println(F(" sec"));
  Serial.print(F("    - Emergency Cooldown: "));
  Serial.print(EMERGENCY_COOLDOWN / 1000);
  Serial.println(F(" sec"));
  Serial.print(F("    - Simultaneous Pumps: "));
  Serial.println(ALLOW_SIMULTANEOUS_PUMPS ? F("ALLOWED") : F("BLOCKED"));
  Serial.println(F("========================================\n"));
}
