/*
 * Wasabi SmartFarm - 수위 센서 노드 설정 파일 (Home Assistant 버전)
 *
 * 하드웨어 : Arduino Uno R4 WiFi + HC-SR04 초음파 수위 센서
 * Node-RED 버전 대비 변경사항:
 *   - MQTT 인증 추가 (MQTT_USER / MQTT_PASSWORD)
 *   - LWT(Last Will) 지원 → HA availability 연동
 *   - 상태 payload "connected" → "online" / "offline" 로 변경
 *   - HA Discovery 옵션 추가
 *   - MQTT_SERVER 플레이스홀더 형식 통일
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */
#ifndef ARDUINO_WATER_LEVEL_SENSOR_NODE_R4_CONFIG_H
#define ARDUINO_WATER_LEVEL_SENSOR_NODE_R4_CONFIG_H
#include <Arduino.h>

// ============================================
// 노드 식별 정보
// ============================================
#define NODE_ID   "g3-01"
#define NODE_TYPE "water_level"

// ============================================
// WiFi 설정
// ============================================
#define WIFI_SSID           "YOUR_WIFI_SSID"       // ← 실제 WiFi 이름으로 변경
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"   // ← 실제 WiFi 비밀번호로 변경
#define WIFI_TIMEOUT        10000                  // WiFi 연결 타임아웃 (ms)
#define WIFI_MAX_RETRY      5                      // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000                  // 재시도 간격 (ms)

// ============================================
// MQTT 설정 (Home Assistant / Mosquitto)
// ============================================
#define MQTT_SERVER   "192.168.0.100"
#define MQTT_PORT     1883

// ⚠️ Mosquitto 브로커 인증 정보 (HA add-on 설정과 동일하게)
#define MQTT_USER     "wasabi_farm"          // ← HA Mosquitto 사용자명
#define MQTT_PASSWORD "your_secure_password" // ← 실제 비밀번호로 변경

// MQTT Client ID (고정)
#define MQTT_CLIENT_ID  "WasabiWaterLevel_" NODE_ID

// ============================================
// MQTT Topics
// ============================================
// ▶ 데이터 발행 토픽
//   Node-RED: "sensor/water_level/data"  (이전)
//   HA      : "sensor/water_tank/data"   (변경) ← mqtt.yaml state_topic 과 일치
#define MQTT_DATA_TOPIC      "smartfarm/wasabi/g3_drain_level/g3-01/data"

// ▶ 하트비트 토픽 (진단용)
#define MQTT_HEARTBEAT_TOPIC "smartfarm/wasabi/g3_drain_level/g3-01/heartbeat"

// ▶ Availability 토픽 (LWT 연동)
//   HA mqtt.yaml 의 availability_topic 과 반드시 일치
#define MQTT_STATUS_TOPIC    "smartfarm/wasabi/g3_drain_level/g3-01/status"

// ▶ Availability payload (HA 규격)
//   Node-RED: "connected" (이전)
//   HA      : "online" / "offline"  (변경)
#define MQTT_PAYLOAD_ONLINE  "online"
#define MQTT_PAYLOAD_OFFLINE "offline"

// ============================================
// Home Assistant Discovery (선택적)
// ============================================
// true 로 변경하면 HA 가 자동으로 센서 등록 (mqtt.yaml 항목 불필요)
// false 일 때는 mqtt.yaml 의 수동 항목이 사용됨
#define HA_DISCOVERY_ENABLED  false
#define HA_DISCOVERY_PREFIX   "homeassistant"
#define HA_DEVICE_NAME        "wasabi_water_level_20"
#define HA_DEVICE_FRIENDLY    "와사비 수위센서 Node20"

// ============================================
// 센서 읽기 주기
// ============================================
#define SENSOR_READ_INTERVAL  3000    // 데이터 발행 주기 (ms) — 3초
#define HEARTBEAT_INTERVAL    60000   // 하트비트 주기 (ms) — 1분

// ============================================
// HC-SR04 초음파 센서 핀 설정
// ============================================
#define TRIG_PIN  7    // D7 → Trigger
#define ECHO_PIN  8    // D8 → Echo

// ============================================
// HC-SR04 측정 파라미터
// ============================================
#define MAX_DISTANCE_CM      400     // 최대 측정 거리 (cm)
#define MIN_DISTANCE_CM      2       // 최소 측정 거리 (cm)
#define MEASUREMENT_TIMEOUT  30000   // 에코 수신 타임아웃 (µs)

// ============================================
// 물탱크 치수 설정  ← 실제 탱크에 맞게 수정 필요
// ============================================
#define TANK_HEIGHT_CM   100.0   // 탱크 내부 높이 (cm)
#define SENSOR_OFFSET_CM   5.0   // 센서 설치 오프셋
                                 // (센서 → 만수위 수면까지 거리)

// ============================================
// 측정값 필터링
// ============================================
#define SAMPLE_COUNT     5    // 이동평균 샘플 수
#define SAMPLE_DELAY_MS  10   // 샘플 간 딜레이 (ms)

// ============================================
// 센서 데이터 구조체
// ============================================
struct WaterLevelData {
    float    distance_cm;         // 센서 → 수면 거리 (cm)
    float    water_level_cm;      // 실제 수위 높이 (cm)
    float    water_level_percent; // 수위 퍼센트 (%)
    bool     is_valid;            // 측정값 유효 여부
    unsigned long timestamp;      // 측정 시각 (millis)
};

// ============================================
// LED 상태 표시
// ============================================
#define LED_BLINK_INTERVAL  1000   // 깜빡임 간격 (ms)

// ============================================
// 디버그 설정
// ============================================
#define DEBUG_MODE      true    // false 로 변경하면 시리얼 출력 비활성화
#define SERIAL_BAUDRATE 115200

#if DEBUG_MODE
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

#ifndef MQTT_WILL_TOPIC
#define MQTT_WILL_TOPIC "homeassistant/status"
#endif
#ifndef MQTT_WILL_MESSAGE
#define MQTT_WILL_MESSAGE "offline"
#endif
#ifndef MQTT_USER
#define MQTT_USER "ha_user"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "ha_password"
#endif
#endif // ARDUINO_WATER_LEVEL_SENSOR_NODE_R4_CONFIG_H
