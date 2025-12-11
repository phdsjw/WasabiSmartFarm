/*
 * MQTT Communication Handler
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"

// ============================================
// 전역 MQTT 객체
// ============================================
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;

// ============================================
// 함수 선언
// ============================================

// WiFi 연결
void connectWiFi();

// MQTT 설정 및 연결
void setupMQTT();
void connectMQTT();

// MQTT 콜백 (Subscribe 메시지 수신)
void mqttCallback(char* topic, byte* payload, unsigned int length);

// 센서 데이터 발행
void publishEnvironmentData(const SensorData& data);
void publishWaterLevel(uint8_t tankNum, int level);
void publishHeartbeat();

// 유틸리티
String getClientId();

#endif // MQTT_HANDLER_H
