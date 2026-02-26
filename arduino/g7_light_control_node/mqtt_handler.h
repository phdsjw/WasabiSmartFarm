/*
 * Wasabi SmartFarm - MQTT 핸들러 (조명 제어 노드용)
 * 
 * 기능:
 * - WiFi 연결 관리
 * - MQTT Broker 연결 및 재연결
 * - 조명 명령 수신 및 상태 전송
 * - 하트비트 전송
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2026-02-25
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "light_control.h"

class MQTTHandler {
private:
  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  
  String _clientId;
  unsigned long _lastReconnectAttempt;
  LightControl* _lightControl;
  
  // WiFi 연결
  bool connectWiFi();
  
  // MQTT 연결
  bool connectMQTT();
  
  // MQTT 콜백 (Static으로 선언하여 콜백으로 사용)
  static void mqttCallback(char* topic, byte* payload, unsigned int length);

public:
  // 생성자
  MQTTHandler(LightControl* lightControl);
  
  // 초기화
  bool begin();
  
  // WiFi 연결 상태 확인
  bool isWiFiConnected();
  
  // MQTT 연결 상태 확인
  bool isConnected();
  
  // 연결 유지 (loop에서 호출)
  void loop();
  
  // 상태 전송
  bool publishState();
  
  // 하트비트 전송
  bool publishHeartbeat();
  
  // 상태 정보 전송
  bool publishStatus(const char* message);
  
  // WiFi RSSI 가져오기
  int getWiFiRSSI();
};

#endif // MQTT_HANDLER_H
