/*
 * Wasabi SmartFarm - 대기+토양 통합 센서 노드 설정 파일
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */
#ifndef ARDUINO_AIR_SOIL_COMBINED_NODE_G101_CONFIG_H
#define ARDUINO_AIR_SOIL_COMBINED_NODE_G101_CONFIG_H
#include <Arduino.h>

// ============================================
// 노드 식별 정보 (각 노드마다 변경 필요!)
// ============================================
#define ZONE_ID "1"  // 기본 구역 ID (g101)
#define TANK_ID "1"  // 기본 탱크 ID (g101)

// 승인된 노드 ID 규칙
#define NODE_ID "g1-01"



// ============================================
// WiFi 설정
// ============================================
#define WIFI_SSID        "kunlim"       // WiFi SSID
#define WIFI_PASSWORD    "1qazxsw212"   // WiFi 비밀번호
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
#define MQTT_CLIENT_ID_PREFIX  "WasabiCombined_"

// MQTT Topics (승인된 구조)
#define TOPIC_BASE             "smartfarm/wasabi/g1_air_soil/" NODE_ID
#define MQTT_TOPIC_AIR_DATA    TOPIC_BASE "/air/data"
#define MQTT_TOPIC_SOIL_DATA   TOPIC_BASE "/soil/data"
#define MQTT_TOPIC_HEARTBEAT   TOPIC_BASE "/heartbeat"
#define MQTT_TOPIC_STATUS      TOPIC_BASE "/status"

// ============================================
// 센서 읽기 주기 (밀리초)
// ============================================
#define SENSOR_READ_INTERVAL   10000   // 10초
#define HEARTBEAT_INTERVAL     60000   // 1분

// ============================================
// SHT30 센서 설정 (대기 온습도)
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
// SEN0604 Modbus RTU 설정 (토양 센서)
// ============================================
#define MODBUS_SLAVE_ID        1       // SEN0604 기본 Slave ID (변경 가능)
#define MODBUS_BAUDRATE        4800    // SEN0604 보드레이트 (지원: 2400/4800/9600)
#define MODBUS_TIMEOUT         1000    // Modbus 응답 타임아웃 (1초)

// RS485 핀 설정 (Arduino Uno R4 + DFR0259)
// Serial1 사용 (TX: D1, RX: D0)
#define RS485_TX_ENABLE_PIN    2       // RS485 TX Enable 핀 (DE/RE)

// SEN0604 Modbus 레지스터 주소
#define REG_SOIL_MOISTURE      0x0000  // 토양 습도 (0~100%)
#define REG_SOIL_TEMP          0x0001  // 토양 온도 (°C)
#define REG_SOIL_EC            0x0002  // 토양 EC (μS/cm)
#define REG_SOIL_PH            0x0003  // 토양 pH (0.01 pH)

// 읽을 레지스터 개수
#define NUM_REGISTERS          4       // 습도, 온도, EC, pH

// ============================================
// 하드웨어 핀 설정
// ============================================
// I2C 핀 (Arduino Uno R4 WiFi 기본 I2C)
// SDA: A4 (SHT30)
// SCL: A5 (SHT30)

// RS485 핀 (Serial1)
// TX: D1 (SEN0604)
// RX: D0 (SEN0604)
// DE/RE: D2 (SEN0604)

// LED 핀 (상태 표시용)
#define LED_BUILTIN_PIN  LED_BUILTIN  // 내장 LED
#define LED_BLINK_INTERVAL  1000       // LED 깜빡임 간격 (1초)

// ============================================
// 센서 데이터 구조체
// ============================================
// 대기 센서 데이터
struct AirSensorData {
    float air_temp;       // 대기 온도 (°C)
    float air_humidity;   // 대기 습도 (%)
    unsigned long timestamp;  // 타임스탬프 (ms)
    bool is_valid;        // 데이터 유효성
};

// 토양 센서 데이터
struct SoilSensorData {
    float soil_moisture;    // 토양 습도 (%)
    float soil_temp;        // 토양 온도 (°C)
    float soil_ec;          // 토양 EC (μS/cm)
    float soil_ph;          // 토양 pH
    bool valid;             // 데이터 유효성
    unsigned long timestamp; // 측정 시간
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

#ifndef ZONE_ID
#error "ZONE_ID must be defined"
#endif
#ifndef TANK_ID
#error "TANK_ID must be defined"
#endif
#ifndef NODE_ID
#error "NODE_ID must be defined"
#endif
#ifndef SERIAL_BAUDRATE
#error "SERIAL_BAUDRATE must be defined"
#endif
#ifndef SHT30_I2C_ADDRESS
#error "SHT30_I2C_ADDRESS must be defined"
#endif
#ifndef SHT30_TEMP_MIN
#error "SHT30_TEMP_MIN must be defined"
#endif
#ifndef SHT30_TEMP_MAX
#error "SHT30_TEMP_MAX must be defined"
#endif
#ifndef SHT30_HUMIDITY_MIN
#error "SHT30_HUMIDITY_MIN must be defined"
#endif
#ifndef SHT30_HUMIDITY_MAX
#error "SHT30_HUMIDITY_MAX must be defined"
#endif

#endif // ARDUINO_AIR_SOIL_COMBINED_NODE_G101_CONFIG_H
