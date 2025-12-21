/*
 * Wasabi SmartFarm - 대기 센서 노드 설정 파일
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// 🏷️ 센서 노드 고유 ID (각 노드마다 변경 필요!)
// ============================================
#define ZONE_ID "01"  // Zone 01: 하우스 입구
// #define ZONE_ID "02"  // Zone 02: 하우스 중앙
// #define ZONE_ID "03"  // Zone 03: 하우스 후면

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
#define MQTT_SERVER "192.168.0.100"          // MQTT Broker IP
#define MQTT_PORT 1883                        // MQTT Broker 포트
#define MQTT_CLIENT_ID_PREFIX "WasabiAir_Zone"  // MQTT 클라이언트 ID 접두사
#define MQTT_TOPIC_PREFIX "sensor/air/zone"     // MQTT Topic 접두사

// MQTT 토픽 (자동 생성)
#define MQTT_TOPIC_DATA MQTT_TOPIC_PREFIX ZONE_ID "/data"        // 센서 데이터
#define MQTT_TOPIC_HEARTBEAT MQTT_TOPIC_PREFIX ZONE_ID "/heartbeat"  // 하트비트
#define MQTT_TOPIC_STATUS MQTT_TOPIC_PREFIX ZONE_ID "/status"    // 상태 정보

// ============================================
// ⏱️ 센서 읽기 주기
// ============================================
#define SENSOR_READ_INTERVAL 10000   // 센서 데이터 읽기 주기: 10초
#define HEARTBEAT_INTERVAL 60000     // 하트비트 전송 주기: 60초 (1분)

// ============================================
// 🌡️ SHT30 센서 설정
// ============================================
#define SHT30_I2C_ADDRESS 0x44       // SHT30 I2C 주소 (기본값: 0x44, 대체: 0x45)
#define SHT30_MEASUREMENT_REPEATABILITY 2  // 0=Low, 1=Medium, 2=High

// 측정 범위 (사양)
#define SHT30_TEMP_MIN -40.0         // 온도 최소값 (°C)
#define SHT30_TEMP_MAX 125.0         // 온도 최대값 (°C)
#define SHT30_HUMIDITY_MIN 0.0       // 습도 최소값 (%)
#define SHT30_HUMIDITY_MAX 100.0     // 습도 최대값 (%)

// 측정 정확도
#define SHT30_TEMP_ACCURACY 0.2      // 온도 정확도: ±0.2°C
#define SHT30_HUMIDITY_ACCURACY 2.0  // 습도 정확도: ±2%RH

// ============================================
// 🔧 하드웨어 핀 설정
// ============================================
// I2C 핀 (Arduino Uno R4 WiFi 기본 I2C)
// SDA: A4
// SCL: A5

// LED 핀 (상태 표시용)
#define LED_BUILTIN_PIN LED_BUILTIN  // 내장 LED (R4 WiFi: 13번)

// ============================================
// 🐛 디버그 설정
// ============================================
#define DEBUG_MODE true              // 디버그 모드 (true: 활성화, false: 비활성화)
#define SERIAL_BAUDRATE 115200       // 시리얼 통신 속도

// 디버그 매크로 (가변 인자 지원)
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
struct AirSensorData {
  float air_temp;       // 대기 온도 (°C)
  float air_humidity;   // 대기 습도 (%)
  unsigned long timestamp;  // 타임스탬프 (ms)
  bool is_valid;        // 데이터 유효성
};

#endif // CONFIG_H
