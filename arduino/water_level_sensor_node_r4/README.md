# 수위 센서 노드 - Arduino Uno R4 WiFi용 (Water Level Sensor Node)

**Wasabi SmartFarm - 수위 센서 노드 (Node 20) - Arduino Uno R4 WiFi 버전**

Arduino Uno R4 WiFi 기반 HC-SR04 초음파 센서 노드

---

## 개요

물탱크의 수위를 HC-SR04 초음파 센서로 측정하여 MQTT로 전송하는 센서 노드입니다.

**이 버전은 Arduino Uno R4 WiFi용입니다.**  
ESP8266 (Wemos D1 R1)용 버전은 `arduino/water_level_sensor_node/` 폴더를 참조하세요.

### 노드 번호
- **노드 20**: 수위 센서 (물탱크 1개)

---

## ESP8266 버전과의 차이점

| 항목 | Arduino Uno R4 WiFi (이 버전) | ESP8266 (Wemos D1 R1) |
|------|------------------------------|----------------------|
| **MCU** | Arduino Uno R4 WiFi | Wemos D1 R1 (ESP8266) |
| **WiFi 라이브러리** | WiFiS3 | ESP8266WiFi |
| **시스템 재시작** | NVIC_SystemReset() | ESP.restart() |
| **LED 동작** | HIGH=ON, LOW=OFF | LOW=ON, HIGH=OFF (반전) |
| **핀 번호** | D7 (Trig), D8 (Echo) | D1 (Trig), D2 (Echo) |
| **메모리 확인** | freeMemory() (더미) | ESP.getFreeHeap() |
| **전원** | 5V (USB/DC) | 5V (USB/DC) |
| **가격** | 약 40,000원 | 약 5,000원 |

---

## 하드웨어 구성

### MCU
- **Arduino Uno R4 WiFi** × 1

### 센서
- **HC-SR04** - 초음파 거리 센서
  - 측정 범위: 2cm ~ 400cm
  - 정확도: ±3mm
  - 전원: 5V
  - 초음파: 40kHz

---

## 연결 핀맵

### HC-SR04
| HC-SR04 핀 | Arduino R4 WiFi 핀 |
|-----------|-------------------|
| VCC | 5V |
| GND | GND |
| Trig | D7 |
| Echo | D8 |

---

## 소프트웨어 구성

### 필수 라이브러리
1. `WiFiS3` - Arduino Uno R4 WiFi용 WiFi 라이브러리 (보드 패키지에 포함)
2. `PubSubClient` - MQTT 클라이언트
3. `ArduinoJson` - JSON 직렬화/역직렬화

### 설치 방법

#### 1. Arduino IDE 보드 설정
Arduino IDE에서:
1. `도구` → `보드` → `Arduino Uno R4 WiFi` 선택

#### 2. 라이브러리 설치
Arduino IDE에서 `도구` → `라이브러리 관리`:
- `PubSubClient` - by Nick O'Leary
- `ArduinoJson` - by Benoit Blanchon (v6.x 권장)

---

## 설정 방법

### 1. config.h 수정

`config.h` 파일을 열어 다음 항목을 수정합니다:

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

#### 물탱크 설정 (중요!)
```cpp
#define TANK_HEIGHT_CM    100.0  // 물탱크 실제 높이 (cm)
#define SENSOR_OFFSET_CM  5.0    // 센서 설치 오프셋 (cm)
```

**물탱크 높이 설정 방법:**
1. 물탱크의 실제 높이를 측정 (바닥부터 상단까지)
2. `TANK_HEIGHT_CM`에 측정값 입력
3. 센서는 물탱크 상단에서 아래를 향해 설치
4. 센서 설치 위치 오프셋이 있으면 `SENSOR_OFFSET_CM`에 입력

---

## MQTT 토픽

### 수위 데이터
- **토픽**: `sensor/water_level/data`
- **형식**: JSON
- **예시**:
```json
{
  "node_id": "20",
  "distance_cm": 45.3,
  "water_level_percent": 54.7,
  "timestamp": 123456
}
```

### 하트비트
- **토픽**: `sensor/water_level/heartbeat`
- **형식**: JSON
- **예시**:
```json
{
  "node_id": "20",
  "node_type": "water_level",
  "uptime": 123456,
  "wifi_rssi": -45,
  "free_memory": 0,
  "timestamp": 123456
}
```

### 상태
- **토픽**: `sensor/water_level/status`
- **형식**: JSON

---

## 데이터 전송 주기

| 항목 | 주기 |
|-----|------|
| 수위 센서 데이터 | 3초 |
| 하트비트 | 60초 |

---

## 수위 계산 방식

