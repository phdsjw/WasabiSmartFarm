/*
 * Wasabi SmartFarm - 액추에이터 노드 설정 파일
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */
#ifndef ARDUINO_ACTUATOR_NODE_CONFIG_H
#define ARDUINO_ACTUATOR_NODE_CONFIG_H
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
#define MQTT_SERVER "192.168.0.103"          // MQTT Broker IP
#define MQTT_PORT 1883                        // MQTT Broker 포트

// 승인된 노드 ID 규칙
#define NODE_ID "g6-01"

// MQTT 클라이언트 ID
#define MQTT_CLIENT_ID "WasabiActuator_" NODE_ID

// MQTT 구독 토픽 (명령 수신)
#define TOPIC_BASE "smartfarm/wasabi/g6_pump_ctrl/" NODE_ID
#define MQTT_TOPIC_IRRIGATION TOPIC_BASE "/cmd/irrigation"
#define MQTT_TOPIC_DRAINAGE TOPIC_BASE "/cmd/drainage"
#define MQTT_TOPIC_FAN TOPIC_BASE "/cmd/fan"
#define MQTT_TOPIC_LED TOPIC_BASE "/cmd/led"
#define MQTT_TOPIC_EMERGENCY_STOP TOPIC_BASE "/cmd/emergency_stop"
#define MQTT_TOPIC_EMERGENCY_RELEASE TOPIC_BASE "/cmd/emergency_release"
#define MQTT_TOPIC_RESET TOPIC_BASE "/cmd/reset"

// MQTT 발행 토픽 (상태 전송)
#define MQTT_TOPIC_STATUS TOPIC_BASE "/status"
#define MQTT_TOPIC_HEARTBEAT TOPIC_BASE "/heartbeat"
#define MQTT_TOPIC_STATE TOPIC_BASE "/state"

// ============================================
// ⏱️ 타이머 설정
// ============================================
#define HEARTBEAT_INTERVAL 10000     // 하트비트 전송 주기: 10초
#define STATE_REPORT_INTERVAL 5000   // 상태 리포트 주기: 5초

// ============================================
// 🔧 릴레이 핀 설정 (4채널 릴레이 모듈)
// ============================================
#define RELAY_CH1_PIN 7              // 채널 1: 관수 펌프 (2HP)
#define RELAY_CH2_PIN 8              // 채널 2: 배수 펌프 (1HP)
#define RELAY_CH3_PIN 9              // 채널 3: 천장 팬 (예비)
#define RELAY_CH4_PIN 10             // 채널 4: LED 조명 (예비)

// 릴레이 활성화 레벨 (LOW = 활성화, HIGH = 비활성화)
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// ============================================
// 🚨 안전 설정
// ============================================
// 타임아웃 설정 (자동 종료)
#define IRRIGATION_TIMEOUT 300000    // 관수 펌프 타임아웃: 5분 (300초)
#define DRAINAGE_TIMEOUT 300000      // 배수 펌프 타임아웃: 5분 (300초)
#define FAN_TIMEOUT 3600000          // 팬 타임아웃: 60분 (1시간)
#define LED_TIMEOUT 43200000         // LED 타임아웃: 12시간

// 최소 ON 시간 (채터링 방지)
#define MIN_ON_TIME 1000             // 최소 ON 시간: 1초

// 동시 작동 방지
#define ALLOW_SIMULTANEOUS_PUMPS false  // 관수/배수 펌프 동시 작동 방지

// 긴급 정지 후 복구 시간
#define EMERGENCY_COOLDOWN 5000      // 긴급 정지 후 5초 대기

// ============================================
// 📊 액추에이터 상태 구조체
// ============================================
struct ActuatorState {
  // 릴레이 상태
  bool irrigation_pump;     // 관수 펌프 상태
  bool drainage_pump;       // 배수 펌프 상태
  bool fan;                 // 팬 상태
  bool led;                 // LED 상태
  
  // 시작 시간 (타임아웃 체크용)
  unsigned long irrigation_start_time;
  unsigned long drainage_start_time;
  unsigned long fan_start_time;
  unsigned long led_start_time;
  
  // 긴급 정지 상태
  bool emergency_stop;
  unsigned long emergency_stop_time;
  
  // 통계
  unsigned long total_irrigation_time;  // 총 관수 시간 (ms)
  unsigned long total_drainage_time;    // 총 배수 시간 (ms)
  unsigned int irrigation_count;        // 관수 횟수
  unsigned int drainage_count;          // 배수 횟수
};

// ============================================
// 🔧 하드웨어 핀 설정
// ============================================
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
  // DEBUG_PRINTF는 제거 (Serial.printf() 미지원 - Arduino Uno R4 WiFi)
  // 대신 DEBUG_PRINT + DEBUG_PRINTLN 조합 사용
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
#define MQTT_PASSWORD "your_secure_password"
#endif
#endif // ARDUINO_ACTUATOR_NODE_CONFIG_H
