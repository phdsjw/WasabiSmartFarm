/*
 * Wasabi SmartFarm - 조명 제어 라이브러리 (구현)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "light_control.h"

// ============================================
// 생성자
// ============================================
LightControl::LightControl() {
  _state.light_on = false;
  _state.light_start_time = 0;
  _state.total_on_time = 0;
  _state.on_count = 0;
}

// ============================================
// 초기화
// ============================================
void LightControl::begin() {
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  
  // 초기에는 조명 끄기
  digitalWrite(LIGHT_RELAY_PIN, RELAY_OFF);
  
  DEBUG_PRINTLN(F("[LIGHT] Light control initialized"));
  DEBUG_PRINT(F("[LIGHT] Relay Pin: "));
  DEBUG_PRINTLN(LIGHT_RELAY_PIN);
}

// ============================================
// 릴레이 제어 (내부)
// ============================================
void LightControl::setRelay(bool state) {
  digitalWrite(LIGHT_RELAY_PIN, state ? RELAY_ON : RELAY_OFF);
}

// ============================================
// 조명 켜기
// ============================================
bool LightControl::turnOn() {
  if (_state.light_on) {
    DEBUG_PRINTLN(F("[LIGHT] Light already on"));
    return true;
  }
  
  setRelay(true);
  _state.light_on = true;
  _state.light_start_time = millis();
  _state.on_count++;
  
  DEBUG_PRINTLN(F("[LIGHT] Light turned on"));
  return true;
}

// ============================================
// 조명 끄기
// ============================================
bool LightControl::turnOff() {
  if (!_state.light_on) {
    DEBUG_PRINTLN(F("[LIGHT] Light already off"));
    return true;
  }
  
  // 누적 시간 업데이트
  _state.total_on_time += millis() - _state.light_start_time;
  
  setRelay(false);
  _state.light_on = false;
  _state.light_start_time = 0;
  
  DEBUG_PRINTLN(F("[LIGHT] Light turned off"));
  return true;
}

// ============================================
// 토글
// ============================================
bool LightControl::toggle() {
  if (_state.light_on) {
    return turnOff();
  } else {
    return turnOn();
  }
}

// ============================================
// 상태 가져오기
// ============================================
LightState LightControl::getState() {
  return _state;
}

// ============================================
// 조명이 켜져있는지 확인
// ============================================
bool LightControl::isLightOn() {
  return _state.light_on;
}

// ============================================
// MQTT에서 명령 수신 처리
// ============================================
void LightControl::handleCommand(const char* command) {
  DEBUG_PRINT(F("[LIGHT] Received command: "));
  DEBUG_PRINTLN(command);
  
  if (strcmp(command, "on") == 0 || strcmp(command, "ON") == 0) {
    turnOn();
  } else if (strcmp(command, "off") == 0 || strcmp(command, "OFF") == 0) {
    turnOff();
  } else if (strcmp(command, "toggle") == 0) {
    toggle();
  } else {
    DEBUG_PRINT(F("[LIGHT] Unknown command: "));
    DEBUG_PRINTLN(command);
  }
}
