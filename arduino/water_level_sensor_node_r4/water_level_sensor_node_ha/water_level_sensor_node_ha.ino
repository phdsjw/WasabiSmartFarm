/*
 * Wasabi SmartFarm - 수위 센서 노드 메인 스케치 (Home Assistant 버전)
 *
 * 하드웨어 : Arduino Uno R4 WiFi + HC-SR04 초음파 센서
 *
 * Node-RED 버전 대비 변경사항:
 *   - config.h  → config_ha.h  (MQTT 인증, LWT, HA 토픽)
 *   - mqtt_handler.h/cpp → mqtt_handler_ha.h/cpp
 *   - hcsr04_sensor.h/cpp → hcsr04_sensor_ha.h/cpp
 *   - loop(): LED 상태 구분 (WiFi 끊김 / MQTT 끊김 / 정상) 개선
 *   - publishStatus("online") → HA availability 연동
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */

#include "config_ha.h"
#include "hcsr04_sensor_ha.h"
#include "mqtt_handler_ha.h"

// ── 전역 객체 ──────────────────────────────────
HCSR04Sensor waterLevelSensor;
MQTTHandler  mqttHandler;

// ── 타이머 변수 ────────────────────────────────
unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat  = 0;
unsigned long lastLedBlink   = 0;

bool ledState = false;

// ============================================
// setup()
// ============================================
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial && millis() < 3000);   // USB 시리얼 연결 대기 (최대 3초)

    printStartupMessage();

    // ── HC-SR04 초기화 ──────────────────────
    Serial.println(F("\n[SETUP] Initializing HC-SR04 sensor..."));
    waterLevelSensor.begin();
    Serial.println(F("[SETUP] HC-SR04 initialized"));

    // ── MQTT 핸들러 초기화 (WiFi + MQTT 연결) ─
    Serial.println(F("\n[SETUP] Initializing MQTT handler (HA Edition)..."));
    if (!mqttHandler.begin()) {
        Serial.println(F("[ERROR] MQTT handler init failed!"));
        Serial.println(F("[ERROR] Check config_ha.h:"));
        Serial.println(F("        WIFI_SSID / WIFI_PASSWORD"));
        Serial.println(F("        MQTT_SERVER / MQTT_USER / MQTT_PASSWORD"));
        Serial.println(F("[SETUP] Restarting in 5 seconds..."));
        delay(5000);
        NVIC_SystemReset();   // Arduino Uno R4 WiFi 소프트 리셋
    }
    Serial.println(F("[SETUP] MQTT handler initialized"));

    // ── 초기 상태 발행 ──────────────────────
    // publishStatus("online") 은 connectMQTT() 성공 시 이미 호출됨
    // 아래는 추가 진단용 초기화 완료 메시지
    mqttHandler.publish(
        MQTT_HEARTBEAT_TOPIC,
        "{\"event\":\"startup\",\"version\":\"v1.1.0-HA\"}",
        false
    );

    Serial.println(F("\n========================================"));
    Serial.println(F("  Water Level Sensor Node Ready! (HA)"));
    Serial.print  (F("  Node ID : ")); Serial.println(NODE_ID);
    Serial.print  (F("  Data    : ")); Serial.println(MQTT_DATA_TOPIC);
    Serial.print  (F("  Status  : ")); Serial.println(MQTT_STATUS_TOPIC);
    Serial.println(F("========================================\n"));

    // 초기화 완료: LED 3회 점멸
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH); delay(200);
        digitalWrite(LED_BUILTIN, LOW);  delay(200);
    }

    // 타이머 초기화
    lastSensorRead = millis();
    lastHeartbeat  = millis();
    lastLedBlink   = millis();
}

// ============================================
// loop()
// ============================================
void loop() {
    unsigned long now = millis();

    // ① MQTT / WiFi 연결 유지
    mqttHandler.loop();

    // ② 센서 데이터 읽기 & 발행 (SENSOR_READ_INTERVAL = 3초)
    if (now - lastSensorRead >= SENSOR_READ_INTERVAL) {
        lastSensorRead = now;
        readAndPublishSensorData();
    }

    // ③ 하트비트 발행 (HEARTBEAT_INTERVAL = 60초)
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        lastHeartbeat = now;
        mqttHandler.publishHeartbeat();
    }

    // ④ LED 상태 표시
    if (now - lastLedBlink >= LED_BLINK_INTERVAL) {
        lastLedBlink = now;
        updateLED();
    }

    delay(10);
}

