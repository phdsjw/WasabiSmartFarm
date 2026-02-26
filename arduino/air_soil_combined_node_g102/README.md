# 대기+토양 통합 센서 노드 (Air+Soil Combined Sensor Node)

**Wasabi SmartFarm - 대기+토양 통합 센서 노드**

Arduino Uno R4 WiFi 기반 SHT30 + SEN0604 통합 센서 노드

---

## 개요

하나의 Arduino Uno R4 WiFi 보드에서 대기 센서(SHT30)와 토양 센서(SEN0604)를 동시에 읽어 MQTT로 전송하는 통합 센서 노드입니다.

### 노드 번호
- **노드 13**: Zone 13, Tank 13
- **노드 14**: Zone 14, Tank 14
- **노드 15**: Zone 15, Tank 15

---

## 하드웨어 구성

### MCU
- **Arduino Uno R4 WiFi** × 1

### 센서
1. **SHT30** - 대기 온습도 센서 (I2C)
   - 온도: -40°C ~ +125°C (±0.2°C)
   - 습도: 0% ~ 100% RH (±2% RH)
   - I2C 주소: 0x44 (기본값)

2. **SEN0604** - 4-in-1 토양 센서 (RS485 Modbus RTU)
   - 토양 온도: -40°C ~ +80°C
   - 토양 습도: 0% ~ 100%
   - 토양 EC: 0 ~ 20,000 μS/cm
   - 토양 pH: 3 ~ 9
   - Modbus Slave ID: 1 (기본값)
   - 보드레이트: 4800 bps

### 추가 하드웨어
- RS485 확장보드 (DFR0259 또는 호환)

---

## 연결 핀맵

### SHT30 (I2C)
| SHT30 핀 | Arduino R4 WiFi 핀 |
|----------|-------------------|
| VCC | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

### SEN0604 (RS485)
| 기능 | Arduino R4 WiFi 핀 |
|------|-------------------|
| TX | D1 (Serial1 TX) |
| RX | D0 (Serial1 RX) |
| DE/RE | D2 |

---

## 소프트웨어 구성

### 필수 라이브러리
1. `WiFiS3` - Arduino Uno R4 WiFi용 WiFi 라이브러리
2. `PubSubClient` - MQTT 클라이언트
3. `ArduinoJson` - JSON 직렬화/역직렬화
4. `Adafruit_SHT31` - SHT30 센서 라이브러리
5. `ArduinoModbus` - Modbus RTU 통신
6. `ArduinoRS485` - RS485 통신

### 설치 방법
Arduino IDE에서 `도구` → `라이브러리 관리` → 각 라이브러리 검색 후 설치

---

## 설정 방법

### 1. config.h 수정

`config.h` 파일을 열어 다음 항목을 수정합니다:

#### 노드 ID 설정 (중요!)
```cpp
// 노드 13
#define ZONE_ID "13"
#define TANK_ID "13"

// 노드 14
// #define ZONE_ID "14"
// #define TANK_ID "14"

// 노드 15
// #define ZONE_ID "15"
// #define TANK_ID "15"
```

#### WiFi 설정
```cpp
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"
```

#### MQTT 설정
```cpp
#define MQTT_SERVER      "192.168.0.100"  // MQTT Broker IP
#define MQTT_PORT        1883
```

---

## MQTT 토픽

### 노드 13 기준

#### 대기 센서 데이터
- **토픽**: `sensor/air/zone13/data`
- **형식**: JSON
- **예시**:
```json
{
  "zone_id": "13",
  "air_temp": 22.5,
  "air_humidity": 65.2,
  "timestamp": 123456
}
```

#### 토양 센서 데이터
- **토픽**: `sensor/soil/tank13/data`
- **형식**: JSON
- **예시**:
```json
{
  "tank_id": "13",
  "soil_temp": 20.1,
  "soil_moisture": 85.5,
  "soil_ec": 1250.0,
  "soil_ph": 6.50,
  "timestamp": 123456
}
```

