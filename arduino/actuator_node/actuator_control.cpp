/*
 * Wasabi SmartFarm - 액추에이터 제어 라이브러리 구현
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "actuator_control.h"

// ============================================
// 생성자
// ============================================
ActuatorControl::ActuatorControl() {
  // 상태 초기화
  memset(&_state, 0, sizeof(ActuatorState));
  _state.emergency_stop = false;
}

// ============================================
// 초기화
// ============================================
void ActuatorControl::begin() {
  DEBUG_PRINTLN(F("[ACTUATOR] Initializing actuator control..."));
  
  // 릴레이 핀 설정
  pinMode(RELAY_CH1_PIN, OUTPUT);
  pinMode(RELAY_CH2_PIN, OUTPUT);
  pinMode(RELAY_CH3_PIN, OUTPUT);
  pinMode(RELAY_CH4_PIN, OUTPUT);
  
  // 모든 릴레이 OFF
  digitalWrite(RELAY_CH1_PIN, RELAY_OFF);
  digitalWrite(RELAY_CH2_PIN, RELAY_OFF);
  digitalWrite(RELAY_CH3_PIN, RELAY_OFF);
  digitalWrite(RELAY_CH4_PIN, RELAY_OFF);
  
  DEBUG_PRINTLN(F("[ACTUATOR] Relay pins configured:"));
  DEBUG_PRINTF("[ACTUATOR]   CH1 (Irrigation): D%d\n", RELAY_CH1_PIN);
  DEBUG_PRINTF("[ACTUATOR]   CH2 (Drainage): D%d\n", RELAY_CH2_PIN);
  DEBUG_PRINTF("[ACTUATOR]   CH3 (Fan): D%d\n", RELAY_CH3_PIN);
  DEBUG_PRINTF("[ACTUATOR]   CH4 (LED): D%d\n", RELAY_CH4_PIN);
  DEBUG_PRINTLN(F("[ACTUATOR] All actuators initialized (OFF state)"));
}

// ============================================
// 릴레이 제어 (내부)
// ============================================
void ActuatorControl::setRelay(uint8_t pin, bool state) {
  digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
  DEBUG_PRINTF("[ACTUATOR] Relay D%d set to %s\n", pin, state ? "ON" : "OFF");
}

// ============================================
// 타임아웃 체크
// ============================================
bool ActuatorControl::checkTimeout(unsigned long startTime, unsigned long timeout) {
  if (startTime == 0) return false;  // 시작 안 됨
  
  return (millis() - startTime >= timeout);
}

// ============================================
// 동시 작동 방지 체크
// ============================================
bool ActuatorControl::canActivatePump(bool isIrrigation) {
  if (!ALLOW_SIMULTANEOUS_PUMPS) {
    if (isIrrigation && _state.drainage_pump) {
      DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Cannot start irrigation pump while drainage pump is running"));
      return false;
    }
    if (!isIrrigation && _state.irrigation_pump) {
      DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Cannot start drainage pump while irrigation pump is running"));
      return false;
    }
  }
  return true;
}

// ============================================
// 관수 펌프 시작
// ============================================
bool ActuatorControl::startIrrigationPump() {
  // 긴급 정지 체크
  if (_state.emergency_stop) {
    DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Emergency stop is active!"));
    return false;
  }
  
  // 동시 작동 체크
  if (!canActivatePump(true)) {
    return false;
  }
  
  // 이미 작동 중
  if (_state.irrigation_pump) {
    DEBUG_PRINTLN(F("[ACTUATOR] Irrigation pump is already running"));
    return true;
  }
  
  // 펌프 시작
  setRelay(RELAY_CH1_PIN, true);
  _state.irrigation_pump = true;
  _state.irrigation_start_time = millis();
  _state.irrigation_count++;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Irrigation pump STARTED"));
  DEBUG_PRINTF("[ACTUATOR]   Timeout: %lu seconds\n", IRRIGATION_TIMEOUT / 1000);
  
  return true;
}

// ============================================
// 관수 펌프 정지
// ============================================
bool ActuatorControl::stopIrrigationPump() {
  if (!_state.irrigation_pump) {
    DEBUG_PRINTLN(F("[ACTUATOR] Irrigation pump is already stopped"));
    return true;
  }
  
  // 최소 ON 시간 체크
  if (_state.irrigation_start_time > 0) {
    unsigned long runTime = millis() - _state.irrigation_start_time;
    if (runTime < MIN_ON_TIME) {
      DEBUG_PRINTF("[ACTUATOR] WARNING: Irrigation pump ran for only %lu ms (min: %lu ms)\n", 
                   runTime, MIN_ON_TIME);
    }
    _state.total_irrigation_time += runTime;
  }
  
  // 펌프 정지
  setRelay(RELAY_CH1_PIN, false);
  _state.irrigation_pump = false;
  _state.irrigation_start_time = 0;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Irrigation pump STOPPED"));
  DEBUG_PRINTF("[ACTUATOR]   Total irrigation time: %lu seconds\n", 
               _state.total_irrigation_time / 1000);
  
  return true;
}

// ============================================
// 배수 펌프 시작
// ============================================
bool ActuatorControl::startDrainagePump() {
  // 긴급 정지 체크
  if (_state.emergency_stop) {
    DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Emergency stop is active!"));
    return false;
  }
  
  // 동시 작동 체크
  if (!canActivatePump(false)) {
    return false;
  }
  
  // 이미 작동 중
  if (_state.drainage_pump) {
    DEBUG_PRINTLN(F("[ACTUATOR] Drainage pump is already running"));
    return true;
  }
  
  // 펌프 시작
  setRelay(RELAY_CH2_PIN, true);
  _state.drainage_pump = true;
  _state.drainage_start_time = millis();
  _state.drainage_count++;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Drainage pump STARTED"));
  DEBUG_PRINTF("[ACTUATOR]   Timeout: %lu seconds\n", DRAINAGE_TIMEOUT / 1000);
  
  return true;
}

// ============================================
// 배수 펌프 정지
// ============================================
bool ActuatorControl::stopDrainagePump() {
  if (!_state.drainage_pump) {
    DEBUG_PRINTLN(F("[ACTUATOR] Drainage pump is already stopped"));
    return true;
  }
  
  // 최소 ON 시간 체크
  if (_state.drainage_start_time > 0) {
    unsigned long runTime = millis() - _state.drainage_start_time;
    if (runTime < MIN_ON_TIME) {
      DEBUG_PRINTF("[ACTUATOR] WARNING: Drainage pump ran for only %lu ms (min: %lu ms)\n", 
                   runTime, MIN_ON_TIME);
    }
    _state.total_drainage_time += runTime;
  }
  
  // 펌프 정지
  setRelay(RELAY_CH2_PIN, false);
  _state.drainage_pump = false;
  _state.drainage_start_time = 0;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Drainage pump STOPPED"));
  DEBUG_PRINTF("[ACTUATOR]   Total drainage time: %lu seconds\n", 
               _state.total_drainage_time / 1000);
  
  return true;
}

// ============================================
// 팬 시작
// ============================================
bool ActuatorControl::startFan() {
  if (_state.emergency_stop) {
    DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Emergency stop is active!"));
    return false;
  }
  
  if (_state.fan) {
    DEBUG_PRINTLN(F("[ACTUATOR] Fan is already running"));
    return true;
  }
  
  setRelay(RELAY_CH3_PIN, true);
  _state.fan = true;
  _state.fan_start_time = millis();
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Fan STARTED"));
  
  return true;
}

// ============================================
// 팬 정지
// ============================================
bool ActuatorControl::stopFan() {
  if (!_state.fan) {
    DEBUG_PRINTLN(F("[ACTUATOR] Fan is already stopped"));
    return true;
  }
  
  setRelay(RELAY_CH3_PIN, false);
  _state.fan = false;
  _state.fan_start_time = 0;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Fan STOPPED"));
  
  return true;
}

// ============================================
// LED 조명 시작
// ============================================
bool ActuatorControl::startLED() {
  if (_state.emergency_stop) {
    DEBUG_PRINTLN(F("[ACTUATOR] ERROR: Emergency stop is active!"));
    return false;
  }
  
  if (_state.led) {
    DEBUG_PRINTLN(F("[ACTUATOR] LED is already on"));
    return true;
  }
  
  setRelay(RELAY_CH4_PIN, true);
  _state.led = true;
  _state.led_start_time = millis();
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ LED STARTED"));
  
  return true;
}

// ============================================
// LED 조명 정지
// ============================================
bool ActuatorControl::stopLED() {
  if (!_state.led) {
    DEBUG_PRINTLN(F("[ACTUATOR] LED is already off"));
    return true;
  }
  
  setRelay(RELAY_CH4_PIN, false);
  _state.led = false;
  _state.led_start_time = 0;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ LED STOPPED"));
  
  return true;
}

// ============================================
// 긴급 정지
// ============================================
void ActuatorControl::emergencyStop() {
  DEBUG_PRINTLN(F("\n[ACTUATOR] ⚠️  EMERGENCY STOP ACTIVATED!"));
  
  _state.emergency_stop = true;
  _state.emergency_stop_time = millis();
  
  // 모든 액추에이터 즉시 정지
  stopAll();
  
  DEBUG_PRINTLN(F("[ACTUATOR] All actuators have been stopped"));
  DEBUG_PRINTF("[ACTUATOR] System locked for %lu seconds\n", EMERGENCY_COOLDOWN / 1000);
}

// ============================================
// 긴급 정지 해제
// ============================================
void ActuatorControl::resetEmergencyStop() {
  // 쿨다운 체크
  if (_state.emergency_stop && _state.emergency_stop_time > 0) {
    unsigned long elapsed = millis() - _state.emergency_stop_time;
    if (elapsed < EMERGENCY_COOLDOWN) {
      DEBUG_PRINTF("[ACTUATOR] ERROR: Cannot reset yet. Wait %lu more seconds\n", 
                   (EMERGENCY_COOLDOWN - elapsed) / 1000);
      return;
    }
  }
  
  _state.emergency_stop = false;
  _state.emergency_stop_time = 0;
  
  DEBUG_PRINTLN(F("[ACTUATOR] ✓ Emergency stop RESET"));
  DEBUG_PRINTLN(F("[ACTUATOR] System ready for operation"));
}

// ============================================
// 모든 액추에이터 정지
// ============================================
void ActuatorControl::stopAll() {
  DEBUG_PRINTLN(F("[ACTUATOR] Stopping all actuators..."));
  
  stopIrrigationPump();
  stopDrainagePump();
  stopFan();
  stopLED();
  
  DEBUG_PRINTLN(F("[ACTUATOR] All actuators stopped"));
}

// ============================================
// 타임아웃 체크
// ============================================
void ActuatorControl::checkTimeouts() {
  // 관수 펌프 타임아웃
  if (_state.irrigation_pump && checkTimeout(_state.irrigation_start_time, IRRIGATION_TIMEOUT)) {
    DEBUG_PRINTLN(F("[ACTUATOR] ⚠️  Irrigation pump TIMEOUT!"));
    stopIrrigationPump();
  }
  
  // 배수 펌프 타임아웃
  if (_state.drainage_pump && checkTimeout(_state.drainage_start_time, DRAINAGE_TIMEOUT)) {
    DEBUG_PRINTLN(F("[ACTUATOR] ⚠️  Drainage pump TIMEOUT!"));
    stopDrainagePump();
  }
  
  // 팬 타임아웃
  if (_state.fan && checkTimeout(_state.fan_start_time, FAN_TIMEOUT)) {
    DEBUG_PRINTLN(F("[ACTUATOR] ⚠️  Fan TIMEOUT!"));
    stopFan();
  }
  
  // LED 타임아웃
  if (_state.led && checkTimeout(_state.led_start_time, LED_TIMEOUT)) {
    DEBUG_PRINTLN(F("[ACTUATOR] ⚠️  LED TIMEOUT!"));
    stopLED();
  }
}

// ============================================
// 상태 가져오기
// ============================================
ActuatorState ActuatorControl::getState() {
  return _state;
}

// ============================================
// 긴급 정지 상태 확인
// ============================================
bool ActuatorControl::isEmergencyStopped() {
  return _state.emergency_stop;
}

// ============================================
// 특정 액추에이터 상태 확인
// ============================================
bool ActuatorControl::isIrrigationPumpRunning() {
  return _state.irrigation_pump;
}

bool ActuatorControl::isDrainagePumpRunning() {
  return _state.drainage_pump;
}

bool ActuatorControl::isFanRunning() {
  return _state.fan;
}

bool ActuatorControl::isLEDRunning() {
  return _state.led;
}
