/*
 * MQTT Handler - Home Assistant 버전
 * 
 * 변경사항:
 * - MQTT 인증 지원 추가
 * - Availability (상태) 토픽 발행
 * - Home Assistant Discovery 지원 (선택적)
 * 
 * 작성자: Wasabi SmartFarm
 * 버전: 1.1.0 (HA Edition)
 */

#include "mqtt_handler.h"
#include "config.h"
#include <ArduinoJson.h>

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler() : wifiClient(), mqttClient(wifiClient) {
    // Client ID 생성
    snprintf(clientId, sizeof(clientId), "%s%s", MQTT_CLIENT_ID_PREFIX, TANK_ID);
    
    // 토픽 생성
    snprintf(dataTopic, sizeof(dataTopic), "%s%s%s", MQTT_TOPIC_PREFIX, TANK_ID, MQTT_TOPIC_SUFFIX);
    snprintf(heartbeatTopic, sizeof(heartbeatTopic), "%s%s%s", MQTT_TOPIC_PREFIX, TANK_ID, MQTT_HEARTBEAT_SUFFIX);
    snprintf(statusTopic, sizeof(statusTopic), "%s%s%s", MQTT_TOPIC_PREFIX, TANK_ID, MQTT_STATUS_SUFFIX);
}

// ============================================
// WiFi 연결
// ============================================
bool MQTTHandler::connectWiFi() {
    DEBUG_PRINT(F("[WIFI] Connecting to "));
    DEBUG_PRINTLN(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int retryCount = 0;
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        DEBUG_PRINT(F("."));
        
        // 타임아웃 체크
        if (millis() - startTime > WIFI_TIMEOUT) {
            retryCount++;
            DEBUG_PRINTLN();
            DEBUG_PRINT(F("[WIFI] Connection timeout. Retry "));
            DEBUG_PRINT(retryCount);
            DEBUG_PRINT(F("/"));
            DEBUG_PRINTLN(WIFI_MAX_RETRY);
            
            if (retryCount >= WIFI_MAX_RETRY) {
                DEBUG_PRINTLN(F("[WIFI] Max retries reached. Failed."));
                return false;
            }
            
            // 재시도
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            startTime = millis();
        }
    }
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(F("[WIFI] Connected!"));
    DEBUG_PRINT(F("[WIFI] IP: "));
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINT(F("[WIFI] RSSI: "));
    DEBUG_PRINT(WiFi.RSSI());
    DEBUG_PRINTLN(F(" dBm"));
    
    wifiConnected = true;
    return true;
}

// ============================================
// MQTT 연결 (인증 지원)
// ============================================
bool MQTTHandler::connectMQTT() {
    if (!wifiConnected) {
        DEBUG_PRINTLN(F("[MQTT] WiFi not connected"));
        return false;
    }
    
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    
    DEBUG_PRINT(F("[MQTT] Connecting to "));
    DEBUG_PRINT(MQTT_SERVER);
    DEBUG_PRINT(F(":"));
    DEBUG_PRINTLN(MQTT_PORT);
    DEBUG_PRINT(F("[MQTT] Client ID: "));
    DEBUG_PRINTLN(clientId);
    
    // ⭐ Home Assistant용 Last Will and Testament (LWT) 설정
    // 연결이 끊기면 자동으로 "offline" 발행
    bool connected = false;
    
    #if defined(MQTT_USER) && defined(MQTT_PASSWORD) && strlen(MQTT_USER) > 0
        // 인증 + LWT 사용
        DEBUG_PRINTLN(F("[MQTT] Using authentication"));
        connected = mqttClient.connect(
            clientId,
            MQTT_USER,
            MQTT_PASSWORD,
            statusTopic,    // LWT 토픽
            0,              // QoS
            true,           // Retain
            "offline"       // LWT 메시지
        );
    #else
        // 인증 없이 연결 + LWT
        DEBUG_PRINTLN(F("[MQTT] No authentication"));
        connected = mqttClient.connect(
            clientId,
            statusTopic,    // LWT 토픽
            0,              // QoS
            true,           // Retain
            "offline"       // LWT 메시지
        );
    #endif
    
    if (connected) {
        DEBUG_PRINTLN(F("[MQTT] Connected!"));
        mqttConnected = true;
        
        // 연결 성공 시 "online" 상태 발행
        publishStatus("online");
        
        // Home Assistant Discovery 발행 (선택적)
        #if HA_DISCOVERY_ENABLED
            publishHADiscovery();
        #endif
        
        return true;
    } else {
        DEBUG_PRINT(F("[MQTT] Failed, rc="));
        DEBUG_PRINTLN(mqttClient.state());
        DEBUG_PRINTLN(F("[MQTT] Error codes:"));
        DEBUG_PRINTLN(F("  -4: MQTT_CONNECTION_TIMEOUT"));
        DEBUG_PRINTLN(F("  -3: MQTT_CONNECTION_LOST"));
        DEBUG_PRINTLN(F("  -2: MQTT_CONNECT_FAILED"));
        DEBUG_PRINTLN(F("  -1: MQTT_DISCONNECTED"));
        DEBUG_PRINTLN(F("   1: MQTT_CONNECT_BAD_PROTOCOL"));
        DEBUG_PRINTLN(F("   2: MQTT_CONNECT_BAD_CLIENT_ID"));
        DEBUG_PRINTLN(F("   3: MQTT_CONNECT_UNAVAILABLE"));
        DEBUG_PRINTLN(F("   4: MQTT_CONNECT_BAD_CREDENTIALS"));
        DEBUG_PRINTLN(F("   5: MQTT_CONNECT_UNAUTHORIZED"));
        mqttConnected = false;
        return false;
    }
}

