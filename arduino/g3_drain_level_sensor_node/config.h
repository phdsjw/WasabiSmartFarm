/*
 * Wasabi SmartFarm - 배수수위 센서 노드 설정 파일
 * 
 * 기능: HC-SR04 초음파 센서로 배수槽 수위 측정
 * Hardware: Arduino Uno R4 WiFi + HC-SR04
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25_H
#define ARDUINO_G3_DRAIN_LEVEL_SENSOR_NODE_CONFIG_H
 */
#ifndef ARDUINO_G3_DRAIN_LEVEL_SENSOR_NODE_CONFIG_H
// ============================================
// 🏷️ 노드 식별 정보 (g3-배수수위)
// ============================================
#define NODE_ID "g3-01"

// ============================================
// 📡 WiFi 설정
// ============================================
#define WIFI_SSID "your_wifi_ssid"           // WiFi SSID
#define WIFI_PASSWORD "your_wifi_password"   // WiFi 비밀번호
#define WIFI_TIMEOUT 10000                   // WiFi 연결 타임아웃 (10초)
#define WIFI_MAX_RETRY 5                     // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000            // 재시도 간격 (10초)

// ============================================
// 🔌 MQTT 설정
// ============================================
#define MQTT_SERVER "192.168.0.104"          // MQTT Broker IP
#define MQTT_PORT 1883                        // MQTT Broker 포트
#define MQTT_CLIENT_ID_PREFIX "WasabiDrain_"  // MQTT 클라이언트 ID 접두사

// MQTT 토픽 (g3-배수수위)
#define TOPIC_BASE "smartfarm/wasabi/g3_drain_level/" NODE_ID
#define MQTT_TOPIC_DATA TOPIC_BASE "/data"
#define MQTT_TOPIC_HEARTBEAT TOPIC_BASE "/heartbeat"
#define MQTT_TOPIC_STATUS TOPIC_BASE "/status"

// ============================================
// ⏱️ 센서 읽기 주기
// ============================================
#define SENSOR_READ_INTERVAL 5000    // 센서 데이터 읽기 주기: 5초
#define HEARTBEAT_INTERVAL 60000   // 하트비트 전송 주기: 60초 (1분)

// ============================================
// HC-SR04 센서 설정 (Arduino Uno R4 WiFi)
// ============================================
#define TRIG_PIN 2       // GPIO2 (D2)
#define ECHO_PIN 3       // GPIO3 (D3)

// 측정 설정
#define MAX_DISTANCE_CM   400    // 최대 측정 거리 (cm)
#define MIN_DISTANCE_CM   2     // 최소 측정 거리 (cm)
#define MEASUREMENT_TIMEOUT  30000  // 타임아웃 (마이크로초, 30ms)

// 물탱크/배수槽 설정
#define TANK_HEIGHT_CM    100.0  // 물탱크 높이 (cm) - 실제 값으로 변경 필요!
#define SENSOR_OFFSET_CM  5.0    // 센서 설치 오프셋

// 필터링 설정
#define SAMPLE_COUNT      5      // 평균을 위한 샘플 수
#define SAMPLE_DELAY_MS   10     // 샘플 간 딜레이 (ms)

// ============================================
// 🔧 하드웨어 핀 설정
// ============================================
#define LED_BUILTIN_PIN LED_BUILTIN  // 내장 LED (R4 WiFi: 13번)

// ============================================
// 🐛 디버그 설정
// ============================================
#define DEBUG_MODE true              // 디버그 모드
#define SERIAL_BAUDRATE 115200       // 시리얼 통신 속도

#if DEBUG_MODE
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// ============================================
// 📊 센서 데이터 구조체
// ============================================
struct WaterLevelData {
    float distance_cm;           // 센서에서 수면까지 거리 (cm)
    float water_level_percent;  // 수위 퍼센트 (%)
    unsigned long timestamp;     // 타임스탬프 (ms)
    bool is_valid;              // 데이터 유효성
};

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
#define MQTT_PASSWORD "your_secure_password"
#endif
#endif // ARDUINO_G3_DRAIN_LEVEL_SENSOR_NODE_CONFIG_H
