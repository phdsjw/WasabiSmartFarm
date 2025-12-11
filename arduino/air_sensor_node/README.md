# 🌡️ 와사비 스마트팜 - 대기 센서 노드

**작성자**: 서준원  
**버전**: v1.0.0  
**날짜**: 2024-12-11

---

## 📋 개요

하우스 내 대기 온도와 습도를 측정하여 MQTT를 통해 Node-RED로 전송하는 센서 노드입니다.  
3개의 위치(입구, 중앙, 후면)에 설치되어 공간별 환경 데이터를 제공합니다.

---

## 🎯 주요 기능

- ✅ **SHT30 온습도 센서 지원** (I2C 통신)
- ✅ **WiFi 연결 및 자동 재연결**
- ✅ **MQTT 프로토콜 기반 데이터 전송**
- ✅ **10초 주기 센서 데이터 수집 및 전송**
- ✅ **하트비트 전송** (1분 주기)
- ✅ **LED 상태 표시**
- ✅ **센서 오류 감지 및 자동 재초기화**
- ✅ **CRC 체크섬 검증** (데이터 무결성 보장)

---

## 🔧 하드웨어 구성

### 필수 부품

| 부품명 | 수량 | 사양 | 용도 |
|--------|------|------|------|
| Arduino Uno R4 WiFi | 3개 | MCU | 메인 컨트롤러 |
| SHT30 온습도 센서 | 3개 | I2C, ±0.2°C, ±2%RH | 대기 온습도 측정 |
| 5V 전원 어댑터 | 3개 | 5V/3A | 전원 공급 |
| USB-C 케이블 | 3개 | - | 전원 및 프로그래밍 |
| Dupont 케이블 | 12개 | F-F | I2C 연결 |

### 센서 사양 (SHT30)

| 항목 | 사양 |
|------|------|
| 측정 범위 (온도) | -40°C ~ +125°C |
| 측정 범위 (습도) | 0% ~ 100% RH |
| 정확도 (온도) | ±0.2°C (0~90°C) |
| 정확도 (습도) | ±2% RH (10~90%) |
| 해상도 | 0.01°C, 0.01% RH |
| 통신 방식 | I2C (주소: 0x44 또는 0x45) |
| 전원 전압 | 2.4V ~ 5.5V |

---

## ⚡ 하드웨어 연결

### SHT30 센서 연결 (I2C)

```
Arduino Uno R4 WiFi         SHT30 센서
====================        ===========
5V        ---------->       VCC (빨강)
GND       ---------->       GND (검정)
A4 (SDA)  <-------->        SDA (파랑)
A5 (SCL)  <-------->        SCL (노랑)
```

### 설치 위치

| Zone ID | 위치 | 설명 |
|---------|------|------|
| Zone 01 | 하우스 입구 | 외부 공기 유입 지점 모니터링 |
| Zone 02 | 하우스 중앙 | 재배 공간 중심부 환경 측정 |
| Zone 03 | 하우스 후면 | 환기구 인근 온습도 모니터링 |

---

## 💻 소프트웨어 설정

### 1. Arduino IDE 설정

#### 필수 라이브러리 설치

Arduino IDE → `도구` → `라이브러리 관리` 에서 다음 라이브러리를 설치하세요:

| 라이브러리 | 버전 | 용도 |
|-----------|------|------|
| `WiFiS3` | 최신 | Arduino R4 WiFi 전용 |
| `PubSubClient` | 2.8+ | MQTT 통신 |
| `ArduinoJson` | 6.21+ | JSON 직렬화 |
| `Wire` | 내장 | I2C 통신 |

### 2. config.h 설정

#### 2.1 Zone ID 설정 (각 노드마다 변경!)

```cpp
// Zone 01 (입구)
#define ZONE_ID "01"

// Zone 02 (중앙)
// #define ZONE_ID "02"

// Zone 03 (후면)
// #define ZONE_ID "03"
```

