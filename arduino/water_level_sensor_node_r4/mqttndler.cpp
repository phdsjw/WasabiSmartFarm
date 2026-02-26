/*
 * Wasabi SmartFarm - MQTT 핸들러 구현 (수위 센서 HA 버전)
 *
 * Node-RED 버전 대비 변경사항:
 *   [1] connectMQTT() : LWT + 인증 방식으로 교체
 *   [2] publishStatus(): retain = true, payload "online"/"offline"
 *   [3] publishWaterLevelData(): water_level_cm 필드 추가, rssi 추가
 *   [4] publishHeartbeat(): node_type 필드 추가
 *   [5] freeMemory(): Arduino Uno R4 WiFi (Renesas) 조건부 처리
 *   [6] HA Discovery: 수위 센서 2채널(수위%, 거리cm) 추가
 *
 * 작성자: 서준원
 * 버전  : v1.1.0 (HA Edition)
 * 날짜  : 2026-02-20
 */

#include "mqtt_handler_ha.h"

// ============================================
// 생성자
// ============================================
MQTTHandler::MQTTHandler()
    : _mqttClient(_wifiClient),
      _wifiConnected(false),
      _mqttConnected(false),
      _lastReconnectAttempt(0)
{
}

// ============================================
// 초기화
// ============================================
bool MQTTHandler::begin() {
    DEBUG_PRINTLN(F("\n[MQTT] Initializing MQTT Handler (HA Edition)..."));

    if (!connectWiFi()) {
        DEBUG_PRINTLN(F("[MQTT] WiFi initialization failed"));
        return false;
    }

    _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    _mqttClient.setKeepAlive(60);
    _mqttClient.setSocketTimeout(10);

    if (!connectMQTT()) {
        DEBUG_PRINTLN(F("[MQTT] MQTT initialization failed"));
        return false;
    }

    DEBUG_PRINTLN(F("[MQTT] Handler initialized successfully (HA Edition)"));
    return true;
}

// ============================================
// WiFi 연결 (재시도 로직 포함)
// ============================================
bool MQTTHandler::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        _wifiConnected = true;
        return true;
    }

    DEBUG_PRINT(F("[WiFi] Connecting to: "));
    DEBUG_PRINTLN(WIFI_SSID);

    int retryCount = 0;

    while (retryCount < WIFI_MAX_RETRY) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        unsigned long startTime = millis();

        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - startTime > WIFI_TIMEOUT) {
                retryCount++;
                DEBUG_PRINTF("\n[WiFi] Timeout. Retry %d/%d\n",
                             retryCount, WIFI_MAX_RETRY);
                WiFi.disconnect();
                delay(1000);
                break;
            }
            delay(500);
            DEBUG_PRINT(F("."));
        }

        if (WiFi.status() == WL_CONNECTED) {
            DEBUG_PRINTLN(F("\n[WiFi] Connected!"));
            DEBUG_PRINT(F("[WiFi] IP   : ")); DEBUG_PRINTLN(WiFi.localIP());
            DEBUG_PRINT(F("[WiFi] RSSI : ")); DEBUG_PRINT(WiFi.RSSI()); DEBUG_PRINTLN(F(" dBm"));
            _wifiConnected = true;
            return true;
        }

        if (retryCount < WIFI_MAX_RETRY) {
            DEBUG_PRINTF("[WiFi] Waiting %d sec before retry...\n",
                         WIFI_RETRY_INTERVAL / 1000);
            delay(WIFI_RETRY_INTERVAL);
        }
    }

    DEBUG_PRINTLN(F("[WiFi] Failed after max retries"));
    _wifiConnected = false;
    return false;
}

