/*
 * Wasabi SmartFarm - 수조 센서 노드 설정 파일
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef CONFIG_H
#define CONFIG_H

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

// 승인된 노드 ID 규칙
#define NODE_ID "g4-01"

// MQTT 클라이언트 ID
#define MQTT_CLIENT_ID "WasabiWater_" NODE_ID

// MQTT 토픽 (승인된 구조)
#define TOPIC_BASE "smartfarm/wasabi/g4_water_quality/" NODE_ID
#define MQTT_TOPIC_DATA TOPIC_BASE "/data"
#define MQTT_TOPIC_HEARTBEAT TOPIC_BASE "/heartbeat"
#define MQTT_TOPIC_STATUS TOPIC_BASE "/status"

// ============================================
// ⏱️ 센서 읽기 주기
// ============================================
#define SENSOR_READ_INTERVAL 10000   // 센서 데이터 읽기 주기: 10초
#define HEARTBEAT_INTERVAL 60000     // 하트비트 전송 주기: 60초 (1분)

// ============================================
// 🌡️ DS18B20 수온 센서 설정 (1-Wire)
// ============================================
#define ONE_WIRE_BUS_PIN 4           // 1-Wire 버스 핀 (D4)
#define TEMPERATURE_PRECISION 12     // 온도 해상도 (9, 10, 11, 12 비트)
#define DS18B20_CONVERSION_TIME 750  // 변환 시간 (ms, 12비트: 750ms)

// 측정 범위
#define WATER_TEMP_MIN -10.0         // 최소 수온 (°C)
#define WATER_TEMP_MAX 50.0          // 최대 수온 (°C)

// ============================================
// 📊 아날로그 센서 핀 설정
// ============================================
#define PH_SENSOR_PIN A0             // pH 센서 (SEN0161)
#define TDS_SENSOR_PIN A1            // TDS 센서 (SEN0244)
#define EC_SENSOR_PIN A2             // EC 센서 (SEN0451 Pro)

// ============================================
// 🧪 pH 센서 설정 (SEN0161)
// ============================================
#define PH_VOLTAGE_REF 5.0           // 기준 전압 (5V)
#define PH_ADC_RESOLUTION 1024       // ADC 해상도 (10비트)
#define PH_CALIBRATION_OFFSET 0.0    // pH 보정 오프셋
#define PH_CALIBRATION_SLOPE 3.5     // pH 보정 기울기 (기본값: 3.5)

// 측정 범위
#define PH_MIN 0.0                   // 최소 pH
#define PH_MAX 14.0                  // 최대 pH

// 정상 범위 (경고용)
#define PH_NORMAL_MIN 5.5            // 정상 pH 최소값
#define PH_NORMAL_MAX 8.5            // 정상 pH 최대값

// ============================================
// 💧 TDS 센서 설정 (SEN0244)
// ============================================
#define TDS_VOLTAGE_REF 5.0          // 기준 전압 (5V)
#define TDS_ADC_RESOLUTION 1024      // ADC 해상도 (10비트)
#define TDS_K_VALUE 1.0              // TDS 보정 계수 (기본값: 1.0)
#define TDS_COMPENSATION_COEFFICIENT 1.0  // 온도 보상 계수

// 측정 범위
#define TDS_MIN 0                    // 최소 TDS (ppm)
#define TDS_MAX 1000                 // 최대 TDS (ppm)

// 정상 범위 (경고용)
#define TDS_NORMAL_MIN 300           // 정상 TDS 최소값 (ppm)
#define TDS_NORMAL_MAX 600           // 정상 TDS 최대값 (ppm)

// ============================================
// ⚡ EC 센서 설정 (SEN0451 Pro)
// ============================================
#define EC_VOLTAGE_REF 5.0           // 기준 전압 (5V)
#define EC_ADC_RESOLUTION 1024       // ADC 해상도 (10비트)
#define EC_K_VALUE 1.0               // EC 보정 계수 (기본값: 1.0)
#define EC_REFERENCE_TEMP 25.0       // 기준 온도 (°C)
#define EC_TEMP_COEFFICIENT 0.019    // 온도 보상 계수 (1.9% per °C)

// 측정 범위
#define EC_MIN 0.0                   // 최소 EC (mS/cm)
#define EC_MAX 20.0                  // 최대 EC (mS/cm)

// 정상 범위 (경고용)
#define EC_NORMAL_MIN 0.8            // 정상 EC 최소값 (mS/cm)
#define EC_NORMAL_MAX 2.0            // 정상 EC 최대값 (mS/cm)

// ============================================
// 📈 센서 데이터 필터링
// ============================================
#define ANALOG_SAMPLE_COUNT 10       // 아날로그 센서 샘플링 횟수
#define ANALOG_SAMPLE_DELAY 10       // 샘플 간 딜레이 (ms)

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
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// ============================================
// 📊 센서 데이터 구조체
// ============================================
struct WaterTankSensorData {
  float water_temp;     // 수온 (°C)
  float water_ph;       // pH (0~14)
  int water_tds;        // TDS (ppm)
  float water_ec;       // EC (mS/cm)
  unsigned long timestamp;  // 타임스탬프 (ms)
  bool is_valid;        // 데이터 유효성
};

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
