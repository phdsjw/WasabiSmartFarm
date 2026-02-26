/*
 * Wasabi SmartFarm - 밸브 제어 라이브러리 (헤더)
 * 
 * 기능:
 * - 단일 채널 릴레이로 뺄브 제어
 * - MQTT로 명령 수신 및 상태 보고
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#ifndef VALVE_CONTROL_H
#define VALVE_CONTROL_H

#include <Arduino.h>
#include "config.h"

class ValveControl {
private:
  ValveState _state;
  
  // 릴레이 제어 (내부)
  void setRelay(bool state);
  
public:
  // 생성자
  ValveControl();
  
  // 초기화
  void begin();
  
  // 밸브 열기
  bool openValve();
  
  // 밸브 닫기
  bool closeValve();
  
  // 토글
  bool toggleValve();
  
  // 상태 가져오기
  ValveState getState();
  
  // 밸브가 열려있는지 확인
  bool isValveOpen();
  
  // MQTT에서 명령 수신 처리
  void handleCommand(const char* command);
};

#endif // VALVE_CONTROL_H
