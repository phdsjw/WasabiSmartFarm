/*
 * Wasabi SmartFarm - 토양 센서 노드
 * 
 * 기능:
 * - SEN0604 (4-in-1) 토양 센서 데이터 읽기
 * - WiFi 연결
 * - MQTT를 통한 센서 데이터 전송 (10초 주기)
 * 
 * Hardware:
 * - Arduino Uno R4 WiFi
 * - RS485 확장보드 (DFR0259)
 * - SEN0604 4-in-1 토양 센서
 * 
 * Author: Wasabi SmartFarm Team
 * Version: 1.0.0
 * Date: 2024-12-11
 */

#include "config.h"
#include "sen0604_modbus.h"
#include "mqtt_handler.h"

// 전역 객체
SEN0604Modbus soilSensor;
MQTTHandler mqttHandler;

// 타이머 변수
unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastLedBlink = 0;
bool ledState = false;

void setup() {
    // 시리얼 통신 초기화
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // 3초 대기
    
    printBanner();
    
    // LED 핀 초기화
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    digitalWrite(LED_BUILTIN_PIN, LOW);
    
    // WiFi 연결
    DEBUG_PRINTLN(F("\n[SETUP] Connecting to WiFi..."));
    if (!mqttHandler.connectWiFi()) {
        DEBUG_PRINTLN(F("[SETUP] WiFi connection failed! Restarting..."));
        delay(5000);
        NVIC_SystemReset();  // 시스템 재시작
    }
    
    // MQTT 연결
    DEBUG_PRINTLN(F("[SETUP] Connecting to MQTT Broker..."));
    if (!mqttHandler.connectMQTT()) {
        DEBUG_PRINTLN(F("[SETUP] MQTT connection failed! Continuing anyway..."));
    }
    
    // SEN0604 센서 초기화
    DEBUG_PRINTLN(F("[SETUP] Initializing SEN0604 sensor..."));
    if (!soilSensor.begin()) {
        DEBUG_PRINTLN(F("[SETUP] Warning: SEN0604 initialization failed or sensor not responding"));
        DEBUG_PRINTLN(F("[SETUP] Continuing... Will retry on first read"));
    }
    
    DEBUG_PRINTLN(F("[SETUP] Setup complete!\n"));
    
    // 초기 타이머 설정
    lastSensorRead = millis();
    lastHeartbeat = millis();
    lastLedBlink = millis();
    
    // 시작 LED 깜빡임 (3번)
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN_PIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN_PIN, LOW);
        delay(200);
    }
}

void loop() {
    // MQTT 연결 유지
    mqttHandler.loop();
    
    // LED 상태 표시 (연결 상태)
    updateStatusLED();
    
    // 센서 데이터 읽기 및 전송 (10초마다)
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        DEBUG_PRINTLN(F("\n========================================"));
        DEBUG_PRINT(F("[SENSOR] Reading soil sensor (Tank "));
        DEBUG_PRINT(TANK_ID);
        DEBUG_PRINTLN(F(")..."));
        
        // 센서 데이터 읽기
        SoilSensorData data = soilSensor.readSensorData();
        
        if (data.valid) {
            // 데이터 출력
            printSensorData(data);
            
            // MQTT 전송
            if (mqttHandler.isConnected()) {
                if (mqttHandler.publishSensorData(data)) {
                    DEBUG_PRINTLN(F("[SUCCESS] Data published to MQTT"));
                } else {
                    DEBUG_PRINTLN(F("[ERROR] Failed to publish data"));
                }
            } else {
                DEBUG_PRINTLN(F("[WARNING] MQTT not connected, skipping publish"));
            }
        } else {
            DEBUG_PRINTLN(F("[ERROR] Failed to read sensor data"));
            DEBUG_PRINTLN(F("[INFO] Check sensor connection and wiring"));
        }
        
        lastSensorRead = millis();
        DEBUG_PRINTLN(F("========================================\n"));
    }
    
    // 하트비트 전송 (1분마다)
    if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        DEBUG_PRINTLN(F("[HEARTBEAT] Sending heartbeat..."));
        
        if (mqttHandler.isConnected()) {
            mqttHandler.publishHeartbeat();
        }
        
        lastHeartbeat = millis();
    }
    
    // 짧은 딜레이 (CPU 부하 감소)
    delay(100);
}

void printBanner() {
    DEBUG_PRINTLN(F("\n"));
    DEBUG_PRINTLN(F("╔════════════════════════════════════════╗"));
    DEBUG_PRINTLN(F("║  Wasabi SmartFarm - Soil Sensor Node  ║"));
    DEBUG_PRINTLN(F("╚════════════════════════════════════════╝"));
    DEBUG_PRINT(F("Tank ID: ")); DEBUG_PRINTLN(TANK_ID);
    DEBUG_PRINT(F("Version: ")); DEBUG_PRINTLN(F("1.0.0"));
    DEBUG_PRINT(F("Sensor: SEN0604 (4-in-1)"));
    DEBUG_PRINTLN(F("\n"));
}

void printSensorData(const SoilSensorData& data) {
    DEBUG_PRINTLN(F("┌─────────────────────────────────────┐"));
    DEBUG_PRINTLN(F("│       Soil Sensor Data              │"));
    DEBUG_PRINTLN(F("├─────────────────────────────────────┤"));
    
    DEBUG_PRINT(F("│ Soil Temperature  : "));
    DEBUG_PRINT(data.soil_temp, 1);
    DEBUG_PRINTLN(F(" °C       │"));
    
    DEBUG_PRINT(F("│ Soil Moisture     : "));
    DEBUG_PRINT(data.soil_moisture, 1);
    DEBUG_PRINTLN(F(" %        │"));
    
    DEBUG_PRINT(F("│ Soil EC           : "));
    DEBUG_PRINT(data.soil_ec, 1);
    DEBUG_PRINTLN(F(" μS/cm   │"));
    
    DEBUG_PRINT(F("│ Soil pH           : "));
    DEBUG_PRINTLN(data.soil_ph, 2);
    DEBUG_PRINTLN(F("                     │"));
    
    DEBUG_PRINTLN(F("└─────────────────────────────────────┘"));
}

void updateStatusLED() {
    // LED 깜빡임으로 상태 표시
    if (millis() - lastLedBlink >= LED_BLINK_INTERVAL) {
        if (mqttHandler.isConnected()) {
            // 연결됨: 느린 깜빡임
            ledState = !ledState;
            digitalWrite(LED_BUILTIN_PIN, ledState);
        } else {
            // 연결 안됨: 빠른 깜빡임
            ledState = !ledState;
            digitalWrite(LED_BUILTIN_PIN, ledState);
        }
        lastLedBlink = millis();
    }
}
