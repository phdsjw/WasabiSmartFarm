/*
 * Wasabi SmartFarm - 조명 제어 라이브러리 (헤더)
 * 
 * 기능:
 * - 단일 채널 릴레이로 Grow Light 제어
 * - MQTT로 명령 수신 및 상태 보고
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#ifndef LIGHT_CONTROL_H
#define LIGHT_CONTROL_H

#include <Arduino.h>
#include "config.h"

class LightControl {
private:
  LightState _state;
  
  // 릴레이 제어 (내부)
  void setRelay(bool state);
  
public:
  // 생성자
  LightControl();
  
  // 초기화
  void begin();
  
  // 조명 켜기
  bool turnOn();
  
  // 조명 끄기
  bool turnOff();
  
  // 토글
  bool toggle();
  
  // 상태 가져오기
  LightState getState();
  
  // 조명이 켜져있는지 확인
  bool isLightOn();
  
  // MQTT에서 명령 수신 처리
  void handleCommand(const char* command);
};

#endif // LIGHT_CONTROL_H
