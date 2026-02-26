/*
 * Wasabi SmartFarm - MQTT 핸들러 (대기+토양 통합 센서 노드용)
 * 
 * 기능:
 * - WiFi 연결 관리
 * - MQTT Broker 연결 및 재연결
 * - 대기 센서 데이터 전송 (JSON)
 * - 토양 센서 데이터 전송 (JSON)
 * - 하트비트 및 상태 전송
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

class MQTTHandler {
private:
  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  
  String _clientId;
  String _airDataTopic;
  String _soilDataTopic;
  String _heartbeatTopic;
  String _statusTopic;
  
  unsigned long _lastReconnectAttempt;
  int _reconnectAttempts;
  
  // WiFi 연결
  bool connectWiFi();
  
  // MQTT 연결
  bool connectMQTT();

public:
  // 생성자
  MQTTHandler();
  
  // 초기화
  bool begin();
  
  // WiFi 연결 상태 확인
  bool isWiFiConnected();
  
  // MQTT 연결 상태 확인
  bool isConnected();
  bool isMQTTConnected();  // 하위 호환성
  
  // 연결 유지 (loop에서 호출)
  void loop();
  
  // 대기 센서 데이터 전송
  bool publishAirData(const AirSensorData &data);
  
  // 토양 센서 데이터 전송
  bool publishSoilData(const SoilSensorData &data);
  
  // 하트비트 전송
  bool publishHeartbeat();
  
  // 상태 정보 전송
  bool publishStatus(const char* message);
  
  // WiFi RSSI 가져오기
  int getWiFiRSSI();
};

#endif // MQTT_HANDLER_H