```
수위(%) = (탱크 높이 - 측정 거리 - 오프셋) / 탱크 높이 × 100
```

**예시:**
- 탱크 높이: 100cm
- 센서 오프셋: 5cm
- 측정 거리: 45cm
- 수위: (100 - 45 - 5) / 100 × 100 = 50%

---

## 업로드 방법

### 1. Arduino IDE 설정
- `도구` → `보드` → `Arduino Uno R4 WiFi` 선택
- `도구` → `포트` → 올바른 COM 포트 선택

### 2. 설정 확인
- `config.h` 파일에서 WiFi/MQTT/물탱크 설정 확인

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

### HC-SR04 센서 읽기 실패
- 연결 확인 (Trig: D7, Echo: D8, VCC, GND)
- 센서 전원 확인 (5V 필요)
- 센서 방향 확인 (아래를 향해야 함)
- 측정 범위 확인 (2-400cm)
- 장애물 확인 (센서 앞 장애물 제거)

### 수위 값이 이상함
- `TANK_HEIGHT_CM` 값 확인 (실제 탱크 높이와 일치하는지)
- `SENSOR_OFFSET_CM` 값 확인
- 센서 설치 위치 확인 (탱크 상단 중앙 권장)
- 시리얼 모니터에서 "distance_cm" 값 확인

### 시스템이 멈춤
- WiFi/MQTT 연결 실패 시 5초 후 자동 재시작됨
- NVIC_SystemReset() 함수 동작 확인

### LED 깜빡임 의미
- **느린 깜빡임 (1초)**: MQTT 연결 정상
- **빠른 깜빡임 (500ms)**: MQTT 연결 끊김

---

## ESP8266 버전과 선택 가이드

### Arduino Uno R4 WiFi를 선택하는 경우
- 다른 센서 노드와 동일한 MCU 사용 (통일성)
- 더 많은 메모리 필요 (32KB SRAM vs 80KB)
- 5V 로직 레벨 필요
- Arduino 생태계 선호

### ESP8266 (Wemos D1 R1)을 선택하는 경우
- 비용 절감 (약 5,000원 vs 40,000원)
- 충분한 성능 (수위 측정에는 충분)
- 작은 크기
- ESP8266 생태계 선호

**권장**: 예산이 충분하면 Arduino Uno R4 WiFi, 비용 절감이 목표면 ESP8266

---

## 센서 설치 가이드

### 1. 설치 위치
- 물탱크 상단 중앙에 설치
- 센서가 수직 아래를 향하도록 설치
- 탱크 벽면에서 최소 10cm 이상 떨어진 위치

### 2. 설치 주의사항
- 센서와 수면 사이에 장애물이 없어야 함
- 방수 처리 필요 (센서 자체는 방수 아님)
- 전원선과 신호선 분리 (노이즈 방지)
- 케이블 길이는 최소화 (신호 감쇠 방지)

### 3. 초기 보정
1. 물탱크가 비어있을 때 측정
2. 시리얼 모니터에서 "distance_cm" 값 확인
3. 이 값이 탱크 높이와 유사하면 정상
4. 차이가 크면 `SENSOR_OFFSET_CM` 조정

---

## 파일 구조

```
water_level_sensor_node_r4/
├── water_level_sensor_node_r4.ino  # 메인 코드 (R4 WiFi용)
├── config.h                         # 설정 파일
├── hcsr04_sensor.h                  # HC-SR04 센서 헤더
├── hcsr04_sensor.cpp                # HC-SR04 센서 구현
├── mqtt_handler.h                   # MQTT 핸들러 헤더 (R4 WiFi용)
├── mqtt_handler.cpp                 # MQTT 핸들러 구현 (R4 WiFi용)
└── README.md                        # 본 문서
```

---

## 버전 이력

### v1.0.0 (2025-12-27)
- 초기 버전 (Arduino Uno R4 WiFi용)
- HC-SR04 초음파 센서 지원
- 다중 샘플 평균 필터링 (5 샘플)
- WiFi/MQTT 통신 (WiFiS3 라이브러리)
- 3초 주기 데이터 전송
- 60초 주기 하트비트 전송
- 자동 재시작 기능 (NVIC_SystemReset)

---

## 관련 문서

- ESP8266 버전: `arduino/water_level_sensor_node/README.md`
- 센서 노드 재구성: `SENSOR_NODE_REORGANIZATION.md`

---

## 라이선스

MIT License

---

## 작성자

서준원

## 참고

- [Arduino Uno R4 WiFi](https://docs.arduino.cc/hardware/uno-r4-wifi)
- [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
- [WiFiS3 Library](https://www.arduino.cc/reference/en/libraries/wifis3/)