#### 2.2 WiFi 설정

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
```

#### 2.3 MQTT 설정

```cpp
#define MQTT_SERVER "192.168.0.100"   // Node-RED 서버 IP
#define MQTT_PORT 1883
```

### 3. 펌웨어 업로드

1. Arduino IDE에서 `air_sensor_node.ino` 파일 열기
2. `도구` → `보드` → `Arduino Uno R4 WiFi` 선택
3. `도구` → `포트` → 연결된 포트 선택
4. **각 노드마다 `config.h`에서 `ZONE_ID` 변경!**
5. `업로드` 버튼 클릭

---

## 📡 MQTT 토픽 및 메시지

### 센서 데이터 (10초 주기)

**Topic**: `sensor/air/zone{01~03}/data`

**Payload (JSON)**:
```json
{
  "zone_id": "01",
  "air_temp": 22.5,
  "air_humidity": 65.3,
  "timestamp": 1702284000000,
  "rssi": -65
}
```

| 필드 | 타입 | 단위 | 설명 |
|------|------|------|------|
| `zone_id` | string | - | 센서 노드 ID (01~03) |
| `air_temp` | float | °C | 대기 온도 (소수점 1자리) |
| `air_humidity` | float | % | 대기 습도 (소수점 1자리) |
| `timestamp` | long | ms | 측정 시간 (밀리초) |
| `rssi` | int | dBm | WiFi 신호 강도 |

### 하트비트 (60초 주기)

**Topic**: `sensor/air/zone{01~03}/heartbeat`

**Payload (JSON)**:
```json
{
  "zone_id": "01",
  "status": "alive",
  "uptime": 3600000,
  "rssi": -65,
  "free_memory": 12345
}
```

### 상태 메시지

**Topic**: `sensor/air/zone{01~03}/status`

**Payload (JSON)**:
```json
{
  "zone_id": "01",
  "status": "initialized",
  "timestamp": 1702284000000
}
```

**Status 값**:
- `"initialized"`: 센서 노드 초기화 완료
- `"online"`: MQTT 연결 성공
- `"error"`: 센서 오류 발생

---

## 🔍 테스트 및 디버그

### 시리얼 모니터 출력 확인

1. Arduino IDE → `도구` → `시리얼 모니터` 열기
2. 보드레이트: **115200 bps** 설정
3. 출력 예시:

```
========================================
  Wasabi SmartFarm
  Air Sensor Node
========================================
  Version: v1.0.0
  Author: 서준원
  Date: 2024-12-11
========================================
  Zone ID: 01
  Sensor: SHT30 (I2C Address: 0x44)
  WiFi SSID: your_wifi_ssid
  MQTT Broker: 192.168.0.100:1883
  Sensor Read Interval: 10 sec
  Heartbeat Interval: 60 sec
========================================

[SETUP] Initializing SHT30 sensor...
[SHT30] Initializing sensor...
[SHT30] Sensor initialized successfully
[SETUP] SHT30 sensor initialized successfully

[MQTT] Initializing MQTT Handler...
[WiFi] Connecting to: your_wifi_ssid
...........
[WiFi] Connected!
[WiFi] IP Address: 192.168.0.101
[WiFi] RSSI: -65 dBm
[MQTT] Connecting to broker: 192.168.0.100:1883
[MQTT] Client ID: WasabiAir_Zone01
[MQTT] Connected to broker!
[SETUP] MQTT handler initialized successfully

========================================
  Air Sensor Node Ready!
  Zone: 01
========================================

--- Reading Air Sensor Data ---
[SHT30] Temp: 22.50°C, Humidity: 65.30%
[SENSOR] Air sensor data:
  Zone ID: 01
  Air Temperature: 22.5 °C
  Air Humidity: 65.3 %
[MQTT] Sensor data published:
  Topic: sensor/air/zone01/data
  Payload: {"zone_id":"01","air_temp":22.5,"air_humidity":65.3,"timestamp":10234,"rssi":-65}
