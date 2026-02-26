/*
 * Wasabi SmartFarm - HC-SR04 초음파 센서 라이브러리 (헤더)
 * 
 * 기능:
 * - HC-SR04 초음파 센서로 거리 측정
 * - 다중 샘플 평균 필터링
 * - 물탱크 수위 계산 (%)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#ifndef HCSR04_SENSOR_H
#define HCSR04_SENSOR_H

#include <Arduino.h>
#include "config.h"

class HCSR04Sensor {
private:
  // 단일 거리 측정
  float measureDistance();
  
  // 다중 샘플 평균
  float measureDistanceAverage();
  
  // 수위 계산
  float calculateWaterLevel(float distance_cm);
  
public:
  // 생성자
  HCSR04Sensor();
  
  // 초기화
  void begin();
  
  // 수위 데이터 읽기
  WaterLevelData readWaterLevel();
};

#endif // HCSR04_SENSOR_H
