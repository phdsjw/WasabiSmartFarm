/*
 * MQTT 통신 핸들러 구현
 */

#include "mqtt_handler.h"

MQTTHandler::MQTTHandler() : _mqttClient(_wifiClient) {
    _lastReconnectAttempt = 0;
    
    // MQTT Client ID 생성 (예: WasabiSoil_Tank01)
    _clientId = String(MQTT_CLIENT_ID_PREFIX) + String(NODE_ID);
    
    // MQTT Topic 생성
    _dataTopic = String(MQTT_TOPIC_DATA);
    _heartbeatTopic = String(MQTT_TOPIC_HEARTBEAT);
    
    DEBUG_PRINT(F("[MQTT] Client ID: ")); DEBUG_PRINTLN(_clientId);
    DEBUG_PRINT(F("[MQTT] Data Topic: ")); DEBUG_PRINTLN(_dataTopic);
    DEBUG_PRINT(F("[MQTT] Heartbeat Topic: ")); DEBUG_PRINTLN(_heartbeatTopic);
}

bool MQTTHandler::connectWiFi() {
    DEBUG_PRINTLN(F("[WiFi] Connecting..."));
    DEBUG_PRINT(F("[WiFi] SSID: ")); DEBUG_PRINTLN(WIFI_SSID);
    
    int retryCount = 0;
    
    while (retryCount < WIFI_MAX_RETRY) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        unsigned long startAttemptTime = millis();
        
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT) {
            delay(500);
            DEBUG_PRINT(F("."));
        }
        DEBUG_PRINTLN();
        
        if (WiFi.status() == WL_CONNECTED) {
            DEBUG_PRINTLN(F("[WiFi] Connected!"));
            DEBUG_PRINT(F("[WiFi] IP Address: "));
            DEBUG_PRINTLN(WiFi.localIP());
            return true;
        } else {
            retryCount++;
            DEBUG_PRINT(F("[WiFi] Connection failed. Retry "));
            DEBUG_PRINT(retryCount);
            DEBUG_PRINT(F("/"));
            DEBUG_PRINTLN(WIFI_MAX_RETRY);
            
            if (retryCount < WIFI_MAX_RETRY) {
                DEBUG_PRINT(F("[WiFi] Waiting "));
                DEBUG_PRINT(WIFI_RETRY_INTERVAL / 1000);
                DEBUG_PRINTLN(F("s before retry..."));
                delay(WIFI_RETRY_INTERVAL);
            }
        }
    }
    
    DEBUG_PRINTLN(F("[WiFi] ERROR: Max retry reached. Continuing without WiFi..."));
    return false;
}

bool MQTTHandler::connectMQTT() {
    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setBufferSize(512);  // JSON 크기에 맞게 조정
    
    return reconnectMQTT();
}

bool MQTTHandler::reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    DEBUG_PRINTLN(F("[WiFi] Reconnecting..."));
    WiFi.disconnect();
    delay(100);
    return connectWiFi();
}

bool MQTTHandler::reconnectMQTT() {
    if (_mqttClient.connected()) {
        return true;
    }
    
    // 재연결 시도 간격 제한 (5초)
    if (millis() - _lastReconnectAttempt < 5000) {
        return false;
    }
    _lastReconnectAttempt = millis();
    
    DEBUG_PRINT(F("[MQTT] Connecting to broker: "));
    DEBUG_PRINTLN(MQTT_SERVER);
    
    // MQTT 연결 시도
    bool connected = false;
    if (strlen(MQTT_USER) > 0) {
        // 인증 사용
        connected = _mqttClient.connect(_clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        // 인증 없음
        connected = _mqttClient.connect(_clientId.c_str());
    }
    
    if (connected) {
        DEBUG_PRINTLN(F("[MQTT] Connected!"));
        
        // 연결 성공 메시지 전송
        publishHeartbeat();
        return true;
    } else {
        DEBUG_PRINT(F("[MQTT] Connection failed, rc="));
        DEBUG_PRINTLN(_mqttClient.state());
        return false;
    }
}

void MQTTHandler::loop() {
    // WiFi 연결 확인
    if (!isWiFiConnected()) {
        reconnectWiFi();
        return;
    }
    
    // MQTT 연결 확인
    if (!isMQTTConnected()) {
        reconnectMQTT();
        return;
    }
    
    // MQTT 루프
    _mqttClient.loop();
}

bool MQTTHandler::publishSensorData(const SoilSensorData& data) {
    if (!isMQTTConnected()) {
        DEBUG_PRINTLN(F("[MQTT] Not connected, cannot publish"));
        return false;
    }
    
    if (!data.valid) {
        DEBUG_PRINTLN(F("[MQTT] Invalid sensor data, skipping publish"));
        return false;
    }
    
    // JSON 문서 생성
    StaticJsonDocument<256> doc;
    
    doc["tank_id"] = String(TANK_ID);
    doc["soil_temp"] = round(data.soil_temp * 10.0) / 10.0;      // 소수점 1자리
    doc["soil_moisture"] = round(data.soil_moisture * 10.0) / 10.0;
    doc["soil_ec"] = round(data.soil_ec * 10.0) / 10.0;
    doc["soil_ph"] = round(data.soil_ph * 100.0) / 100.0;        // 소수점 2자리
    doc["timestamp"] = data.timestamp;
    
    // JSON 직렬화
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    
    // MQTT 전송
    DEBUG_PRINT(F("[MQTT] Publishing to: ")); DEBUG_PRINTLN(_dataTopic);
    DEBUG_PRINT(F("[MQTT] Payload: ")); DEBUG_PRINTLN(buffer);
    
    bool success = _mqttClient.publish(_dataTopic.c_str(), buffer);
    
    if (success) {
        DEBUG_PRINTLN(F("[MQTT] Published successfully"));
    } else {
        DEBUG_PRINTLN(F("[MQTT] Publish failed"));
    }
    
    return success;
}

bool MQTTHandler::publishHeartbeat() {
    if (!isMQTTConnected()) {
        return false;
    }
    
    // JSON 문서 생성
    StaticJsonDocument<128> doc;
    
    doc["tank_id"] = String(TANK_ID);
  doc["status"] = "alive";
    doc["uptime"] = millis() / 1000;  // 초 단위
    doc["timestamp"] = millis();
    
    // JSON 직렬화
    char buffer[128];
    serializeJson(doc, buffer);
    
    // MQTT 전송
    DEBUG_PRINT(F("[MQTT] Heartbeat: ")); DEBUG_PRINTLN(buffer);
    
    return _mqttClient.publish(_heartbeatTopic.c_str(), buffer);
}

bool MQTTHandler::isConnected() {
    return isWiFiConnected() && isMQTTConnected();
}

bool MQTTHandler::isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

bool MQTTHandler::isMQTTConnected() {
    return _mqttClient.connected();
}
