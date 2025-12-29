/*
 * Wasabi SmartFarm - HC-SR04 초음파 센서 라이브러리 (구현)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#include "hcsr04_sensor.h"

// ============================================
// 생성자
// ============================================
HCSR04Sensor::HCSR04Sensor() {
}

// ============================================
// 초기화
// ============================================
void HCSR04Sensor::begin() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // 초기 상태
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  DEBUG_PRINTLN(F("[HCSR04] Sensor initialized"));
  DEBUG_PRINT(F("[HCSR04] Trig Pin: D"));
  DEBUG_PRINTLN(TRIG_PIN);
  DEBUG_PRINT(F("[HCSR04] Echo Pin: D"));
  DEBUG_PRINTLN(ECHO_PIN);
}

// ============================================
// 단일 거리 측정
// ============================================
float HCSR04Sensor::measureDistance() {
  // Trig 핀을 10us 동안 HIGH
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Echo 핀에서 HIGH 신호 대기 (타임아웃: 30ms)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, MEASUREMENT_TIMEOUT);
  
  // 타임아웃 확인
  if (duration == 0) {
    DEBUG_PRINTLN(F("[HCSR04] Timeout: No echo received"));
    return -1.0;
  }
  
  // 거리 계산 (cm)
  // 음속 340m/s = 0.034cm/us
  // 왕복 거리이므로 2로 나눔
  float distance_cm = duration * 0.034 / 2.0;
  
  // 유효 범위 확인
  if (distance_cm < MIN_DISTANCE_CM || distance_cm > MAX_DISTANCE_CM) {
    DEBUG_PRINT(F("[HCSR04] Out of range: "));
    DEBUG_PRINT(distance_cm);
    DEBUG_PRINTLN(F(" cm"));
    return -1.0;
  }
  
  return distance_cm;
}

// ============================================
// 다중 샘플 평균 측정
// ============================================
float HCSR04Sensor::measureDistanceAverage() {
  float sum = 0.0;
  int validCount = 0;
  
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float distance = measureDistance();
    
    if (distance > 0) {
      sum += distance;
      validCount++;
    }
    
    if (i < SAMPLE_COUNT - 1) {
      delay(SAMPLE_DELAY_MS);
    }
  }
  
  // 유효한 샘플이 없으면 -1 반환
  if (validCount == 0) {
    DEBUG_PRINTLN(F("[HCSR04] No valid samples"));
    return -1.0;
  }
  
  // 평균 계산
  float average = sum / validCount;
  
  DEBUG_PRINT(F("[HCSR04] Average distance ("));
  DEBUG_PRINT(validCount);
  DEBUG_PRINT(F(" samples): "));
  DEBUG_PRINT(average, 1);
  DEBUG_PRINTLN(F(" cm"));
  
  return average;
}

// ============================================
// 수위 계산 (%)
// ============================================
float HCSR04Sensor::calculateWaterLevel(float distance_cm) {
  // 물탱크 높이에서 측정 거리를 뺀 값이 현재 수위
  // 수위(%) = (탱크 높이 - 측정 거리) / 탱크 높이 × 100
  
  float waterHeight = TANK_HEIGHT_CM - distance_cm - SENSOR_OFFSET_CM;
  
  // 음수 보정
  if (waterHeight < 0) {
    waterHeight = 0;
  }
  
  // 탱크 높이 초과 보정
  if (waterHeight > TANK_HEIGHT_CM) {
    waterHeight = TANK_HEIGHT_CM;
  }
  
  float waterLevel = (waterHeight / TANK_HEIGHT_CM) * 100.0;
  
  DEBUG_PRINT(F("[HCSR04] Water height: "));
  DEBUG_PRINT(waterHeight, 1);
  DEBUG_PRINTLN(F(" cm"));
  DEBUG_PRINT(F("[HCSR04] Water level: "));
  DEBUG_PRINT(waterLevel, 1);
  DEBUG_PRINTLN(F(" %"));
  
  return waterLevel;
}

// ============================================
// 수위 데이터 읽기
// ============================================
WaterLevelData HCSR04Sensor::readWaterLevel() {
  WaterLevelData data;
  data.timestamp = millis();
  data.is_valid = false;
  
  // 거리 측정 (평균)
  float distance = measureDistanceAverage();
  
  if (distance < 0) {
    DEBUG_PRINTLN(F("[HCSR04] Failed to measure distance"));
    return data;
  }
  
  // 수위 계산
  data.distance_cm = distance;
  data.water_level_percent = calculateWaterLevel(distance);
  data.is_valid = true;
  
  return data;
}
