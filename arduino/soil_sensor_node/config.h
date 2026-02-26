/*
 * 토양 센서 노드 설정 파일
 * 
 * 18개 노드에서 공통으로 사용
 * TANK_ID만 각 노드마다 변경하여 업로드
 */
#ifndef ARDUINO_SOIL_SENSOR_NODE_CONFIG_H
#define ARDUINO_SOIL_SENSOR_NODE_CONFIG_H
#include <Arduino.h>

// ============================================
// 노드 식별 정보 (각 노드마다 변경 필요)
// ============================================
#define TANK_ID "01"  // 01~18 중 하나로 설정
// Tank 01 = "01"
// Tank 02 = "02"
// ...
// Tank 18 = "18"

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
#define MQTT_SERVER      "192.168.0.103"        // MQTT Broker IP
#define MQTT_PORT        1883                   // MQTT 포트
#define MQTT_USER        "wasabi_farm"                     // MQTT 사용자명 (인증 사용 시)
#define MQTT_PASSWORD    "your_secure_password"                     // MQTT 비밀번호 (인증 사용 시)

// MQTT Client ID (자동 생성: WasabiSoil_Tank01)
#define MQTT_CLIENT_ID_PREFIX  "WasabiSoil_"

// MQTT Topic (승인된 구조)
#define TOPIC_BASE             "smartfarm/wasabi/g2_soil/" NODE_ID
#define MQTT_TOPIC_DATA        TOPIC_BASE "/data"
#define MQTT_TOPIC_HEARTBEAT   TOPIC_BASE "/heartbeat"

// ============================================
// 센서 읽기 주기 (밀리초)
// ============================================
#define SENSOR_READ_INTERVAL   10000   // 10초
#define HEARTBEAT_INTERVAL     60000   // 1분

// ============================================
// Modbus RTU 설정 (SEN0604)
// ============================================
#define MODBUS_SLAVE_ID        1       // SEN0604 기본 Slave ID (변경 가능)
#define MODBUS_BAUDRATE        4800    // SEN0604 보드레이트 (지원: 2400/4800/9600, 공장 기본 9600)
#define MODBUS_TIMEOUT         1000    // Modbus 응답 타임아웃 (1초)

// RS485 핀 설정 (Arduino Uno R4 + DFR0259)
// Serial1 사용 (TX: D1, RX: D0)
#define RS485_TX_ENABLE_PIN    2       // RS485 TX Enable 핀 (DE/RE)

// ============================================
// SEN0604 Modbus 레지스터 주소
// ============================================
#define REG_SOIL_MOISTURE      0x0000  // 토양 습도 (0~100%)
#define REG_SOIL_TEMP          0x0001  // 토양 온도 (°C)
#define REG_SOIL_EC            0x0002  // 토양 EC (μS/cm)
#define REG_SOIL_PH            0x0003  // 토양 pH (0.01 pH)
#define REG_SOIL_NITROGEN      0x0004  // 질소 (mg/kg) - 옵션
#define REG_SOIL_PHOSPHORUS    0x0005  // 인 (mg/kg) - 옵션
#define REG_SOIL_POTASSIUM     0x0006  // 칼륨 (mg/kg) - 옵션

// 읽을 레지스터 개수
#define NUM_REGISTERS          4       // 습도, 온도, EC, pH

// ============================================
// 센서 데이터 구조체
// ============================================
struct SoilSensorData {
    float soil_moisture;    // 토양 습도 (%)
    float soil_temp;        // 토양 온도 (°C) - 음수 지원
    float soil_ec;          // 토양 EC (μS/cm)
    float soil_ph;          // 토양 pH
    bool valid;             // 데이터 유효성
    unsigned long timestamp; // 측정 시간
};

// ============================================
// 디버그 설정
// ============================================
#define DEBUG_MODE       true   // 시리얼 디버그 출력 활성화

#if DEBUG_MODE
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

// ============================================
// LED 설정 (상태 표시)
// ============================================
#define LED_BUILTIN_PIN  LED_BUILTIN  // 내장 LED
#define LED_BLINK_INTERVAL  1000       // LED 깜빡임 간격 (1초)

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
#endif // ARDUINO_SOIL_SENSOR_NODE_CONFIG_H
#define NODE_ID "g2-01"
