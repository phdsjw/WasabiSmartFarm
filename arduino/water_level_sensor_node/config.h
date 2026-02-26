/*
 * Wasabi SmartFarm - 수위 센서 노드 설정 파일
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */
#ifndef ARDUINO_WATER_LEVEL_SENSOR_NODE_CONFIG_H
#define ARDUINO_WATER_LEVEL_SENSOR_NODE_CONFIG_H
#include <Arduino.h>

// ============================================
// 노드 식별 정보
// ============================================
#define NODE_ID "20"  // 수위 센서 노드 번호

// ============================================
// WiFi 설정
// ============================================
#define WIFI_SSID        "YOUR_WIFI_SSID"       // WiFi SSID
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"   // WiFi 비밀번호
#define WIFI_TIMEOUT     10000                  // WiFi 연결 타임아웃 (10초)
#define WIFI_MAX_RETRY   5                      // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000               // 재시도 간격 (10초)

// ============================================
// MQTT 설정
// ============================================
#define MQTT_SERVER      "192.168.0.104"        // MQTT Broker IP
#define MQTT_PORT        1883                   // MQTT 포트
#define MQTT_USER        "wasabi_farm"                     // MQTT 사용자명 (인증 사용 시)
#define MQTT_PASSWORD    "1qazxsw2"                     // MQTT 비밀번호 (인증 사용 시)

// MQTT Client ID
#define MQTT_CLIENT_ID   "WasabiWaterLevel_20"

// MQTT Topics
#define MQTT_DATA_TOPIC       "sensor/water_level/data"
#define MQTT_HEARTBEAT_TOPIC  "sensor/water_level/heartbeat"
#define MQTT_STATUS_TOPIC     "sensor/water_level/status"

// ============================================
// 센서 읽기 주기 (밀리초)
// ============================================
#define SENSOR_READ_INTERVAL   3000    // 3초
#define HEARTBEAT_INTERVAL     60000   // 1분

// ============================================
// HC-SR04 센서 설정
// ============================================
// Wemos D1 R1 핀맵
#define TRIG_PIN  D1      // GPIO5 (D1)
#define ECHO_PIN  D2      // GPIO4 (D2)

// 측정 설정
#define MAX_DISTANCE_CM   400    // 최대 측정 거리 (cm)
#define MIN_DISTANCE_CM   2      // 최소 측정 거리 (cm)
#define MEASUREMENT_TIMEOUT  30000  // 타임아웃 (마이크로초, 30ms)

// 물탱크 설정
#define TANK_HEIGHT_CM    100.0  // 물탱크 높이 (cm) - 실제 값으로 변경 필요!
#define SENSOR_OFFSET_CM  5.0    // 센서 설치 오프셋 (센서에서 물탱크 바닥까지 거리)

// 필터링 설정
#define SAMPLE_COUNT      5      // 평균을 위한 샘플 수
#define SAMPLE_DELAY_MS   10     // 샘플 간 딜레이 (ms)

// ============================================
// LED 설정 (상태 표시)
// ============================================
// ESP8266 내장 LED는 LOW가 ON, HIGH가 OFF
#define LED_BLINK_INTERVAL  1000   // LED 깜빡임 간격 (1초)

// ============================================
// 센서 데이터 구조체
// ============================================
struct WaterLevelData {
    float distance_cm;          // 센서에서 수면까지 거리 (cm)
    float water_level_percent;  // 수위 퍼센트 (%)
    unsigned long timestamp;    // 타임스탬프 (ms)
    bool is_valid;              // 데이터 유효성
};

// ============================================
// 디버그 설정
// ============================================
#define DEBUG_MODE       true   // 시리얼 디버그 출력 활성화
#define SERIAL_BAUDRATE  115200 // 시리얼 통신 속도

#if DEBUG_MODE
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

#ifndef MQTT_WILL_TOPIC
#define MQTT_WILL_TOPIC "homeassistant/status"
#endif
#ifndef MQTT_WILL_MESSAGE
#define MQTT_WILL_MESSAGE "offline"
#endif
#ifndef MQTT_USER
#define MQTT_USER "wasabi_farm"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "1qazxsw2"
#endif
#endif // ARDUINO_WATER_LEVEL_SENSOR_NODE_CONFIG_H
