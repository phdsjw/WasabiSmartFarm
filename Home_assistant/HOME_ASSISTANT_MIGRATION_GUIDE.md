# 🏠 와사비 스마트팜 - Home Assistant 마이그레이션 가이드

**작성일**: 2026-01-06  
**버전**: v1.0.0  
**대상**: Node-RED → Home Assistant + Arduino Uno R4 WiFi + MQTT

---

## 📋 목차

1. [현재 시스템 분석](#1-현재-시스템-분석)
2. [마이그레이션 개요](#2-마이그레이션-개요)
3. [Step 1: Home Assistant 환경 구축](#step-1-home-assistant-환경-구축)
4. [Step 2: MQTT 브로커 설정](#step-2-mqtt-브로커-설정)
5. [Step 3: Arduino 펌웨어 수정](#step-3-arduino-펌웨어-수정)
6. [Step 4: Home Assistant MQTT 센서 설정](#step-4-home-assistant-mqtt-센서-설정)
7. [Step 5: 자동화 로직 구현](#step-5-자동화-로직-구현)
8. [Step 6: 대시보드 구성](#step-6-대시보드-구성)
9. [Step 7: 테스트 및 검증](#step-7-테스트-및-검증)
10. [Step 8: 고급 기능 구현](#step-8-고급-기능-구현)
11. [문제 해결](#문제-해결)
12. [체크리스트](#체크리스트)

---

## 1. 현재 시스템 분석

### 1.1 기존 시스템 구성

```
┌─────────────────────────────────────────────────────────────┐
│                    현재 시스템 (Node-RED)                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  센서 노드 (21개)        Node-RED Server        제어        │
│  ┌──────────────┐       ┌───────────────┐    ┌──────────┐ │
│  │ 토양센서 x18 │──MQTT─→│  Flow Logic   │───→│액추에이터│ │
│  │ 대기센서 x1  │       │  Dashboard    │    │  노드    │ │
│  │ 물탱크 x1    │       │  InfluxDB     │    └──────────┘ │
│  │ 시스템 x1    │       └───────────────┘                  │
│  └──────────────┘              ↓                           │
│                         Mosquitto MQTT                     │
│                         (Port 1883)                        │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 현재 MQTT 토픽 구조

| 토픽 | 설명 | 데이터 형식 |
|------|------|-------------|
| `sensor/soil/tank{01-18}/data` | 토양 센서 데이터 | JSON |
| `sensor/soil/tank{01-18}/heartbeat` | 토양 센서 하트비트 | JSON |
| `sensor/air/zone1/data` | 대기 센서 데이터 | JSON |
| `sensor/water_tank/data` | 물탱크 센서 데이터 | JSON |
| `actuator/heartbeat` | 액추에이터 하트비트 | JSON |
| `actuator/irrigation_pump/on` | 관수 펌프 ON 명령 | - |
| `actuator/irrigation_pump/off` | 관수 펌프 OFF 명령 | - |
| `actuator/drainage_pump/on` | 배수 펌프 ON 명령 | - |
| `actuator/drainage_pump/off` | 배수 펌프 OFF 명령 | - |
| `actuator/emergency_stop` | 비상 정지 | - |
| `actuator/emergency_release` | 비상 정지 해제 | - |

### 1.3 센서 데이터 JSON 구조

**토양 센서 (SEN0604)**:
```json
{
  "tank_id": "01",
  "soil_temp": 20.5,
  "soil_moisture": 92.3,
  "soil_ec": 3.2,
  "soil_ph": 6.5,
  "timestamp": 1702284000000
}
```

**대기 센서 (SHT30)**:
```json
{
  "zone_id": "01",
  "air_temp": 22.5,
  "air_humidity": 65.3,
  "timestamp": 1702284000000
}
```

**물탱크 센서**:
```json
{
  "water_temp": 18.5,
  "water_ph": 6.8,
  "water_ec": 1.2,
  "water_tds": 450,
  "timestamp": 1702284000000
}
```

### 1.4 제어 로직

| 조건 | 임계값 | 동작 |
|------|--------|------|
| 토양 습도 평균 | ≤ 95% | 관수 펌프 ON (4분) |
| 토양 EC 평균 | ≥ 5.0 μS/cm | 관수 펌프 ON (4분) |
| 토양 온도 평균 | ≥ 22°C | 관수 펌프 ON (4분) |
| 관수 최소 간격 | 1시간 | - |

---

## 2. 마이그레이션 개요

### 2.1 대상 시스템 구성

```
┌─────────────────────────────────────────────────────────────┐
│                  대상 시스템 (Home Assistant)                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  센서 노드 (21개)       Home Assistant          제어        │
│  ┌──────────────┐       ┌───────────────┐    ┌──────────┐ │
│  │ 토양센서 x18 │──MQTT─→│  MQTT 통합    │───→│액추에이터│ │
│  │ 대기센서 x1  │       │  Automations  │    │  노드    │ │
│  │ 물탱크 x1    │       │  Dashboard    │    └──────────┘ │
│  │ 시스템 x1    │       │  Recorder     │                  │
│  └──────────────┘       └───────────────┘                  │
│         ↓                      ↓                           │
│  Arduino Uno R4         MQTT Add-on                        │
│  WiFi (그대로)          (Mosquitto)                        │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 변경 사항 요약

| 항목 | Node-RED | Home Assistant | 비고 |
|------|----------|----------------|------|
| 제어 엔진 | Node-RED Flow | Automations + Scripts | 설정 파일 기반 |
| UI 대시보드 | Node-RED Dashboard | Lovelace Dashboard | 더 풍부한 UI |
| MQTT 브로커 | Mosquitto (별도 설치) | Mosquitto Add-on | 통합 관리 |
| 데이터베이스 | InfluxDB | Recorder (SQLite/Maria) | 기본 내장 |
| 알림 | 수동 구현 | Notification 통합 | 다양한 플랫폼 |
| 모바일 앱 | 웹 접속 | HA Companion App | 네이티브 앱 |

### 2.3 Arduino 펌웨어 변경점

**변경 없음** (대부분 그대로 사용 가능):
- WiFi 연결 코드
- MQTT 연결 코드
- 센서 읽기 코드
- 릴레이 제어 코드

**선택적 변경** (호환성 향상):
- MQTT 토픽 구조 (Home Assistant 컨벤션)
- JSON 페이로드 (HA Discovery 지원)
- 하트비트 → Availability 토픽

---

## Step 1: Home Assistant 환경 구축

### 1.1 Home Assistant 설치 방법 선택

**방법 A: Home Assistant OS (권장 - Raspberry Pi)**
```bash
# Raspberry Pi 4에 설치
# 1. Raspberry Pi Imager 다운로드
# 2. Home Assistant OS 이미지 선택
# 3. SD 카드에 굽기
# 4. 첫 부팅 후 http://homeassistant.local:8123 접속
```

**방법 B: Home Assistant Supervised (기존 Linux 서버)**
```bash
# Debian 12에서 설치 예시
sudo apt update
sudo apt install -y apparmor jq wget curl \
    udisks2 libglib2.0-bin network-manager \
    dbus systemd-journal-remote

# Docker 설치
curl -fsSL https://get.docker.com | sh

# Home Assistant Supervised 설치
wget -O ha-supervised.deb https://github.com/home-assistant/supervised-installer/releases/latest/download/homeassistant-supervised.deb
sudo dpkg -i ha-supervised.deb
```

**방법 C: Home Assistant Container (Windows/Docker)**
```bash
# Docker Desktop 설치 후
docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=Asia/Seoul \
  -v /PATH_TO_YOUR_CONFIG:/config \
  -v /run/dbus:/run/dbus:ro \
  --network=host \
  ghcr.io/home-assistant/home-assistant:stable
```

### 1.2 초기 설정

1. **웹 UI 접속**: `http://[HA_IP]:8123`
2. **계정 생성**: 관리자 계정 설정
3. **위치 설정**: 한국 시간대 설정
4. **통합 검색**: 자동 감지된 장치 확인

### 1.3 필수 Add-on 설치

**설정 → Add-ons → Add-on Store**에서 설치:

1. **Mosquitto broker** (필수)
   - MQTT 브로커
   - 기존 Arduino 노드들과 연동

2. **File editor** (권장)
   - configuration.yaml 편집

3. **Terminal & SSH** (권장)
   - 고급 설정 및 디버깅

4. **Samba share** (선택)
   - Windows에서 설정 파일 편집

---

## Step 2: MQTT 브로커 설정

### 2.1 Mosquitto Add-on 설정

**Add-on 설치 후 Configuration 탭에서:**

```yaml
# Mosquitto Broker Add-on Configuration
logins:
  - username: wasabi_farm
    password: your_secure_password
customize:
  active: false
  folder: mosquitto
certfile: fullchain.pem
keyfile: privkey.pem
require_certificate: false
```

### 2.2 MQTT 사용자 설정

**Configuration → Integrations → MQTT:**

```yaml
# configuration.yaml 에 추가 (또는 UI에서 설정)
mqtt:
  broker: localhost  # 또는 Add-on 사용 시 core-mosquitto
  port: 1883
  username: wasabi_farm
  password: your_secure_password
```

### 2.3 Arduino config.h 업데이트

각 Arduino 노드의 `config.h` 파일을 수정:

```cpp
// ============================================
// MQTT 설정 (Home Assistant용 업데이트)
// ============================================
#define MQTT_SERVER      "192.168.0.xxx"       // Home Assistant IP
#define MQTT_PORT        1883
#define MQTT_USER        "wasabi_farm"         // 새로 추가
#define MQTT_PASSWORD    "your_secure_password" // 새로 추가

// Home Assistant MQTT Discovery (선택적)
#define HA_DISCOVERY_PREFIX  "homeassistant"
#define HA_DEVICE_NAME       "wasabi_soil_01"
```

### 2.4 MQTT 연결 테스트

```bash
# Home Assistant에서 MQTT 메시지 테스트
# 설정 → 통합 → MQTT → 수신/발행

# 토픽: sensor/soil/tank01/data
# 페이로드: {"tank_id":"01","soil_temp":20.5,"soil_moisture":92.3}
```

---

## Step 3: Arduino 펌웨어 수정

### 3.1 수정이 필요 없는 부분

기존 Arduino 코드는 **대부분 그대로 사용 가능**합니다:

- ✅ WiFi 연결 로직 (`WiFiS3` 라이브러리)
- ✅ MQTT 연결 로직 (`PubSubClient` 라이브러리)
- ✅ 센서 읽기 로직 (SEN0604 Modbus RTU)
- ✅ JSON 데이터 생성 (`ArduinoJson`)
- ✅ 릴레이 제어 로직

### 3.2 MQTT 인증 추가 (필수 변경)

**mqtt_handler.cpp 수정:**

```cpp
// 기존 코드
bool MQTTHandler::connectMQTT() {
    // ...
    if (mqttClient.connect(clientId)) {
        // 연결 성공
    }
}

// 변경된 코드 (인증 추가)
bool MQTTHandler::connectMQTT() {
    // ...
    // MQTT_USER와 MQTT_PASSWORD가 정의되어 있으면 인증 사용
    #if defined(MQTT_USER) && defined(MQTT_PASSWORD) && strlen(MQTT_USER) > 0
        if (mqttClient.connect(clientId, MQTT_USER, MQTT_PASSWORD)) {
            // 연결 성공
        }
    #else
        if (mqttClient.connect(clientId)) {
            // 연결 성공
        }
    #endif
}
```

### 3.3 Home Assistant MQTT Discovery 지원 (선택적)

**장점**: Home Assistant에서 자동으로 센서/스위치 인식

**새 파일: `ha_discovery.h`**:

```cpp
#ifndef HA_DISCOVERY_H
#define HA_DISCOVERY_H

#include "config.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

class HADiscovery {
public:
    HADiscovery(PubSubClient* client) : mqttClient(client) {}
    
    // 토양 센서 Discovery 메시지 발행
    void publishSoilSensorConfig(const char* tankId) {
        char topic[128];
        char payload[512];
        
        // 온도 센서
        snprintf(topic, sizeof(topic), 
            "homeassistant/sensor/wasabi_soil_%s_temp/config", tankId);
        
        StaticJsonDocument<512> doc;
        doc["name"] = String("와사비 토양 온도 Tank ") + tankId;
        doc["unique_id"] = String("wasabi_soil_") + tankId + "_temp";
        doc["state_topic"] = String("sensor/soil/tank") + tankId + "/data";
        doc["value_template"] = "{{ value_json.soil_temp }}";
        doc["unit_of_measurement"] = "°C";
        doc["device_class"] = "temperature";
        
        // Device 정보
        JsonObject device = doc.createNestedObject("device");
        device["identifiers"][0] = String("wasabi_soil_") + tankId;
        device["name"] = String("와사비 토양센서 Tank ") + tankId;
        device["model"] = "Arduino Uno R4 WiFi + SEN0604";
        device["manufacturer"] = "Wasabi SmartFarm";
        
        // Availability
        doc["availability_topic"] = String("sensor/soil/tank") + tankId + "/status";
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
        
        serializeJson(doc, payload);
        mqttClient->publish(topic, payload, true);  // retained
        
        // 습도 센서
        snprintf(topic, sizeof(topic), 
            "homeassistant/sensor/wasabi_soil_%s_moisture/config", tankId);
        doc.clear();
        doc["name"] = String("와사비 토양 습도 Tank ") + tankId;
        doc["unique_id"] = String("wasabi_soil_") + tankId + "_moisture";
        doc["state_topic"] = String("sensor/soil/tank") + tankId + "/data";
        doc["value_template"] = "{{ value_json.soil_moisture }}";
        doc["unit_of_measurement"] = "%";
        doc["device_class"] = "humidity";
        
        // 이하 동일...
        serializeJson(doc, payload);
        mqttClient->publish(topic, payload, true);
    }
    
    // Availability 상태 발행
    void publishAvailability(const char* tankId, bool online) {
        char topic[64];
        snprintf(topic, sizeof(topic), "sensor/soil/tank%s/status", tankId);
        mqttClient->publish(topic, online ? "online" : "offline", true);
    }

private:
    PubSubClient* mqttClient;
};

#endif // HA_DISCOVERY_H
```

### 3.4 수정된 soil_sensor_node.ino 예시

```cpp
/*
 * Wasabi SmartFarm - 토양 센서 노드
 * Home Assistant 버전
 */

#include "config.h"
#include "sen0604_modbus.h"
#include "mqtt_handler.h"
// #include "ha_discovery.h"  // 선택적: HA Discovery 지원

SEN0604Modbus soilSensor;
MQTTHandler mqttHandler;
// HADiscovery haDiscovery(&mqttHandler.getClient());  // 선택적

unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat = 0;
bool firstConnect = true;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    printBanner();
    
    pinMode(LED_BUILTIN_PIN, OUTPUT);
    
    // WiFi 연결
    if (!mqttHandler.connectWiFi()) {
        DEBUG_PRINTLN(F("[ERROR] WiFi failed! Restarting..."));
        delay(5000);
        NVIC_SystemReset();
    }
    
    // MQTT 연결
    if (!mqttHandler.connectMQTT()) {
        DEBUG_PRINTLN(F("[WARNING] MQTT failed, will retry..."));
    }
    
    // 센서 초기화
    if (!soilSensor.begin()) {
        DEBUG_PRINTLN(F("[WARNING] Sensor init failed"));
    }
    
    DEBUG_PRINTLN(F("[SETUP] Complete!"));
}

void loop() {
    mqttHandler.loop();
    
    // 첫 연결 시 HA Discovery 메시지 발행 (선택적)
    if (firstConnect && mqttHandler.isConnected()) {
        // haDiscovery.publishSoilSensorConfig(TANK_ID);
        // haDiscovery.publishAvailability(TANK_ID, true);
        
        // LWT (Last Will Testament) 대신 수동 상태 발행
        publishOnlineStatus();
        firstConnect = false;
    }
    
    // 센서 데이터 읽기 (10초마다)
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        SoilSensorData data = soilSensor.readSensorData();
        
        if (data.valid && mqttHandler.isConnected()) {
            mqttHandler.publishSensorData(data);
        }
        
        lastSensorRead = millis();
    }
    
    // 하트비트 (1분마다)
    if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        if (mqttHandler.isConnected()) {
            mqttHandler.publishHeartbeat();
            publishOnlineStatus();  // Availability 갱신
        }
        lastHeartbeat = millis();
    }
    
    delay(100);
}

void publishOnlineStatus() {
    char topic[64];
    snprintf(topic, sizeof(topic), "sensor/soil/tank%s/status", TANK_ID);
    mqttHandler.publish(topic, "online", true);
}

void printBanner() {
    DEBUG_PRINTLN(F("\n========================================"));
    DEBUG_PRINTLN(F("  Wasabi SmartFarm - Soil Sensor Node"));
    DEBUG_PRINTLN(F("  (Home Assistant Edition)"));
    DEBUG_PRINT(F("  Tank ID: ")); DEBUG_PRINTLN(TANK_ID);
    DEBUG_PRINTLN(F("========================================\n"));
}
```

### 3.5 수정된 actuator_node.ino 예시

액추에이터 노드는 기존 코드가 거의 그대로 동작합니다.

**주요 확인 사항:**
1. MQTT 인증 추가
2. 토픽이 Home Assistant 설정과 일치하는지 확인

---

## Step 4: Home Assistant MQTT 센서 설정

### 4.1 configuration.yaml 기본 설정

```yaml
# configuration.yaml

# 기본 설정
homeassistant:
  name: 와사비 스마트팜
  unit_system: metric
  time_zone: Asia/Seoul
  currency: KRW

# Recorder 설정 (데이터베이스)
recorder:
  purge_keep_days: 30
  include:
    domains:
      - sensor
      - switch
      - binary_sensor
    entity_globs:
      - sensor.wasabi_*
      - switch.wasabi_*

# Logger 설정
logger:
  default: warning
  logs:
    homeassistant.components.mqtt: info
```

### 4.2 MQTT 센서 설정

**mqtt.yaml** (별도 파일로 분리 권장):

```yaml
# mqtt.yaml

# ================================================
# 토양 센서 (18개)
# ================================================
sensor:
  # Tank 01
  - name: "와사비 토양 온도 Tank01"
    unique_id: "wasabi_soil_01_temp"
    state_topic: "sensor/soil/tank01/data"
    value_template: "{{ value_json.soil_temp | round(1) }}"
    unit_of_measurement: "°C"
    device_class: temperature
    availability:
      - topic: "sensor/soil/tank01/heartbeat"
        payload_available: '{"status":"alive"}'
        value_template: "{{ value_json.status }}"
    
  - name: "와사비 토양 습도 Tank01"
    unique_id: "wasabi_soil_01_moisture"
    state_topic: "sensor/soil/tank01/data"
    value_template: "{{ value_json.soil_moisture | round(1) }}"
    unit_of_measurement: "%"
    device_class: humidity

  - name: "와사비 토양 EC Tank01"
    unique_id: "wasabi_soil_01_ec"
    state_topic: "sensor/soil/tank01/data"
    value_template: "{{ value_json.soil_ec | round(2) }}"
    unit_of_measurement: "μS/cm"
    icon: mdi:flash

  - name: "와사비 토양 pH Tank01"
    unique_id: "wasabi_soil_01_ph"
    state_topic: "sensor/soil/tank01/data"
    value_template: "{{ value_json.soil_ph | round(2) }}"
    icon: mdi:ph

  # Tank 02 ~ Tank 18 (동일한 패턴으로 반복)
  # 아래에 템플릿 스크립트로 생성 가능

# ================================================
# 대기 센서 (1개)
# ================================================
  - name: "와사비 대기 온도 Zone1"
    unique_id: "wasabi_air_zone1_temp"
    state_topic: "sensor/air/zone1/data"
    value_template: "{{ value_json.air_temp | round(1) }}"
    unit_of_measurement: "°C"
    device_class: temperature

  - name: "와사비 대기 습도 Zone1"
    unique_id: "wasabi_air_zone1_humidity"
    state_topic: "sensor/air/zone1/data"
    value_template: "{{ value_json.air_humidity | round(1) }}"
    unit_of_measurement: "%"
    device_class: humidity

# ================================================
# 물탱크 센서 (1개)
# ================================================
  - name: "와사비 수조 수온"
    unique_id: "wasabi_water_temp"
    state_topic: "sensor/water_tank/data"
    value_template: "{{ value_json.water_temp | round(1) }}"
    unit_of_measurement: "°C"
    device_class: temperature

  - name: "와사비 수조 pH"
    unique_id: "wasabi_water_ph"
    state_topic: "sensor/water_tank/data"
    value_template: "{{ value_json.water_ph | round(2) }}"
    icon: mdi:ph

  - name: "와사비 수조 TDS"
    unique_id: "wasabi_water_tds"
    state_topic: "sensor/water_tank/data"
    value_template: "{{ value_json.water_tds }}"
    unit_of_measurement: "ppm"
    icon: mdi:water

  - name: "와사비 수조 EC"
    unique_id: "wasabi_water_ec"
    state_topic: "sensor/water_tank/data"
    value_template: "{{ value_json.water_ec | round(2) }}"
    unit_of_measurement: "mS/cm"
    icon: mdi:flash

# ================================================
# 액추에이터 스위치
# ================================================
switch:
  - name: "와사비 관수 펌프"
    unique_id: "wasabi_irrigation_pump"
    command_topic: "actuator/irrigation_pump/set"
    state_topic: "actuator/state"
    value_template: "{{ value_json.irrigation_pump }}"
    payload_on: "on"
    payload_off: "off"
    state_on: true
    state_off: false
    icon: mdi:water-pump

  - name: "와사비 배수 펌프"
    unique_id: "wasabi_drainage_pump"
    command_topic: "actuator/drainage_pump/set"
    state_topic: "actuator/state"
    value_template: "{{ value_json.drainage_pump }}"
    payload_on: "on"
    payload_off: "off"
    state_on: true
    state_off: false
    icon: mdi:water-pump-off

  - name: "와사비 천장 팬"
    unique_id: "wasabi_ceiling_fan"
    command_topic: "actuator/fan/set"
    state_topic: "actuator/state"
    value_template: "{{ value_json.fan }}"
    payload_on: "on"
    payload_off: "off"
    state_on: true
    state_off: false
    icon: mdi:fan

  - name: "와사비 LED 조명"
    unique_id: "wasabi_led_light"
    command_topic: "actuator/led/set"
    state_topic: "actuator/state"
    value_template: "{{ value_json.led }}"
    payload_on: "on"
    payload_off: "off"
    state_on: true
    state_off: false
    icon: mdi:led-on

# ================================================
# 버튼 (단발성 명령)
# ================================================
button:
  - name: "와사비 비상 정지"
    unique_id: "wasabi_emergency_stop"
    command_topic: "actuator/emergency_stop"
    payload_press: ""
    icon: mdi:alert-octagon

  - name: "와사비 비상 정지 해제"
    unique_id: "wasabi_emergency_release"
    command_topic: "actuator/emergency_release"
    payload_press: "RELEASED"
    icon: mdi:check-circle
```

### 4.3 configuration.yaml에 포함

```yaml
# configuration.yaml

mqtt: !include mqtt.yaml
```

### 4.4 센서 18개 자동 생성 스크립트

**Python 스크립트로 yaml 생성:**

```python
#!/usr/bin/env python3
# generate_mqtt_sensors.py

sensors_yaml = """# 자동 생성된 토양 센서 설정
sensor:
"""

for tank_id in range(1, 19):
    tank_str = f"{tank_id:02d}"
    sensors_yaml += f"""
  # Tank {tank_str}
  - name: "와사비 토양 온도 Tank{tank_str}"
    unique_id: "wasabi_soil_{tank_str}_temp"
    state_topic: "sensor/soil/tank{tank_str}/data"
    value_template: "{{{{ value_json.soil_temp | round(1) }}}}"
    unit_of_measurement: "°C"
    device_class: temperature

  - name: "와사비 토양 습도 Tank{tank_str}"
    unique_id: "wasabi_soil_{tank_str}_moisture"
    state_topic: "sensor/soil/tank{tank_str}/data"
    value_template: "{{{{ value_json.soil_moisture | round(1) }}}}"
    unit_of_measurement: "%"
    device_class: humidity

  - name: "와사비 토양 EC Tank{tank_str}"
    unique_id: "wasabi_soil_{tank_str}_ec"
    state_topic: "sensor/soil/tank{tank_str}/data"
    value_template: "{{{{ value_json.soil_ec | round(2) }}}}"
    unit_of_measurement: "μS/cm"
    icon: mdi:flash

  - name: "와사비 토양 pH Tank{tank_str}"
    unique_id: "wasabi_soil_{tank_str}_ph"
    state_topic: "sensor/soil/tank{tank_str}/data"
    value_template: "{{{{ value_json.soil_ph | round(2) }}}}"
    icon: mdi:ph
"""

print(sensors_yaml)
```

---

## Step 5: 자동화 로직 구현

### 5.1 템플릿 센서 (평균값 계산)

**template.yaml:**

```yaml
# template.yaml

sensor:
  # 토양 온도 평균 (18개 센서)
  - name: "와사비 토양 온도 평균"
    unique_id: "wasabi_soil_temp_avg"
    unit_of_measurement: "°C"
    device_class: temperature
    state: >
      {% set sensors = [
        states('sensor.wasabi_soil_01_temp'),
        states('sensor.wasabi_soil_02_temp'),
        states('sensor.wasabi_soil_03_temp'),
        states('sensor.wasabi_soil_04_temp'),
        states('sensor.wasabi_soil_05_temp'),
        states('sensor.wasabi_soil_06_temp'),
        states('sensor.wasabi_soil_07_temp'),
        states('sensor.wasabi_soil_08_temp'),
        states('sensor.wasabi_soil_09_temp'),
        states('sensor.wasabi_soil_10_temp'),
        states('sensor.wasabi_soil_11_temp'),
        states('sensor.wasabi_soil_12_temp'),
        states('sensor.wasabi_soil_13_temp'),
        states('sensor.wasabi_soil_14_temp'),
        states('sensor.wasabi_soil_15_temp'),
        states('sensor.wasabi_soil_16_temp'),
        states('sensor.wasabi_soil_17_temp'),
        states('sensor.wasabi_soil_18_temp')
      ] %}
      {% set valid = sensors | select('is_number') | list %}
      {% if valid | length >= 12 %}
        {{ (valid | map('float') | sum / valid | length) | round(1) }}
      {% else %}
        unavailable
      {% endif %}

  # 토양 습도 평균
  - name: "와사비 토양 습도 평균"
    unique_id: "wasabi_soil_moisture_avg"
    unit_of_measurement: "%"
    device_class: humidity
    state: >
      {% set sensors = [
        states('sensor.wasabi_soil_01_moisture'),
        states('sensor.wasabi_soil_02_moisture'),
        states('sensor.wasabi_soil_03_moisture'),
        states('sensor.wasabi_soil_04_moisture'),
        states('sensor.wasabi_soil_05_moisture'),
        states('sensor.wasabi_soil_06_moisture'),
        states('sensor.wasabi_soil_07_moisture'),
        states('sensor.wasabi_soil_08_moisture'),
        states('sensor.wasabi_soil_09_moisture'),
        states('sensor.wasabi_soil_10_moisture'),
        states('sensor.wasabi_soil_11_moisture'),
        states('sensor.wasabi_soil_12_moisture'),
        states('sensor.wasabi_soil_13_moisture'),
        states('sensor.wasabi_soil_14_moisture'),
        states('sensor.wasabi_soil_15_moisture'),
        states('sensor.wasabi_soil_16_moisture'),
        states('sensor.wasabi_soil_17_moisture'),
        states('sensor.wasabi_soil_18_moisture')
      ] %}
      {% set valid = sensors | select('is_number') | list %}
      {% if valid | length >= 12 %}
        {{ (valid | map('float') | sum / valid | length) | round(1) }}
      {% else %}
        unavailable
      {% endif %}

  # 토양 EC 평균
  - name: "와사비 토양 EC 평균"
    unique_id: "wasabi_soil_ec_avg"
    unit_of_measurement: "μS/cm"
    icon: mdi:flash
    state: >
      {% set sensors = [
        states('sensor.wasabi_soil_01_ec'),
        states('sensor.wasabi_soil_02_ec'),
        states('sensor.wasabi_soil_03_ec'),
        states('sensor.wasabi_soil_04_ec'),
        states('sensor.wasabi_soil_05_ec'),
        states('sensor.wasabi_soil_06_ec'),
        states('sensor.wasabi_soil_07_ec'),
        states('sensor.wasabi_soil_08_ec'),
        states('sensor.wasabi_soil_09_ec'),
        states('sensor.wasabi_soil_10_ec'),
        states('sensor.wasabi_soil_11_ec'),
        states('sensor.wasabi_soil_12_ec'),
        states('sensor.wasabi_soil_13_ec'),
        states('sensor.wasabi_soil_14_ec'),
        states('sensor.wasabi_soil_15_ec'),
        states('sensor.wasabi_soil_16_ec'),
        states('sensor.wasabi_soil_17_ec'),
        states('sensor.wasabi_soil_18_ec')
      ] %}
      {% set valid = sensors | select('is_number') | list %}
      {% if valid | length >= 12 %}
        {{ (valid | map('float') | sum / valid | length) | round(2) }}
      {% else %}
        unavailable
      {% endif %}

  # 토양 pH 평균
  - name: "와사비 토양 pH 평균"
    unique_id: "wasabi_soil_ph_avg"
    icon: mdi:ph
    state: >
      {% set sensors = [
        states('sensor.wasabi_soil_01_ph'),
        states('sensor.wasabi_soil_02_ph'),
        states('sensor.wasabi_soil_03_ph'),
        states('sensor.wasabi_soil_04_ph'),
        states('sensor.wasabi_soil_05_ph'),
        states('sensor.wasabi_soil_06_ph'),
        states('sensor.wasabi_soil_07_ph'),
        states('sensor.wasabi_soil_08_ph'),
        states('sensor.wasabi_soil_09_ph'),
        states('sensor.wasabi_soil_10_ph'),
        states('sensor.wasabi_soil_11_ph'),
        states('sensor.wasabi_soil_12_ph'),
        states('sensor.wasabi_soil_13_ph'),
        states('sensor.wasabi_soil_14_ph'),
        states('sensor.wasabi_soil_15_ph'),
        states('sensor.wasabi_soil_16_ph'),
        states('sensor.wasabi_soil_17_ph'),
        states('sensor.wasabi_soil_18_ph')
      ] %}
      {% set valid = sensors | select('is_number') | list %}
      {% if valid | length >= 12 %}
        {{ (valid | map('float') | sum / valid | length) | round(2) }}
      {% else %}
        unavailable
      {% endif %}
```

### 5.2 입력 헬퍼 설정

**input_boolean.yaml:**

```yaml
# input_boolean.yaml

wasabi_auto_irrigation:
  name: "와사비 자동 관수 모드"
  icon: mdi:water-auto
```

**input_number.yaml:**

```yaml
# input_number.yaml

wasabi_moisture_threshold:
  name: "토양 습도 임계값"
  min: 80
  max: 100
  step: 1
  unit_of_measurement: "%"
  mode: slider
  icon: mdi:water-percent

wasabi_ec_threshold:
  name: "토양 EC 임계값"
  min: 1
  max: 10
  step: 0.1
  unit_of_measurement: "μS/cm"
  mode: slider
  icon: mdi:flash

wasabi_temp_threshold:
  name: "토양 온도 임계값"
  min: 15
  max: 30
  step: 0.5
  unit_of_measurement: "°C"
  mode: slider
  icon: mdi:thermometer

wasabi_irrigation_duration:
  name: "관수 지속 시간"
  min: 60
  max: 600
  step: 30
  unit_of_measurement: "초"
  mode: slider
  icon: mdi:timer

wasabi_irrigation_cooldown:
  name: "관수 최소 간격"
  min: 30
  max: 120
  step: 5
  unit_of_measurement: "분"
  mode: slider
  icon: mdi:timer-off
```

### 5.3 자동화 - 자동 관수

**automations.yaml:**

```yaml
# automations.yaml

- id: wasabi_auto_irrigation_start
  alias: "와사비 자동 관수 시작"
  description: "조건 충족 시 자동으로 관수 시작"
  mode: single
  trigger:
    # 1분마다 조건 체크
    - platform: time_pattern
      minutes: "/1"
  condition:
    # 자동 모드가 켜져 있어야 함
    - condition: state
      entity_id: input_boolean.wasabi_auto_irrigation
      state: "on"
    # 현재 관수 중이 아니어야 함
    - condition: state
      entity_id: switch.wasabi_irrigation_pump
      state: "off"
    # 마지막 관수 후 최소 시간 경과
    - condition: template
      value_template: >
        {% set last = state_attr('automation.wasabi_auto_irrigation_start', 'last_triggered') %}
        {% if last is none %}
          true
        {% else %}
          {{ (now() - last).total_seconds() > (states('input_number.wasabi_irrigation_cooldown') | float * 60) }}
        {% endif %}
    # 관수 조건 (OR)
    - condition: or
      conditions:
        # 습도가 임계값 이하
        - condition: template
          value_template: >
            {{ states('sensor.wasabi_soil_moisture_avg') | float(100) <= states('input_number.wasabi_moisture_threshold') | float(95) }}
        # EC가 임계값 이상
        - condition: template
          value_template: >
            {{ states('sensor.wasabi_soil_ec_avg') | float(0) >= states('input_number.wasabi_ec_threshold') | float(5.0) }}
        # 온도가 임계값 이상
        - condition: template
          value_template: >
            {{ states('sensor.wasabi_soil_temp_avg') | float(0) >= states('input_number.wasabi_temp_threshold') | float(22) }}
  action:
    - service: switch.turn_on
      target:
        entity_id: switch.wasabi_irrigation_pump
    - service: notify.persistent_notification
      data:
        title: "🌱 와사비 자동 관수 시작"
        message: >
          습도: {{ states('sensor.wasabi_soil_moisture_avg') }}%,
          EC: {{ states('sensor.wasabi_soil_ec_avg') }} μS/cm,
          온도: {{ states('sensor.wasabi_soil_temp_avg') }}°C
    # 설정된 시간 후 자동 정지
    - delay:
        seconds: "{{ states('input_number.wasabi_irrigation_duration') | int }}"
    - service: switch.turn_off
      target:
        entity_id: switch.wasabi_irrigation_pump
    - service: notify.persistent_notification
      data:
        title: "💧 와사비 자동 관수 완료"
        message: "{{ states('input_number.wasabi_irrigation_duration') }}초 관수 완료"

- id: wasabi_emergency_stop_automation
  alias: "와사비 비상 정지"
  description: "비상 정지 시 모든 액추에이터 정지"
  mode: single
  trigger:
    - platform: state
      entity_id: button.wasabi_emergency_stop
  action:
    - service: switch.turn_off
      target:
        entity_id:
          - switch.wasabi_irrigation_pump
          - switch.wasabi_drainage_pump
          - switch.wasabi_ceiling_fan
          - switch.wasabi_led_light
    - service: input_boolean.turn_off
      target:
        entity_id: input_boolean.wasabi_auto_irrigation
    - service: notify.persistent_notification
      data:
        title: "🚨 와사비 비상 정지 활성화"
        message: "모든 액추에이터가 정지되었습니다. 자동 모드가 비활성화되었습니다."
```

### 5.4 스크립트 - 수동 제어

**scripts.yaml:**

```yaml
# scripts.yaml

wasabi_manual_irrigation:
  alias: "와사비 수동 관수"
  description: "수동으로 관수 시작 (설정된 시간만큼)"
  icon: mdi:water
  mode: single
  sequence:
    - service: switch.turn_on
      target:
        entity_id: switch.wasabi_irrigation_pump
    - delay:
        seconds: "{{ states('input_number.wasabi_irrigation_duration') | int }}"
    - service: switch.turn_off
      target:
        entity_id: switch.wasabi_irrigation_pump

wasabi_test_sensors:
  alias: "와사비 센서 테스트"
  description: "현재 모든 센서 값 확인"
  icon: mdi:test-tube
  sequence:
    - service: notify.persistent_notification
      data:
        title: "📊 와사비 센서 현황"
        message: >
          **토양 평균값**
          - 온도: {{ states('sensor.wasabi_soil_temp_avg') }}°C
          - 습도: {{ states('sensor.wasabi_soil_moisture_avg') }}%
          - EC: {{ states('sensor.wasabi_soil_ec_avg') }} μS/cm
          - pH: {{ states('sensor.wasabi_soil_ph_avg') }}
          
          **대기**
          - 온도: {{ states('sensor.wasabi_air_zone1_temp') }}°C
          - 습도: {{ states('sensor.wasabi_air_zone1_humidity') }}%
          
          **수조**
          - 수온: {{ states('sensor.wasabi_water_temp') }}°C
          - pH: {{ states('sensor.wasabi_water_ph') }}
          - TDS: {{ states('sensor.wasabi_water_tds') }} ppm
```

---

## Step 6: 대시보드 구성

### 6.1 Lovelace 대시보드 설정

**ui-lovelace.yaml:**

```yaml
# Lovelace Dashboard 설정

title: 와사비 스마트팜
views:
  # ================================================
  # 1. 개요 탭
  # ================================================
  - title: 개요
    path: overview
    icon: mdi:home
    badges:
      - entity: switch.wasabi_irrigation_pump
      - entity: switch.wasabi_drainage_pump
      - entity: input_boolean.wasabi_auto_irrigation
    cards:
      # 시스템 상태 카드
      - type: entities
        title: 🎛️ 시스템 상태
        entities:
          - entity: input_boolean.wasabi_auto_irrigation
            name: 자동 관수 모드
          - entity: switch.wasabi_irrigation_pump
            name: 관수 펌프
          - entity: switch.wasabi_drainage_pump
            name: 배수 펌프
          - entity: switch.wasabi_ceiling_fan
            name: 천장 팬
          - entity: switch.wasabi_led_light
            name: LED 조명

      # 토양 평균값 카드
      - type: glance
        title: 🌱 토양 센서 평균
        columns: 4
        entities:
          - entity: sensor.wasabi_soil_temp_avg
            name: 온도
          - entity: sensor.wasabi_soil_moisture_avg
            name: 습도
          - entity: sensor.wasabi_soil_ec_avg
            name: EC
          - entity: sensor.wasabi_soil_ph_avg
            name: pH

      # 대기 센서 카드
      - type: glance
        title: 🌡️ 대기 센서
        columns: 2
        entities:
          - entity: sensor.wasabi_air_zone1_temp
            name: 대기 온도
          - entity: sensor.wasabi_air_zone1_humidity
            name: 대기 습도

      # 수조 센서 카드
      - type: glance
        title: 💧 수조 센서
        columns: 4
        entities:
          - entity: sensor.wasabi_water_temp
            name: 수온
          - entity: sensor.wasabi_water_ph
            name: pH
          - entity: sensor.wasabi_water_ec
            name: EC
          - entity: sensor.wasabi_water_tds
            name: TDS

      # 수동 제어 버튼
      - type: horizontal-stack
        cards:
          - type: button
            name: 수동 관수
            icon: mdi:water
            tap_action:
              action: call-service
              service: script.wasabi_manual_irrigation
          - type: button
            name: 비상 정지
            icon: mdi:alert-octagon
            icon_color: red
            tap_action:
              action: call-service
              service: button.press
              target:
                entity_id: button.wasabi_emergency_stop

  # ================================================
  # 2. 토양 센서 트렌드 탭
  # ================================================
  - title: 토양 센서
    path: soil
    icon: mdi:sprout
    cards:
      # 토양 온도 차트
      - type: custom:apexcharts-card
        header:
          title: 토양 온도 (°C)
          show: true
        graph_span: 6h
        series:
          - entity: sensor.wasabi_soil_temp_avg
            name: 평균
            color: "#FF5722"
          - entity: sensor.wasabi_soil_01_temp
            name: Tank 01
            opacity: 0.5
          - entity: sensor.wasabi_soil_02_temp
            name: Tank 02
            opacity: 0.5
          # ... (나머지 센서 추가)

      # 토양 습도 차트
      - type: custom:apexcharts-card
        header:
          title: 토양 습도 (%)
          show: true
        graph_span: 6h
        series:
          - entity: sensor.wasabi_soil_moisture_avg
            name: 평균
            color: "#2196F3"

      # 토양 EC 차트
      - type: custom:apexcharts-card
        header:
          title: 토양 EC (μS/cm)
          show: true
        graph_span: 6h
        series:
          - entity: sensor.wasabi_soil_ec_avg
            name: 평균
            color: "#4CAF50"

      # 토양 pH 차트
      - type: custom:apexcharts-card
        header:
          title: 토양 pH
          show: true
        graph_span: 6h
        series:
          - entity: sensor.wasabi_soil_ph_avg
            name: 평균
            color: "#9C27B0"

  # ================================================
  # 3. 제어 설정 탭
  # ================================================
  - title: 제어 설정
    path: control
    icon: mdi:cog
    cards:
      # 임계값 설정
      - type: entities
        title: ⚙️ 관수 임계값 설정
        entities:
          - entity: input_number.wasabi_moisture_threshold
            name: 토양 습도 임계값
          - entity: input_number.wasabi_ec_threshold
            name: 토양 EC 임계값
          - entity: input_number.wasabi_temp_threshold
            name: 토양 온도 임계값

      # 타이머 설정
      - type: entities
        title: ⏱️ 타이머 설정
        entities:
          - entity: input_number.wasabi_irrigation_duration
            name: 관수 지속 시간
          - entity: input_number.wasabi_irrigation_cooldown
            name: 관수 최소 간격

      # 자동화 상태
      - type: entities
        title: 🤖 자동화 상태
        entities:
          - entity: automation.wasabi_auto_irrigation_start
            name: 자동 관수

  # ================================================
  # 4. 히스토리 탭
  # ================================================
  - title: 히스토리
    path: history
    icon: mdi:chart-line
    cards:
      - type: history-graph
        title: 최근 24시간 토양 상태
        hours_to_show: 24
        entities:
          - entity: sensor.wasabi_soil_temp_avg
            name: 온도
          - entity: sensor.wasabi_soil_moisture_avg
            name: 습도

      - type: logbook
        title: 관수 이력
        hours_to_show: 48
        entities:
          - switch.wasabi_irrigation_pump
          - switch.wasabi_drainage_pump
```

### 6.2 HACS 커스텀 카드 설치 (권장)

**HACS (Home Assistant Community Store) 설치 후:**

1. **apexcharts-card**: 고급 차트
2. **mushroom-cards**: 모던한 UI
3. **mini-graph-card**: 간단한 그래프
4. **auto-entities**: 동적 엔티티 목록

---

## Step 7: 테스트 및 검증

### 7.1 단계별 테스트 체크리스트

#### Phase 1: 기본 연결 테스트
```
[ ] Home Assistant 웹 UI 접속 가능
[ ] Mosquitto Add-on 실행 중
[ ] MQTT 통합 연결 성공
```

#### Phase 2: Arduino 통신 테스트
```
[ ] Arduino WiFi 연결 성공
[ ] Arduino MQTT 연결 성공 (인증 포함)
[ ] 시리얼 모니터에서 정상 로그 확인
```

#### Phase 3: 센서 데이터 수신 테스트
```
[ ] HA 개발자 도구 → 상태에서 센서 값 확인
[ ] MQTT 탐색기에서 토픽 수신 확인
[ ] 18개 토양 센서 모두 값 수신
[ ] 대기 센서 값 수신
[ ] 물탱크 센서 값 수신
```

#### Phase 4: 제어 테스트
```
[ ] HA에서 관수 펌프 ON → Arduino 릴레이 ON 확인
[ ] HA에서 관수 펌프 OFF → Arduino 릴레이 OFF 확인
[ ] 비상 정지 버튼 동작 확인
```

#### Phase 5: 자동화 테스트
```
[ ] 자동 모드 ON 상태에서 조건 충족 시 관수 시작
[ ] 설정 시간 후 자동 정지
[ ] 쿨다운 시간 동안 재시작 안 됨
```

#### Phase 6: 대시보드 테스트
```
[ ] 모든 카드 정상 표시
[ ] 차트 데이터 갱신
[ ] 버튼 동작 확인
```

### 7.2 MQTT 테스트 명령어

```bash
# Home Assistant 터미널에서

# 토양 센서 데이터 구독
mosquitto_sub -h localhost -u wasabi_farm -P your_password \
  -t "sensor/soil/+/data" -v

# 테스트 데이터 발행
mosquitto_pub -h localhost -u wasabi_farm -P your_password \
  -t "sensor/soil/tank01/data" \
  -m '{"tank_id":"01","soil_temp":21.5,"soil_moisture":93.5,"soil_ec":2.8,"soil_ph":6.3}'

# 액추에이터 명령 구독 (Arduino 역할)
mosquitto_sub -h localhost -u wasabi_farm -P your_password \
  -t "actuator/#" -v

# 관수 펌프 ON 명령 발행
mosquitto_pub -h localhost -u wasabi_farm -P your_password \
  -t "actuator/irrigation_pump/set" -m "on"
```

---

## Step 8: 고급 기능 구현

### 8.1 알림 설정

**configuration.yaml:**

```yaml
# Telegram 알림 (선택)
telegram_bot:
  - platform: polling
    api_key: YOUR_TELEGRAM_BOT_API_KEY
    allowed_chat_ids:
      - YOUR_CHAT_ID

notify:
  - platform: telegram
    name: wasabi_telegram
    chat_id: YOUR_CHAT_ID
```

**자동화에 알림 추가:**

```yaml
- id: wasabi_critical_alert
  alias: "와사비 이상 상태 알림"
  trigger:
    - platform: numeric_state
      entity_id: sensor.wasabi_soil_temp_avg
      above: 25
      for:
        minutes: 5
    - platform: numeric_state
      entity_id: sensor.wasabi_soil_moisture_avg
      below: 85
      for:
        minutes: 5
  action:
    - service: notify.wasabi_telegram
      data:
        title: "🚨 와사비 스마트팜 경고"
        message: >
          이상 상태 감지:
          토양 온도: {{ states('sensor.wasabi_soil_temp_avg') }}°C
          토양 습도: {{ states('sensor.wasabi_soil_moisture_avg') }}%
```

### 8.2 데이터 장기 저장 (InfluxDB 연동)

**configuration.yaml:**

```yaml
# InfluxDB 연동 (기존 InfluxDB 재사용)
influxdb:
  host: localhost
  port: 8086
  database: wasabi_smartfarm
  default_measurement: state
  include:
    entity_globs:
      - sensor.wasabi_*
      - switch.wasabi_*
```

### 8.3 모바일 앱 설정

1. **Home Assistant Companion App** 설치 (iOS/Android)
2. **서버 주소** 입력: `http://[HA_IP]:8123`
3. **장기 액세스 토큰** 생성 (프로필 → 보안)
4. **알림 설정**: 앱에서 푸시 알림 활성화

### 8.4 Node-RED 병행 운영 (전환 기간)

Node-RED와 Home Assistant를 동시에 운영하면서 점진적으로 전환:

```yaml
# Home Assistant에서 Node-RED Add-on 설치
# Node-RED에서 HA 노드 사용 가능
# 복잡한 로직은 Node-RED, 단순 자동화는 HA
```

---

## 문제 해결

### 일반적인 문제

#### MQTT 연결 실패
```
문제: Arduino가 MQTT에 연결되지 않음
해결:
1. MQTT 브로커 IP 확인
2. 사용자명/비밀번호 확인
3. 방화벽 포트 1883 개방
4. Mosquitto Add-on 로그 확인
```

#### 센서 값이 표시되지 않음
```
문제: Home Assistant에서 센서 값 없음
해결:
1. MQTT 탐색기에서 토픽 확인
2. value_template 문법 확인
3. JSON 데이터 형식 확인
4. HA 개발자 도구 → MQTT 청취
```

#### 자동화가 동작하지 않음
```
문제: 조건 충족해도 자동화 미동작
해결:
1. 자동화 활성화 상태 확인
2. 조건 템플릿 테스트 (개발자 도구 → 템플릿)
3. 로그 확인 (설정 → 로그)
4. 자동화 트리거 수동 테스트
```

### 로그 확인 방법

```yaml
# configuration.yaml - 디버그 로깅 활성화
logger:
  default: warning
  logs:
    homeassistant.components.mqtt: debug
    homeassistant.components.automation: debug
```

---

## 체크리스트

### 마이그레이션 전 준비
```
[ ] Home Assistant 설치 환경 결정 (RPi/Docker/Supervised)
[ ] 기존 Node-RED 설정 백업
[ ] Arduino config.h 원본 백업
[ ] MQTT 토픽 구조 문서화
[ ] 현재 제어 로직 문서화
```

### Home Assistant 설정
```
[ ] HA 설치 및 초기 설정
[ ] Mosquitto Add-on 설치
[ ] MQTT 통합 설정
[ ] 필수 입력 헬퍼 생성
```

### Arduino 수정
```
[ ] config.h MQTT 인증 추가
[ ] mqtt_handler.cpp 인증 로직 추가
[ ] 컴파일 및 업로드 테스트
[ ] 18개 토양 센서 노드 업데이트
[ ] 액추에이터 노드 업데이트
```

### Home Assistant 센서/자동화
```
[ ] MQTT 센서 설정 (mqtt.yaml)
[ ] 템플릿 센서 설정 (평균값)
[ ] 자동화 설정 (automations.yaml)
[ ] 스크립트 설정 (scripts.yaml)
```

### 대시보드 및 마무리
```
[ ] Lovelace 대시보드 구성
[ ] HACS 카드 설치 (선택)
[ ] 알림 설정 (선택)
[ ] 전체 기능 테스트
[ ] 7일 안정화 운영
```

---

## 참고 자료

- [Home Assistant 공식 문서](https://www.home-assistant.io/docs/)
- [MQTT 통합 문서](https://www.home-assistant.io/integrations/mqtt/)
- [Lovelace 대시보드 문서](https://www.home-assistant.io/lovelace/)
- [자동화 문서](https://www.home-assistant.io/docs/automation/)
- [Arduino Uno R4 WiFi 문서](https://docs.arduino.cc/hardware/uno-r4-wifi/)

---

**작성자**: Wasabi SmartFarm Team  
**버전**: v1.0.0  
**최종 수정**: 2026-01-06