// ============================================
// MQTT 연결
// ★ Node-RED 버전 대비 핵심 변경 포인트
//   1) LWT 설정 추가 → 비정상 종료 시 HA에 "offline" 자동 발행
//   2) 인증 (USER/PASSWORD) 추가
//   3) 연결 성공 후 "online" retained 발행
//   4) HA Discovery 선택 발행
// ============================================
bool MQTTHandler::connectMQTT() {
    if (_mqttClient.connected()) {
        _mqttConnected = true;
        return true;
    }

    if (!_wifiConnected) {
        DEBUG_PRINTLN(F("[MQTT] WiFi not connected, skipping MQTT"));
        return false;
    }

    DEBUG_PRINT(F("[MQTT] Connecting to "));
    DEBUG_PRINT(MQTT_SERVER); DEBUG_PRINT(F(":")); DEBUG_PRINTLN(MQTT_PORT);
    DEBUG_PRINT(F("[MQTT] Client ID : ")); DEBUG_PRINTLN(MQTT_CLIENT_ID);
    DEBUG_PRINT(F("[MQTT] LWT topic : ")); DEBUG_PRINTLN(MQTT_STATUS_TOPIC);

    bool connected = false;

    // ── 인증 유무에 따라 분기 ──────────────────────
    if (strlen(MQTT_USER) > 0) {
        // [변경] 인증 + LWT 동시 설정
        DEBUG_PRINTLN(F("[MQTT] Using authentication + LWT"));
        connected = _mqttClient.connect(
            MQTT_CLIENT_ID,
            MQTT_USER,           // 사용자명
            MQTT_PASSWORD,       // 비밀번호
            MQTT_STATUS_TOPIC,   // LWT 토픽
            0,                   // QoS 0
            true,                // Retain = true
            MQTT_PAYLOAD_OFFLINE // LWT payload: "offline"
        );
    } else {
        // 인증 없이 연결 (개발/테스트 환경)
        DEBUG_PRINTLN(F("[MQTT] No authentication + LWT"));
        connected = _mqttClient.connect(
            MQTT_CLIENT_ID,
            nullptr,             // user
            nullptr,             // pass
            MQTT_STATUS_TOPIC,
            0,
            true,
            MQTT_PAYLOAD_OFFLINE
        );
    }

    if (connected) {
        DEBUG_PRINTLN(F("[MQTT] Connected!"));
        _mqttConnected = true;

        // [변경] "connected" → "online" (HA availability 규격)
        publishStatus(MQTT_PAYLOAD_ONLINE);

        // HA Discovery 발행 (선택적)
#if HA_DISCOVERY_ENABLED
        publishHADiscovery();
#endif
        return true;

    } else {
        // PubSubClient 오류 코드 안내
        int rc = _mqttClient.state();
        DEBUG_PRINT(F("[MQTT] Failed, rc="));
        DEBUG_PRINTLN(rc);
        switch (rc) {
            case -4: DEBUG_PRINTLN(F("  → MQTT_CONNECTION_TIMEOUT")); break;
            case -3: DEBUG_PRINTLN(F("  → MQTT_CONNECTION_LOST"));    break;
            case -2: DEBUG_PRINTLN(F("  → MQTT_CONNECT_FAILED"));     break;
            case  4: DEBUG_PRINTLN(F("  → BAD_CREDENTIALS (USER/PASS 확인)")); break;
            case  5: DEBUG_PRINTLN(F("  → UNAUTHORIZED"));            break;
            default: break;
        }
        _mqttConnected = false;
        return false;
    }
}

// ============================================
// 연결 유지 (loop() 에서 매 프레임 호출)
// ============================================
void MQTTHandler::loop() {
    // WiFi 재연결
    if (WiFi.status() != WL_CONNECTED) {
        _wifiConnected = false;
        DEBUG_PRINTLN(F("[WiFi] Connection lost. Reconnecting..."));
        connectWiFi();
    }

    // MQTT 재연결 (5초 간격)
    if (!_mqttClient.connected()) {
        _mqttConnected = false;
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            DEBUG_PRINTLN(F("[MQTT] Connection lost. Reconnecting..."));
            connectMQTT();
        }
    }

    _mqttClient.loop();
}

// ============================================
// 수위 센서 데이터 발행
// ★ Node-RED 버전 대비 변경
//   - water_level_cm 필드 추가 (실제 수위 cm)
//   - rssi 필드 추가 (신호 강도 진단)
//   - node_type 필드 추가
// ★ HA mqtt.yaml value_template 대응 키:
//   water_level_percent, distance_cm, water_level_cm
// ============================================
bool MQTTHandler::publishWaterLevelData(const WaterLevelData &data) {
    if (!_mqttConnected) {
        DEBUG_PRINTLN(F("[MQTT] Not connected. Cannot publish water level data"));
        return false;
    }
    if (!data.is_valid) {
        DEBUG_PRINTLN(F("[MQTT] Invalid data. Skipping publish"));
        return false;
    }

    StaticJsonDocument<256> doc;
    doc["node_id"]              = NODE_ID;
    doc["node_type"]            = NODE_TYPE;
    // ── HA mqtt.yaml 이 읽는 키들 ──────────────
    doc["water_level_percent"]  = round(data.water_level_percent * 10) / 10.0;
    doc["distance_cm"]          = round(data.distance_cm * 10) / 10.0;
    doc["water_level_cm"]       = round(data.water_level_cm * 10) / 10.0;
    // ───────────────────────────────────────────
    doc["rssi"]                 = WiFi.RSSI();
    doc["timestamp"]            = data.timestamp;

    char payload[256];
    serializeJson(doc, payload);

    DEBUG_PRINT(F("[MQTT] → ")); DEBUG_PRINTLN(MQTT_DATA_TOPIC);
    DEBUG_PRINT(F("[MQTT]   ")); DEBUG_PRINTLN(payload);

    return _mqttClient.publish(MQTT_DATA_TOPIC, payload);
}

// ============================================
// 하트비트 발행
// ============================================
bool MQTTHandler::publishHeartbeat() {
    if (!_mqttConnected) return false;

    StaticJsonDocument<128> doc;
    doc["node_id"]   = NODE_ID;
    doc["node_type"] = NODE_TYPE;
    doc["uptime"]    = millis() / 1000;
    doc["rssi"]      = WiFi.RSSI();
    doc["free_mem"]  = freeMemory();

    char payload[128];
    serializeJson(doc, payload);

    DEBUG_PRINT(F("[MQTT] Heartbeat → ")); DEBUG_PRINTLN(MQTT_HEARTBEAT_TOPIC);
    DEBUG_PRINTLN(payload);

    return _mqttClient.publish(MQTT_HEARTBEAT_TOPIC, payload);
}

