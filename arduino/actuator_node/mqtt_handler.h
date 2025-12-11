/*
 * Wasabi SmartFarm - MQTT 핸들러 (액추에이터 노드용)
 * 
 * 기능:
 * - WiFi 연결 관리
 * - MQTT Broker 연결 및 재연결
 * - 명령 토픽 구독
 * - 상태 및 하트비트 전송
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "actuator_control.h"

class MQTTHandler {
private:
  WiFiClient _wifiClient;
  PubSubClient _mqttClient;
  ActuatorControl* _actuatorControl;
  
  unsigned long _lastReconnectAttempt;
  
  // WiFi 연결
  bool connectWiFi();
  
  // MQTT 연결
  bool connectMQTT();
  
  // MQTT 콜백 (정적 함수)
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
  
  // 명령 처리 (내부)
  void handleCommand(const char* topic, const char* message);

public:
  // 싱글톤 인스턴스 (콜백용)
  static MQTTHandler* instance;
  
  // 생성자
  MQTTHandler(ActuatorControl* actuatorControl);
  
  // 초기화
  bool begin();
  
  // WiFi 연결 상태 확인
  bool isWiFiConnected();
  
  // MQTT 연결 상태 확인
  bool isMQTTConnected();
  
  // 연결 유지 (loop에서 호출)
  void loop();
  
  // 상태 리포트 전송
  bool publishStateReport(const ActuatorState &state);
  
  // 하트비트 전송
  bool publishHeartbeat(const ActuatorState &state);
  
  // 상태 메시지 전송
  bool publishStatus(const char* message);
  
  // WiFi RSSI 가져오기
  int getWiFiRSSI();
};

#endif // MQTT_HANDLER_H
