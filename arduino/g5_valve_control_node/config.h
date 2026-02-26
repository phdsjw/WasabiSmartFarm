/*
 * Wasabi SmartFarm - 밸브 제어 노드 설정 파일
 * 
 * 기능: 수위 танк 뺄브 제어 (on/off)
 * Hardware: Arduino Uno R4 WiFi + Single Channel Relay
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// 🏷️ 노드 식별 정보 (g5-밸브제어)
// ============================================
#define NODE_ID "g5-01"

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
#define MQTT_CLIENT_ID_PREFIX "WasabiValve_"  // MQTT 클라이언트 ID 접두사

// MQTT 토픽 (g5-밸브제어)
#define TOPIC_BASE "smartfarm/wasabi/g5_water_level_valve/" NODE_ID
#define MQTT_TOPIC_CMD TOPIC_BASE "/valve/cmd"
#define MQTT_TOPIC_STATE TOPIC_BASE "/valve/state"
#define MQTT_TOPIC_HEARTBEAT TOPIC_BASE "/heartbeat"
#define MQTT_TOPIC_STATUS TOPIC_BASE "/status"

// ============================================
// ⏱️ 타이머 설정
// ============================================
#define HEARTBEAT_INTERVAL 60000     // 하트비트 전송 주기: 60초

// ============================================
// 🔧 하드웨어 핀 설정 (Single Channel Relay)
// ============================================
#define VALVE_RELAY_PIN 7    // 밸브 릴레이 핀

// 릴레이 활성화 레벨 (LOW = 활성화, HIGH = 비활성화)
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// ============================================
// 🔧 하드웨어 핀 설정
// ============================================
#define LED_BUILTIN_PIN LED_BUILTIN  // 내장 LED (R4 WiFi: 13번)

// ============================================
// 📊 밸브 상태 구조체
// ============================================
struct ValveState {
  bool valve_open;              // 밸브 상태 (true = 열림, false = 닫힘)
  unsigned long valve_start_time;  // 밸브가 열린 시간
  unsigned long total_open_time;   // 총 열린 시간 (ms)
  unsigned int open_count;         // 열린 횟수
};

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
#endif // CONFIG_H
