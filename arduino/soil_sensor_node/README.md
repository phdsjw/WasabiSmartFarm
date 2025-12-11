# 토양 센서 노드 펌웨어

**Wasabi SmartFarm - Soil Sensor Node**

각 재배상(Tank)의 토양 환경을 모니터링하는 센서 노드 펌웨어입니다.

---

## 📋 개요

- **노드 수량**: 18개 (각 재배상마다 1개)
- **센서**: SEN0604 (4-in-1 토양 센서)
- **측정 데이터**: 토양 온도, 토양 습도, 토양 EC, 토양 pH
- **전송 주기**: 10초
- **통신**: WiFi + MQTT

---

## 🔧 하드웨어 구성

### 필수 부품
| 부품 | 모델명 | 수량 | 용도 |
|------|-------|------|------|
| **MCU** | Arduino Uno R4 WiFi | 1 | 메인 컨트롤러 |
| **RS485 확장보드** | DFR0259 | 1 | Modbus RTU 통신 |
| **토양 센서** | SEN0604 (4-in-1) | 1 | 토양 측정 |
| **전원** | 5V/3A | 1 | 전원 공급 |

### 배선도

```
Arduino Uno R4 WiFi
├─ A4 (SDA) ────────┐
├─ A5 (SCL) ────────┼─── I2C (사용 안 함)
├─ D0 (RX) ─────────┼─── RS485 RX (Serial1)
├─ D1 (TX) ─────────┼─── RS485 TX (Serial1)
├─ D2 ──────────────┼─── RS485 DE/RE (TX Enable)
└─ 5V, GND ─────────┴─── 전원

RS485 확장보드 (DFR0259)
├─ A ──────────────────── SEN0604 A
├─ B ──────────────────── SEN0604 B
└─ GND ────────────────── SEN0604 GND

SEN0604 토양 센서
├─ VCC (12V) ──────────── 12V 전원
├─ GND ────────────────── GND
├─ A ──────────────────── RS485 A
└─ B ──────────────────── RS485 B
```

---

## 💻 소프트웨어 설정

### 1. Arduino IDE 설정