// ============================================
// 센서 데이터 읽기 & MQTT 발행
// ============================================
void readAndPublishSensorData() {
    DEBUG_PRINTLN(F("\n--- [SENSOR] Reading water level ---"));

    WaterLevelData data = waterLevelSensor.readWaterLevel();

    if (data.is_valid) {
        Serial.print(F("[WATER] Dist    : ")); Serial.print(data.distance_cm, 1);         Serial.println(F(" cm"));
        Serial.print(F("[WATER] Level   : ")); Serial.print(data.water_level_cm, 1);      Serial.println(F(" cm"));
        Serial.print(F("[WATER] Percent : ")); Serial.print(data.water_level_percent, 1); Serial.println(F(" %"));

        if (mqttHandler.publishWaterLevelData(data)) {
            Serial.println(F("[WATER] Published OK"));
        } else {
            Serial.println(F("[WATER] WARNING: Publish failed"));
        }
    } else {
        Serial.println(F("[WATER] ERROR: Sensor read failed"));
        Serial.println(F("  Possible causes:"));
        Serial.println(F("  - Sensor not connected (check D7/D8)"));
        Serial.println(F("  - Distance out of range (2~400 cm)"));
        Serial.println(F("  - Echo timeout"));
    }
}

// ============================================
// LED 상태 표시
//   정상(MQTT 연결됨) : 1초 간격 천천히 점멸
//   MQTT 끊김         : 250ms 빠른 점멸
//   WiFi 끊김         : LED 꺼짐
// ============================================
void updateLED() {
    if (!mqttHandler.isWiFiConnected()) {
        // WiFi 끊김: LED OFF
        digitalWrite(LED_BUILTIN, LOW);
        ledState = false;
    } else if (!mqttHandler.isConnected()) {
        // MQTT 끊김: 빠른 점멸 (250ms 단위로 별도 처리)
        static unsigned long fastBlink = 0;
        if (millis() - fastBlink >= 250) {
            fastBlink = millis();
            ledState = !ledState;
            digitalWrite(LED_BUILTIN, ledState);
        }
    } else {
        // 정상: 느린 점멸 (1초)
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }
}

// ============================================
// 시작 메시지
// ============================================
void printStartupMessage() {
    Serial.println(F("\n========================================"));
    Serial.println(F("  Wasabi SmartFarm"));
    Serial.println(F("  Water Level Sensor Node (HA Edition)"));
    Serial.println(F("========================================"));
    Serial.println(F("  Version : v1.1.0-HA"));
    Serial.println(F("  Author  : 서준원"));
    Serial.println(F("  Date    : 2026-02-20"));
    Serial.println(F("----------------------------------------"));
    Serial.print  (F("  Node ID       : ")); Serial.println(NODE_ID);
    Serial.print  (F("  MCU           : ")); Serial.println(F("Arduino Uno R4 WiFi"));
    Serial.print  (F("  Sensor        : ")); Serial.println(F("HC-SR04 (Ultrasonic)"));
    Serial.print  (F("  WiFi SSID     : ")); Serial.println(WIFI_SSID);
    Serial.print  (F("  MQTT Broker   : ")); Serial.print(MQTT_SERVER);
                                             Serial.print(F(":")); Serial.println(MQTT_PORT);
    Serial.print  (F("  Data topic    : ")); Serial.println(MQTT_DATA_TOPIC);
    Serial.print  (F("  Status topic  : ")); Serial.println(MQTT_STATUS_TOPIC);
    Serial.print  (F("  Read interval : ")); Serial.print(SENSOR_READ_INTERVAL / 1000); Serial.println(F(" sec"));
    Serial.print  (F("  Tank height   : ")); Serial.print(TANK_HEIGHT_CM); Serial.println(F(" cm"));
    Serial.print  (F("  Sensor offset : ")); Serial.print(SENSOR_OFFSET_CM); Serial.println(F(" cm"));
    Serial.println(F("========================================\n"));
}