[SENSOR] Data published successfully
-------------------------------
```

### LED 상태 표시

| LED 패턴 | 의미 |
|---------|------|
| 깜빡임 (1초 주기) | 정상 동작 중 |
| 빠른 깜빡임 (0.2초) | SHT30 센서 오류 |
| 느린 깜빡임 (1초) | WiFi/MQTT 연결 오류 |
| 1초간 켜짐 | 초기화 완료 |

---

## 🛠️ 문제 해결

### 센서 초기화 실패

**증상**: `[ERROR] Failed to initialize SHT30 sensor!`

**해결 방법**:
1. I2C 연결 확인:
   - SDA → A4
   - SCL → A5
   - VCC → 5V
   - GND → GND
2. SHT30 I2C 주소 확인:
   - 기본값: `0x44`
   - 대체: `0x45` (ADDR 핀 HIGH)
3. I2C Scanner 테스트:
   ```cpp
   #include <Wire.h>
   
   void setup() {
     Wire.begin();
     Serial.begin(115200);
     Serial.println("I2C Scanner");
     
     byte error, address;
     for(address = 1; address < 127; address++ ) {
       Wire.beginTransmission(address);
       error = Wire.endTransmission();
       
       if (error == 0) {
         Serial.print("I2C device found at 0x");
         if (address < 16) Serial.print("0");
         Serial.println(address, HEX);
       }
     }
   }
   
   void loop() {}
   ```

### WiFi 연결 실패

**증상**: `[WiFi] ERROR: Connection timeout!`

**해결 방법**:
1. SSID/비밀번호 재확인 (`config.h`)
2. 2.4GHz WiFi인지 확인 (5GHz 지원 안 됨)
3. WiFi 신호 강도 확인 (RSSI > -80 dBm 권장)

### MQTT 연결 실패

**증상**: `[MQTT] ERROR: Connection failed, rc=-2`

**해결 방법**:

| Error Code | 의미 | 해결 방법 |
|-----------|------|----------|
| `-4` | Connection timeout | MQTT 브로커 IP/Port 확인 |
| `-3` | Connection lost | 네트워크 안정성 확인 |
| `-2` | Connect failed | 브로커 실행 상태 확인 |
| `5` | Connection refused | 클라이언트 ID 중복 확인 |

### CRC 오류

**증상**: `[SHT30] ERROR: Temperature/Humidity CRC mismatch`

**해결 방법**:
1. 센서와 Arduino 사이 거리 줄이기 (< 1m)
2. I2C 풀업 저항 추가 (4.7kΩ)
3. 전원 노이즈 필터 커패시터 추가 (0.1μF)

---

## 📊 성능 및 리소스

| 항목 | 값 |
|------|-----|
| 메모리 사용 (프로그램) | ~25KB / 256KB (10%) |
| 메모리 사용 (SRAM) | ~8KB / 32KB (25%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 센서 측정 시간 | ~20ms |
| 평균 전력 소비 | ~150mA @ 5V |

---

## 📁 파일 구조

```
air_sensor_node/
├── air_sensor_node.ino    # 메인 펌웨어
├── config.h               # 설정 파일
├── sht30_sensor.h         # SHT30 센서 헤더
├── sht30_sensor.cpp       # SHT30 센서 구현
├── mqtt_handler.h         # MQTT 핸들러 헤더
├── mqtt_handler.cpp       # MQTT 핸들러 구현
├── README.md              # 이 파일
└── CHANGELOG.md           # 변경 이력
```

---

## 🔄 업그레이드 가이드

### 다른 I2C 온습도 센서 사용

SHT30 대신 다른 센서를 사용하려면:

1. **DHT22** (디지털):
   - 라이브러리: `DHT sensor library`
   - 정확도: ±0.5°C, ±2~5%RH
   
2. **BME280** (I2C, 온습도+기압):
   - 라이브러리: `Adafruit BME280`
   - 추가 데이터: 기압 (hPa)

3. **SHT31** (I2C, SHT30 상위 모델):
   - 동일한 라이브러리 사용 가능
   - 정확도: ±0.1°C, ±1.5%RH

---

## 📞 지원

- **GitHub**: https://github.com/phdsjw/WasabiSmartFarm
- **작성자**: 서준원
- **이메일**: (이메일 주소)

---

## 📜 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다.