// ============================================
// Availability 상태 발행
// ★ Node-RED 버전 대비 변경
//   - retain = true 추가 → HA 재시작 후에도 상태 유지
//   - payload: "online" / "offline" (HA 규격)
// ============================================
bool MQTTHandler::publishStatus(const char *status) {
    if (!_mqttClient.connected()) return false;

    DEBUG_PRINT(F("[MQTT] Status → ")); DEBUG_PRINTLN(status);

    // [변경] retain = true
    return _mqttClient.publish(MQTT_STATUS_TOPIC, status, true);
}

// ============================================
// 범용 발행
// ============================================
bool MQTTHandler::publish(const char *topic, const char *payload, bool retained) {
    if (!_mqttConnected) return false;
    return _mqttClient.publish(topic, payload, retained);
}

// ============================================
// 연결 상태 확인
// ============================================
bool MQTTHandler::isConnected() {
    return _mqttConnected && _mqttClient.connected();
}

bool MQTTHandler::isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

// ============================================
// WiFi RSSI
// ============================================
int MQTTHandler::getWiFiRSSI() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -999;
}

// ============================================
// 메모리 잔량
// Arduino Uno R4 WiFi (Renesas RA4M1) 는 정확한 heap
// 측정 API 가 없으므로 0 반환 (진단 참고용)
// ============================================
int MQTTHandler::freeMemory() {
#ifdef ARDUINO_ARCH_RENESAS
    return 0;
#else
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
#endif
}

// ============================================
// HA Discovery 발행 (HA_DISCOVERY_ENABLED = true 시)
// 수위 센서 2채널 등록:
//   ① 수위 퍼센트 (water_level_percent)
//   ② 센서-수면 거리 (distance_cm)
// ============================================
#if HA_DISCOVERY_ENABLED
void MQTTHandler::publishHADiscovery() {
    DEBUG_PRINTLN(F("[HA] Publishing Discovery config..."));

    char   topic[128];
    char   payload[512];
    StaticJsonDocument<512> doc;

    // ── 공통 Device 정보 ──────────────────────
    auto buildDevice = [&](JsonObject &dev) {
        dev["identifiers"][0]  = HA_DEVICE_NAME;
        dev["name"]            = HA_DEVICE_FRIENDLY;
        dev["model"]           = "Arduino Uno R4 WiFi + HC-SR04";
        dev["manufacturer"]    = "Wasabi SmartFarm";
        dev["sw_version"]      = "v1.1.0-HA";
    };

    // ① 수위 퍼센트
    snprintf(topic, sizeof(topic),
        "%s/sensor/%s_pct/config", HA_DISCOVERY_PREFIX, HA_DEVICE_NAME);

    doc.clear();
    doc["name"]                 = "와사비 수위 퍼센트";
    doc["unique_id"]            = "wasabi_water_level_percent";
    doc["state_topic"]          = MQTT_DATA_TOPIC;
    doc["value_template"]       = "{{ value_json.water_level_percent | round(1) }}";
    doc["unit_of_measurement"]  = "%";
    doc["icon"]                 = "mdi:water-percent";
    doc["availability_topic"]   = MQTT_STATUS_TOPIC;
    doc["payload_available"]    = MQTT_PAYLOAD_ONLINE;
    doc["payload_not_available"]= MQTT_PAYLOAD_OFFLINE;
    {
        JsonObject dev = doc.createNestedObject("device");
        buildDevice(dev);
    }
    serializeJson(doc, payload);
    _mqttClient.publish(topic, payload, true);
    DEBUG_PRINTLN(F("[HA] Discovery: water_level_percent published"));

    delay(100);  // 브로커 부하 방지

    // ② 센서-수면 거리 (cm)
    snprintf(topic, sizeof(topic),
        "%s/sensor/%s_dist/config", HA_DISCOVERY_PREFIX, HA_DEVICE_NAME);

    doc.clear();
    doc["name"]                 = "와사비 수위 거리";
    doc["unique_id"]            = "wasabi_water_distance_cm";
    doc["state_topic"]          = MQTT_DATA_TOPIC;
    doc["value_template"]       = "{{ value_json.distance_cm | round(1) }}";
    doc["unit_of_measurement"]  = "cm";
    doc["icon"]                 = "mdi:ruler";
    doc["availability_topic"]   = MQTT_STATUS_TOPIC;
    doc["payload_available"]    = MQTT_PAYLOAD_ONLINE;
    doc["payload_not_available"]= MQTT_PAYLOAD_OFFLINE;
    {
        JsonObject dev = doc.createNestedObject("device");
        buildDevice(dev);
    }
    serializeJson(doc, payload);
    _mqttClient.publish(topic, payload, true);
    DEBUG_PRINTLN(F("[HA] Discovery: distance_cm published"));

    DEBUG_PRINTLN(F("[HA] Discovery config published successfully"));
}
#endif