// ============================================
// MQTT 루프 (연결 유지)
// ============================================
void MQTTHandler::loop() {
    if (!mqttClient.connected()) {
        mqttConnected = false;
        
        // 재연결 시도 (5초 간격)
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > 5000) {
            lastReconnect = millis();
            DEBUG_PRINTLN(F("[MQTT] Reconnecting..."));
            connectMQTT();
        }
    }
    
    mqttClient.loop();
}

// ============================================
// 센서 데이터 발행
// ============================================
bool MQTTHandler::publishSensorData(const SoilSensorData& data) {
    if (!mqttConnected) return false;
    
    StaticJsonDocument<256> doc;
    doc["tank_id"] = TANK_ID;
    doc["soil_temp"] = round(data.soil_temp * 10) / 10.0;
    doc["soil_moisture"] = round(data.soil_moisture * 10) / 10.0;
    doc["soil_ec"] = round(data.soil_ec * 100) / 100.0;
    doc["soil_ph"] = round(data.soil_ph * 100) / 100.0;
    doc["timestamp"] = millis();
    doc["rssi"] = WiFi.RSSI();
    
    char payload[256];
    serializeJson(doc, payload);
    
    DEBUG_PRINT(F("[MQTT] Publishing to "));
    DEBUG_PRINTLN(dataTopic);
    DEBUG_PRINT(F("[MQTT] Payload: "));
    DEBUG_PRINTLN(payload);
    
    return mqttClient.publish(dataTopic, payload);
}

// ============================================
// 하트비트 발행
// ============================================
bool MQTTHandler::publishHeartbeat() {
    if (!mqttConnected) return false;
    
    StaticJsonDocument<128> doc;
    doc["status"] = "alive";
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["free_memory"] = freeMemory();
    
    char payload[128];
    serializeJson(doc, payload);
    
    DEBUG_PRINT(F("[MQTT] Heartbeat: "));
    DEBUG_PRINTLN(payload);
    
    return mqttClient.publish(heartbeatTopic, payload);
}

// ============================================
// 상태 발행 (Availability)
// ============================================
bool MQTTHandler::publishStatus(const char* status) {
    if (!mqttClient.connected()) return false;
    
    DEBUG_PRINT(F("[MQTT] Status: "));
    DEBUG_PRINTLN(status);
    
    // Retained 메시지로 발행 (Home Assistant에서 상태 유지)
    return mqttClient.publish(statusTopic, status, true);
}

// ============================================
// 연결 상태 확인
// ============================================
bool MQTTHandler::isConnected() {
    return mqttConnected && mqttClient.connected();
}

// ============================================
// 일반 발행 (외부 사용)
// ============================================
bool MQTTHandler::publish(const char* topic, const char* payload, bool retained) {
    if (!mqttConnected) return false;
    return mqttClient.publish(topic, payload, retained);
}

// ============================================
// 메모리 확인 (디버그용)
// ============================================
int MQTTHandler::freeMemory() {
    // Arduino Uno R4 WiFi용
    #ifdef ARDUINO_ARCH_RENESAS
        return 0;  // 정확한 측정 불가
    #else
        extern int __heap_start, *__brkval;
        int v;
        return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    #endif
}

// ============================================
// Home Assistant Discovery 발행 (선택적)
// ============================================
#if HA_DISCOVERY_ENABLED
void MQTTHandler::publishHADiscovery() {
    DEBUG_PRINTLN(F("[HA] Publishing Discovery config..."));
    
    char topic[128];
    char payload[512];
    StaticJsonDocument<512> doc;
    
    // 온도 센서
    snprintf(topic, sizeof(topic), 
        "%s/sensor/%s%s_temp/config", HA_DISCOVERY_PREFIX, HA_DEVICE_NAME_PREFIX, TANK_ID);
    
    doc.clear();
    doc["name"] = String("와사비 토양 온도 Tank ") + TANK_ID;
    doc["unique_id"] = String(HA_DEVICE_NAME_PREFIX) + TANK_ID + "_temp";
    doc["state_topic"] = dataTopic;
    doc["value_template"] = "{{ value_json.soil_temp }}";
    doc["unit_of_measurement"] = "°C";
    doc["device_class"] = "temperature";
    doc["availability_topic"] = statusTopic;
    doc["payload_available"] = "online";
    doc["payload_not_available"] = "offline";
    
    // Device 정보
    JsonObject device = doc.createNestedObject("device");
    device["identifiers"][0] = String(HA_DEVICE_NAME_PREFIX) + TANK_ID;
    device["name"] = String("와사비 토양센서 Tank ") + TANK_ID;
    device["model"] = "Arduino Uno R4 WiFi + SEN0604";
    device["manufacturer"] = "Wasabi SmartFarm";
    
    serializeJson(doc, payload);
    mqttClient.publish(topic, payload, true);
    
    // 습도, EC, pH도 동일하게...
    // (코드 간략화를 위해 생략)
    
    DEBUG_PRINTLN(F("[HA] Discovery config published"));
}
#endif