#### 하트비트
- **토픽**: `sensor/combined/zone13/heartbeat`
- **형식**: JSON
- **예시**:
```json
{
  "zone_id": "13",
  "tank_id": "13",
  "node_type": "combined",
  "uptime": 123456,
  "wifi_rssi": -45,
  "free_memory": 0,
  "timestamp": 123456
}
```

#### 상태
- **토픽**: `sensor/combined/zone13/status`
- **형식**: JSON

---

## 데이터 전송 주기

| 항목 | 주기 |
|-----|------|
| 대기 센서 데이터 | 10초 |
| 토양 센서 데이터 | 10초 |
| 하트비트 | 60초 |

---

## 업로드 방법

### 1. Arduino IDE 설정
- `도구` → `보드` → `Arduino Uno R4 WiFi` 선택
- `도구` → `포트` → 올바른 COM 포트 선택

### 2. 노드 ID 설정
- `config.h` 파일에서 `ZONE_ID`와 `TANK_ID`를 해당 노드 번호로 설정
- **노드 13**: ZONE_ID="13", TANK_ID="13"
- **노드 14**: ZONE_ID="14", TANK_ID="14"
- **노드 15**: ZONE_ID="15", TANK_ID="15"

### 3. 컴파일 및 업로드
- `스케치` → `확인/컴파일` (오류 확인)
- `스케치` → `업로드`

### 4. 시리얼 모니터 확인
- `도구` → `시리얼 모니터` (115200 baud)
- 초기화 메시지, WiFi/MQTT 연결 상태, 센서 데이터 확인

---

## 문제 해결

### WiFi 연결 실패
- SSID/비밀번호 확인
- 2.4GHz WiFi 사용 확인 (5GHz 지원 안됨)
- 공유기 거리 확인

### MQTT 연결 실패
- MQTT Broker IP/포트 확인
- 방화벽 설정 확인 (포트 1883 열림)
- Broker 실행 상태 확인

### SHT30 센서 읽기 실패
- I2C 연결 확인 (SDA, SCL, VCC, GND)
- I2C 주소 확인 (0x44 또는 0x45)
- 풀업 저항 확인 (대부분 내장됨)

### SEN0604 센서 읽기 실패
- RS485 연결 확인 (A, B, TX, RX, DE/RE)
- 보드레이트 확인 (4800 bps)
- Slave ID 확인 (기본값: 1)
- RS485 터미네이션 저항 확인

### LED 깜빡임 의미
- **느린 깜빡임 (1초)**: MQTT 연결 정상
- **빠른 깜빡임 (500ms)**: MQTT 연결 끊김

---

## 파일 구조

```
air_soil_combined_node/
├── air_soil_combined_node.ino  # 메인 코드
├── config.h                     # 설정 파일
├── sht30_sensor.h               # SHT30 센서 헤더
├── sht30_sensor.cpp             # SHT30 센서 구현
├── sen0604_modbus.h             # SEN0604 센서 헤더
├── sen0604_modbus.cpp           # SEN0604 센서 구현
├── mqtt_handler.h               # MQTT 핸들러 헤더
├── mqtt_handler.cpp             # MQTT 핸들러 구현
└── README.md                    # 본 문서
```

---

## 버전 이력

### v1.0.0 (2025-12-27)
- 초기 버전
- SHT30 + SEN0604 통합 센서 노드
- WiFi/MQTT 통신
- 10초 주기 데이터 전송
- 60초 주기 하트비트 전송

---

## 라이선스

MIT License

---

## 작성자

서준원

## 참고

- [Arduino Uno R4 WiFi](https://docs.arduino.cc/hardware/uno-r4-wifi)
- [SHT30 Datasheet](https://www.sensirion.com/sht30)
- [SEN0604 Wiki](https://wiki.dfrobot.com/SEN0604)
