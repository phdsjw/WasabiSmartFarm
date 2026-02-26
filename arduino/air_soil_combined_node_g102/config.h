/*
 * 토양 센서 노드 설정 파일 (Home Assistant 버전)
 * 
 * 18개 노드에서 공통으로 사용
 * TANK_ID만 각 노드마다 변경하여 업로드
 * 
 * 변경사항:
 * - MQTT 인증 추가 (MQTT_USER, MQTT_PASSWORD)
 * - Home Assistant Discovery 옵션
 */

#ifndef CONFIG_H
#define CONFIG_H

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
#define WIFI_SSID        "kunlim"       // WiFi SSID
#define WIFI_PASSWORD    "1qazxsw212"   // WiFi 비밀번호
#define WIFI_TIMEOUT     10000                  // WiFi 연결 타임아웃 (10초)
#define WIFI_MAX_RETRY   5                      // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000               // 재시도 간격 (10초)

// ============================================
// MQTT 설정 (Home Assistant용)
// ============================================
#define MQTT_SERVER      "192.168.0.103"        // Home Assistant IP 주소
#define MQTT_PORT        1883                   // MQTT 포트

// ⚠️ Home Assistant Mosquitto 브로커 인증 정보
#define MQTT_USER        "kunlim"          // MQTT 사용자명
#define MQTT_PASSWORD    "1qazxsw212" // MQTT 비밀번호

// MQTT Client ID (자동 생성: WasabiSoil_Tank01)
#define MQTT_CLIENT_ID_PREFIX  "WasabiSoil_Tank"

// MQTT Topic (자동 생성: sensor/soil/tank01/data)
// ※ 기존 Node-RED와 동일한 토픽 구조 유지
#define MQTT_TOPIC_PREFIX      "sensor/soil/tank"
#define MQTT_TOPIC_SUFFIX      "/data"

// MQTT 하트비트 Topic
#define MQTT_HEARTBEAT_SUFFIX  "/heartbeat"

// MQTT Availability Topic (Home Assistant용)
#define MQTT_STATUS_SUFFIX     "/status"

// ============================================
// Home Assistant Discovery (선택적)
// ============================================
// true로 설정하면 Home Assistant에서 자동으로 센서 인식
#define HA_DISCOVERY_ENABLED   false
#define HA_DISCOVERY_PREFIX    "homeassistant"
#define HA_DEVICE_NAME_PREFIX  "wasabi_soil_"

// ============================================
// 센서 읽기 주기 (밀리초)
// ============================================
#define SENSOR_READ_INTERVAL   10000   // 10초
// ============================================
// 대기 센서 (SHT30) 설정
// ============================================
#define SHT30_I2C_ADDRESS      0x44    // SHT30 I2C 주소 (기본값: 0x44)

#define HEARTBEAT_INTERVAL     60000   // 1분

// ============================================
// Modbus RTU 설정 (SEN0604)
// ============================================
#define MODBUS_SLAVE_ID        1       // SEN0604 기본 Slave ID
#define MODBUS_BAUDRATE        4800    // SEN0604 보드레이트
#define MODBUS_TIMEOUT         1000    // Modbus 응답 타임아웃 (1초)

// RS485 핀 설정 (Arduino Uno R4 + DFR0259)
#define RS485_TX_ENABLE_PIN    2       // RS485 TX Enable 핀 (DE/RE)

// ============================================
// SEN0604 Modbus 레지스터 주소
// ============================================
#define REG_SOIL_MOISTURE      0x0000  // 토양 습도 (0~100%)
#define REG_SOIL_TEMP          0x0001  // 토양 온도 (°C)
#define REG_SOIL_EC            0x0002  // 토양 EC (μS/cm)
#define REG_SOIL_PH            0x0003  // 토양 pH

// 읽을 레지스터 개수
#define NUM_REGISTERS          4       // 습도, 온도, EC, pH

// ============================================
// 센서 데이터 구조체
// ============================================
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
#define MQTT_USER "ha_user"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "ha_password"
#endif
#endif // CONFIG_H
