/*
 * Wasabi SmartFarm - 밸브 제어 라이브러리 (구현)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#include "valve_control.h"

// ============================================
// 생성자
// ============================================
ValveControl::ValveControl() {
  _state.valve_open = false;
  _state.valve_start_time = 0;
  _state.total_open_time = 0;
  _state.open_count = 0;
}

// ============================================
// 초기화
// ============================================
void ValveControl::begin() {
  pinMode(VALVE_RELAY_PIN, OUTPUT);
  
  // 초기에는 밸브 닫기
  digitalWrite(VALVE_RELAY_PIN, RELAY_OFF);
  
  DEBUG_PRINTLN(F("[VALVE] Valve control initialized"));
  DEBUG_PRINT(F("[VALVE] Relay Pin: "));
  DEBUG_PRINTLN(VALVE_RELAY_PIN);
}

// ============================================
// 릴레이 제어 (내부)
// ============================================
void ValveControl::setRelay(bool state) {
  digitalWrite(VALVE_RELAY_PIN, state ? RELAY_ON : RELAY_OFF);
}

// ============================================
// 밸브 열기
// ============================================
bool ValveControl::openValve() {
  if (_state.valve_open) {
    DEBUG_PRINTLN(F("[VALVE] Valve already open"));
    return true;
  }
  
  setRelay(true);
  _state.valve_open = true;
  _state.valve_start_time = millis();
  _state.open_count++;
  
  DEBUG_PRINTLN(F("[VALVE] Valve opened"));
  return true;
}

// ============================================
// 밸브 닫기
// ============================================
bool ValveControl::closeValve() {
  if (!_state.valve_open) {
    DEBUG_PRINTLN(F("[VALVE] Valve already closed"));
    return true;
  }
  
  // 누적 시간 업데이트
  _state.total_open_time += millis() - _state.valve_start_time;
  
  setRelay(false);
  _state.valve_open = false;
  _state.valve_start_time = 0;
  
  DEBUG_PRINTLN(F("[VALVE] Valve closed"));
  return true;
}

// ============================================
// 토글
// ============================================
bool ValveControl::toggleValve() {
  if (_state.valve_open) {
    return closeValve();
  } else {
    return openValve();
  }
}

// ============================================
// 상태 가져오기
// ============================================
ValveState ValveControl::getState() {
  return _state;
}

// ============================================
// 밸브가 열려있는지 확인
// ============================================
bool ValveControl::isValveOpen() {
  return _state.valve_open;
}

// ============================================
// MQTT에서 명령 수신 처리
// ============================================
void ValveControl::handleCommand(const char* command) {
  DEBUG_PRINT(F("[VALVE] Received command: "));
  DEBUG_PRINTLN(command);
  
  if (strcmp(command, "on") == 0 || strcmp(command, "OPEN") == 0) {
    openValve();
  } else if (strcmp(command, "off") == 0 || strcmp(command, "CLOSE") == 0) {
    closeValve();
  } else if (strcmp(command, "toggle") == 0) {
    toggleValve();
  } else {
    DEBUG_PRINT(F("[VALVE] Unknown command: "));
    DEBUG_PRINTLN(command);
  }
}
