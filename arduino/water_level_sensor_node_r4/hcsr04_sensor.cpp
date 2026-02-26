/*
 * Wasabi SmartFarm - HC-SR04 초음파 수위 센서 드라이버 구현
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */

#include "hcsr04_sensor_ha.h"

// ============================================
// 초기화
// ============================================
void HCSR04Sensor::begin() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    delay(100);
    DEBUG_PRINTLN(F("[HC-SR04] Sensor initialized"));
    DEBUG_PRINTF("[HC-SR04] TRIG=D%d, ECHO=D%d\n", TRIG_PIN, ECHO_PIN);
    DEBUG_PRINTF("[HC-SR04] Tank height=%.1f cm, Offset=%.1f cm\n",
                 TANK_HEIGHT_CM, SENSOR_OFFSET_CM);
}

// ============================================
// 수위 읽기 (외부 호출 진입점)
// ============================================
WaterLevelData HCSR04Sensor::readWaterLevel() {
    WaterLevelData result;
    result.is_valid   = false;
    result.timestamp  = millis();

    // SAMPLE_COUNT 회 측정 후 평균
    float sum   = 0.0;
    int   valid = 0;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        float d = measureDistanceCm();
        if (d > 0.0) {
            sum += d;
            valid++;
        }
        delay(SAMPLE_DELAY_MS);
    }

    if (valid == 0) {
        DEBUG_PRINTLN(F("[HC-SR04] All samples invalid"));
        return result;
    }

    float avg = sum / valid;

    // 유효 범위 체크
    if (avg < MIN_DISTANCE_CM || avg > MAX_DISTANCE_CM) {
        DEBUG_PRINTF("[HC-SR04] Distance out of range: %.1f cm\n", avg);
        return result;
    }

    result.distance_cm         = avg;
    result.water_level_percent = distanceToPercent(avg);
    // 실제 수위 높이 = 탱크 높이 - (거리 - 오프셋)
    result.water_level_cm      = TANK_HEIGHT_CM - (avg - SENSOR_OFFSET_CM);
    if (result.water_level_cm < 0.0) result.water_level_cm = 0.0;
    result.is_valid            = true;

    DEBUG_PRINTF("[HC-SR04] dist=%.1f cm | level=%.1f cm | pct=%.1f%%\n",
                 result.distance_cm,
                 result.water_level_cm,
                 result.water_level_percent);

    return result;
}

// ============================================
// HC-SR04 단일 거리 측정
// ============================================
float HCSR04Sensor::measureDistanceCm() {
    // TRIG 펄스 10µs
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // ECHO 펄스 폭 측정
    long duration = pulseIn(ECHO_PIN, HIGH, MEASUREMENT_TIMEOUT);
    if (duration == 0) {
        return -1.0;  // 타임아웃
    }

    // 거리 계산: 음속 343m/s, 왕복이므로 /2
    float dist = (duration * 0.0343) / 2.0;
    return dist;
}

// ============================================
// 거리 → 수위 퍼센트 변환
//
// 탱크 구조:
//   센서 (상단)
//     ↓  SENSOR_OFFSET_CM  (센서 → 만수위 수면)
//   만수위 ────────── 100 %
//     ↓  (TANK_HEIGHT_CM)
//   바닥  ────────── 0 %
//
// 계산식:
//   usable_height = TANK_HEIGHT_CM
//   water_depth   = usable_height - (distance_cm - SENSOR_OFFSET_CM)
//   percent       = water_depth / usable_height * 100
// ============================================
float HCSR04Sensor::distanceToPercent(float distance_cm) {
    float water_depth = TANK_HEIGHT_CM - (distance_cm - SENSOR_OFFSET_CM);
    if (water_depth < 0.0)          water_depth = 0.0;
    if (water_depth > TANK_HEIGHT_CM) water_depth = TANK_HEIGHT_CM;

    return (water_depth / TANK_HEIGHT_CM) * 100.0;
}
