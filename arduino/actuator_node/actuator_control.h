/*
 * Wasabi SmartFarm - 액추에이터 제어 라이브러리
 * 
 * 기능:
 * - 4채널 릴레이 제어
 * - 타임아웃 자동 종료
 * - 긴급 정지
 * - 안전 인터록
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef ACTUATOR_CONTROL_H
#define ACTUATOR_CONTROL_H

#include <Arduino.h>
#include "config.h"

class ActuatorControl {
private:
  ActuatorState _state;
  
  // 릴레이 제어 (내부)
  void setRelay(uint8_t pin, bool state);
  
  // 타임아웃 체크
  bool checkTimeout(unsigned long startTime, unsigned long timeout);
  
  // 동시 작동 방지 체크
  bool canActivatePump(bool isIrrigation);

public:
  // 생성자
  ActuatorControl();
  
  // 초기화
  void begin();
  
  // 관수 펌프 제어
  bool startIrrigationPump();
  bool stopIrrigationPump();
  
  // 배수 펌프 제어
  bool startDrainagePump();
  bool stopDrainagePump();
  
  // 팬 제어
  bool startFan();
  bool stopFan();
  
  // LED 조명 제어
  bool startLED();
  bool stopLED();
  
  // 긴급 정지
  void emergencyStop();
  
  // 긴급 정지 해제
  void resetEmergencyStop();
  
  // 모든 액추에이터 정지
  void stopAll();
  
  // 타임아웃 체크 (loop에서 호출)
  void checkTimeouts();
  
  // 상태 가져오기
  ActuatorState getState();
  
  // 긴급 정지 상태 확인
  bool isEmergencyStopped();
  
  // 특정 액추에이터 상태 확인
  bool isIrrigationPumpRunning();
  bool isDrainagePumpRunning();
  bool isFanRunning();
  bool isLEDRunning();
};

#endif // ACTUATOR_CONTROL_H
