/*
 * Wasabi SmartFarm - MQTT 핸들러 헤더 (수위 센서 HA 버전)
 *
 * Node-RED 버전 대비 변경사항:
 *   - LWT(Last Will) 지원
 *   - MQTT 인증 지원
 *   - HA availability "online" / "offline" payload
 *   - HA Discovery 지원 (선택적)
 *   - publish() 공개 메서드 추가 (retained 옵션)
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */

#ifndef MQTT_HANDLER_HA_H
#define MQTT_HANDLER_HA_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config_ha.h"

class MQTTHandler {
public:
    // 생성자
    MQTTHandler();

    // 초기화 (setup() 에서 한 번 호출)
    bool begin();

    // 연결 유지 — loop() 에서 매 프레임 호출
    void loop();

    // 연결 상태 확인
    bool isConnected();
    bool isWiFiConnected();

    // ── 발행 메서드 ──────────────────────────────
    // 수위 센서 데이터 발행
    bool publishWaterLevelData(const WaterLevelData &data);

    // 하트비트 발행 (60초 주기)
    bool publishHeartbeat();

    // Availability 상태 발행 ("online" / "offline")
    bool publishStatus(const char *status);

    // 범용 발행 (외부에서 직접 사용 가능)
    bool publish(const char *topic, const char *payload, bool retained = false);

    // WiFi RSSI 반환
    int getWiFiRSSI();

private:
    WiFiClient    _wifiClient;
    PubSubClient  _mqttClient;

    bool _wifiConnected;
    bool _mqttConnected;

    unsigned long _lastReconnectAttempt;

    // 내부 연결 함수
    bool connectWiFi();
    bool connectMQTT();

    // 메모리 잔량 (진단용)
    int freeMemory();

#if HA_DISCOVERY_ENABLED
    void publishHADiscovery();
#endif
};

#endif // MQTT_HANDLER_HA_H
