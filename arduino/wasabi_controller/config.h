/*
 * Configuration File
 * WiFi, MQTT, Sensor Settings
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi 설정
// ============================================
#define WIFI_SSID        "YOUR_WIFI_SSID"      // WiFi SSID로 변경하세요
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"  // WiFi 비밀번호로 변경하세요
#define WIFI_TIMEOUT     10000                 // WiFi 연결 타임아웃 (10초)
#define WIFI_MAX_RETRY   5                     // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000              // 재시도 간격 (10초)

// ============================================
// MQTT 설정
// ============================================
#define MQTT_SERVER      "192.168.0.100"       // MQTT Broker IP (Node-RED 서버)
#define MQTT_PORT        1883                  // MQTT 포트
#define MQTT_CLIENT_ID   "WasabiSmartFarm_Arduino"
#define MQTT_USER        ""                    // MQTT 사용자명 (인증 사용 시)
#define MQTT_PASSWORD    ""                    // MQTT 비밀번호 (인증 사용 시)

// ============================================
// 센서 읽기 주기 (밀리초)
// ============================================
#define SENSOR_READ_INTERVAL       10000  // 환경 센서: 10초
#define WATER_LEVEL_READ_INTERVAL  3000   // 수위 센서: 3초
#define HEARTBEAT_INTERVAL         60000  // 하트비트: 1분

// ============================================
// 하드웨어 핀 설정
// ============================================

// I2C (SHT30 온습도 센서)
// SDA: A4 (기본)
// SCL: A5 (기본)

// 1-Wire (DS18B20 수온 센서)
#define ONE_WIRE_PIN     4

// Analog 핀 (물탱크 센서)
#define PH_SENSOR_PIN    A0
#define TDS_SENSOR_PIN   A1
#define EC_SENSOR_PIN    A2

// RS485 (Modbus RTU - 토양 센서)
// Serial1 사용 (D0, D1은 USB Serial과 충돌하므로 사용 안 함)
// D2: RS485 TX
// D3: RS485 RX
// RS485 TX Enable 핀이 필요한 경우 추가

// 수위 센서 핀 (Analog 또는 Digital)
#define WATER_LEVEL_TANK01  A3
// 수위 센서가 18개이므로 아날로그 멀티플렉서 필요 (CD74HC4067 등)

// ============================================
// 센서 설정
// ============================================

// 탱크 개수
#define TANK_COUNT      18

// Modbus RTU Slave ID (토양 센서)
#define SOIL_SENSOR_START_ID  1  // Tank 1~18 = Slave ID 1~18

// SHT30 I2C 주소
#define SHT30_I2C_ADDRESS  0x44

// 센서 보정값 (필요 시 수정)
#define PH_CALIBRATION_OFFSET    0.0
#define TDS_CALIBRATION_FACTOR   1.0
#define EC_CALIBRATION_FACTOR    1.0

// ============================================
// MQTT 토픽 정의
// ============================================

// 센서 데이터 Publish 토픽
#define TOPIC_ENVIRONMENT      "sensor/environment"
#define TOPIC_WATER_LEVEL      "watering/tank%02d/level"      // %02d = 01, 02, ...
#define TOPIC_SOIL_DATA        "sensor/soil/tank%02d/data"
#define TOPIC_HEARTBEAT        "system/heartbeat"

// 제어 명령 Subscribe 토픽 (Step 2 이후 사용)
// #define TOPIC_WATERING_PUMP_ON   "watering/watering_pump/on"
// #define TOPIC_WATERING_PUMP_OFF  "watering/watering_pump/off"

// ============================================
// 디버그 설정
// ============================================
#define DEBUG_MODE       true   // 시리얼 디버그 출력 활성화

// 디버그 매크로 (가변 인자 지원)
#if DEBUG_MODE
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

#endif // CONFIG_H
