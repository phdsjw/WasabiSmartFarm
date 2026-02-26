/*
 * Wasabi SmartFarm - HC-SR04 초음파 수위 센서 드라이버 헤더
 *
 * 기능:
 *   - TRIG/ECHO 핀 초기화
 *   - 이동평균 필터 (SAMPLE_COUNT 회 측정 후 중간값 반환)
 *   - 거리(cm) → 수위%(%) 변환
 *   - 유효 범위 체크 (MIN_DISTANCE_CM ~ MAX_DISTANCE_CM)
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */

#ifndef HCSR04_SENSOR_HA_H
#define HCSR04_SENSOR_HA_H

#include <Arduino.h>
#include "config_ha.h"

class HCSR04Sensor {
public:
    void begin();
    WaterLevelData readWaterLevel();

private:
    float measureDistanceCm();
    float distanceToPercent(float distance_cm);
};

#endif // HCSR04_SENSOR_HA_H
