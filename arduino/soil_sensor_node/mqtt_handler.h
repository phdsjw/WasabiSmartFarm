/*
 * MQTT 통신 핸들러 헤더
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

class MQTTHandler {
public:
    MQTTHandler();
    
    // WiFi 연결
    bool connectWiFi();
    
    // MQTT 연결
    bool connectMQTT();
    
    // MQTT 연결 유지
    void loop();
    
    // 센서 데이터 전송
    bool publishSensorData(const SoilSensorData& data);
    
    // 하트비트 전송
    bool publishHeartbeat();
    
    // 연결 상태 확인
    bool isConnected();
    bool isWiFiConnected();
    bool isMQTTConnected();
    
private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    
    String _clientId;
    String _dataTopic;
    String _heartbeatTopic;
    
    unsigned long _lastReconnectAttempt;
    
    // WiFi 재연결
    bool reconnectWiFi();
    
    // MQTT 재연결
    bool reconnectMQTT();
};

#endif // MQTT_HANDLER_H