#### 필수 라이브러리
다음 라이브러리를 Arduino IDE에서 설치하세요:
- `WiFiS3` (Arduino Uno R4 WiFi 내장)
- `PubSubClient` (by Nick O'Leary) - MQTT 클라이언트
- `ArduinoModbus` (by Arduino) - Modbus RTU
- `ArduinoJson` (by Benoit Blanchon) - JSON 파싱

#### 보드 설정
- **보드**: Arduino Uno R4 WiFi
- **포트**: Arduino가 연결된 COM 포트 선택

### 2. config.h 수정

`config.h` 파일을 열고 다음 항목을 수정하세요:

```cpp
// ============================================
// 노드 식별 정보 (각 노드마다 변경 필요)
// ============================================
#define TANK_ID "01"  // 01~18 중 하나로 설정

// ============================================
// WiFi 설정
// ============================================
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// ============================================
// MQTT 설정
// ============================================
#define MQTT_SERVER      "192.168.0.100"  // Node-RED 서버 IP
#define MQTT_PORT        1883

// ============================================
// Modbus RTU 설정 (SEN0604)
// ============================================
#define MODBUS_SLAVE_ID        1       // SEN0604 Slave ID
#define MODBUS_BAUDRATE        4800    // SEN0604 보드레이트
```

---

## 🚀 업로드 및 테스트

### 1. 펌웨어 업로드

1. Arduino IDE에서 `soil_sensor_node.ino` 파일 열기
2. `config.h`에서 `TANK_ID` 설정 (예: "01", "02", ..., "18")
3. WiFi 및 MQTT 설정 확인
4. 컴파일 및 업로드 (Ctrl+U)

### 2. 시리얼 모니터 확인

**보드레이트**: 115200

**정상 출력 예시**:
```
╔════════════════════════════════════════╗
║  Wasabi SmartFarm - Soil Sensor Node  ║
╚════════════════════════════════════════╝
Tank ID: 01
Version: 1.0.0
Sensor: SEN0604 (4-in-1)

[SETUP] Connecting to WiFi...
[WiFi] SSID: YOUR_WIFI_SSID
..........
[WiFi] Connected!
[WiFi] IP Address: 192.168.0.101

[SETUP] Connecting to MQTT Broker...
[MQTT] Client ID: WasabiSoil_Tank01
[MQTT] Data Topic: sensor/soil/tank01/data
[MQTT] Heartbeat Topic: sensor/soil/tank01/heartbeat
[MQTT] Connected!

[SETUP] Initializing SEN0604 sensor...
[SEN0604] Initializing Modbus RTU...
[SEN0604] Modbus RTU initialized successfully
[SEN0604] Sensor connected!

[SETUP] Setup complete!

========================================
[SENSOR] Reading soil sensor (Tank 01)...
[SEN0604] Reading registers from slave ID 1
[SEN0604] Data read successfully:
  Moisture: 92.5 %
  Temp: 20.3 °C
  EC: 3200.0 μS/cm
  pH: 6.52
┌─────────────────────────────────────┐
│       Soil Sensor Data              │
├─────────────────────────────────────┤
│ Soil Temperature  : 20.3 °C        │
│ Soil Moisture     : 92.5 %         │
│ Soil EC           : 3200.0 μS/cm   │
│ Soil pH           : 6.52           │
│                                     │
└─────────────────────────────────────┘
[MQTT] Publishing to: sensor/soil/tank01/data
[MQTT] Payload: {"tank_id":"01","soil_temp":20.3,"soil_moisture":92.5,"soil_ec":3200.0,"soil_ph":6.52,"timestamp":12345}
[MQTT] Published successfully
[SUCCESS] Data published to MQTT
========================================
```

---

## 📊 MQTT 데이터 포맷

### 센서 데이터 Topic
```
sensor/soil/tank{01~18}/data
```

### JSON Payload
```json
{
  "tank_id": "01",
  "soil_temp": 20.3,
  "soil_moisture": 92.5,
  "soil_ec": 3200.0,
  "soil_ph": 6.52,
  "timestamp": 1702284000000
}
```

### 하트비트 Topic
```
sensor/soil/tank{01~18}/heartbeat
```

### JSON Payload
```json
{
  "tank_id": "01",
  "status": "online",
  "uptime": 3600,
  "timestamp": 1702284000000
}
```

---

## 🐛 문제 해결

### 1. WiFi 연결 실패
**증상**: `[WiFi] Connection failed!`

**해결 방법**:
- `config.h`에서 SSID와 비밀번호 확인
- WiFi 신호 강도 확인
- 2.4GHz WiFi 사용 확인 (5GHz 미지원)

### 2. MQTT 연결 실패
**증상**: `[MQTT] Connection failed, rc=-2`

**해결 방법**:
- Mosquitto MQTT Broker가 실행 중인지 확인
- `config.h`에서 MQTT_SERVER IP 주소 확인
- 방화벽에서 1883 포트 열기

### 3. 센서 데이터 읽기 실패
**증상**: `[SEN0604] Error: Failed to read sensor data`

**해결 방법**:
- RS485 배선 확인 (A, B 연결)
- SEN0604 전원 확인 (12V)
- Modbus Slave ID 확인 (`MODBUS_SLAVE_ID` 설정)
- **보드레이트 확인** (`MODBUS_BAUDRATE`)
  - 공장 기본값: **9600** bps
  - 지원: 2400 / 4800 / 9600
  - config.h에서 `#define MODBUS_BAUDRATE 9600`으로 변경 시도

### 4. Modbus 통신 오류
**증상**: `[SEN0604] Modbus error: 4`

**에러 코드**:
- `1`: Illegal Function
- `2`: Illegal Data Address
- `3`: Illegal Data Value
- `4`: Slave Device Failure
- `E0`: Invalid Slave ID
- `E1`: Invalid Function
- `E2`: Response Timeout
- `E3`: Frame Error

**해결 방법**:
- RS485 TX Enable 핀 확인 (D2)
- RS485 A/B 극성 확인
- 센서 Slave ID 변경 시 `config.h` 업데이트

---

## 📈 LED 상태 표시

| LED 패턴 | 상태 |
|---------|------|
| 천천히 깜빡임 (1초 간격) | WiFi + MQTT 연결됨 (정상) |
| 빠르게 깜빡임 | WiFi 또는 MQTT 연결 안됨 |
| 켜짐 | 센서 데이터 전송 중 |
| 꺼짐 | 대기 중 |

---

## 🔄 다중 노드 배포

18개 노드에 펌웨어를 업로드할 때:

1. `config.h`에서 `TANK_ID`만 변경
2. Tank 01 → `#define TANK_ID "01"`
3. Tank 02 → `#define TANK_ID "02"`
4. ...
5. Tank 18 → `#define TANK_ID "18"`

**팁**: 각 노드에 라벨을 붙여 Tank ID 표시

---

## 📝 라이선스

MIT License

---

**작성자**: 서준원  
**버전**: 1.0.0  
**최종 수정**: 2024-12-11
