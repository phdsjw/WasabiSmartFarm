# 💧 와사비 스마트팜 - 수조 센서 노드

**작성자**: 서준원  
**버전**: v1.0.0  
**날짜**: 2024-12-11

---

## 📋 개요

메인 수조의 수질을 실시간으로 측정하여 MQTT를 통해 Node-RED로 전송하는 센서 노드입니다.  
수온, pH, TDS, EC 4가지 수질 파라미터를 모니터링합니다.

---

## 🎯 주요 기능

- ✅ **DS18B20 수온 센서 지원** (1-Wire 디지털 통신)
- ✅ **SEN0161 pH 센서 지원** (아날로그, 자동 보정)
- ✅ **SEN0244 TDS 센서 지원** (아날로그, 온도 보상)
- ✅ **SEN0451 Pro EC 센서 지원** (아날로그, 온도 보상)
- ✅ **WiFi 연결 및 자동 재연결**
- ✅ **MQTT 프로토콜 기반 데이터 전송**
- ✅ **10초 주기 센서 데이터 수집 및 전송**
- ✅ **하트비트 전송** (1분 주기)
- ✅ **LED 상태 표시**
- ✅ **센서 오류 감지 및 자동 재초기화**
- ✅ **다중 샘플링 및 평균 필터링** (노이즈 제거)

---

## 🔧 하드웨어 구성

### 필수 부품

| 부품명 | 수량 | 사양 | 용도 |
|--------|------|------|------|
| Arduino Uno R4 WiFi | 1개 | MCU | 메인 컨트롤러 |
| DS18B20 | 1개 | 1-Wire, -55~125°C | 수온 측정 |
| SEN0161 | 1개 | Analog, pH 0~14 | pH 측정 |
| SEN0244 | 1개 | Analog, 0~1000 ppm | TDS 측정 |
| SEN0451 Pro | 1개 | Analog, 0~20 mS/cm | EC 측정 |
| 4.7kΩ 저항 | 1개 | 1-Wire 풀업 | DS18B20용 |
| 5V 전원 어댑터 | 1개 | 5V/3A | 전원 공급 |
| USB-C 케이블 | 1개 | - | 전원 및 프로그래밍 |
| Dupont 케이블 | 다수 | M-F | 센서 연결 |

### 센서 사양

#### DS18B20 (수온 센서)
| 항목 | 사양 |
|------|------|
| 측정 범위 | -55°C ~ +125°C |
| 정확도 | ±0.5°C (-10~85°C) |
| 해상도 | 0.0625°C (12비트) |
| 통신 방식 | 1-Wire |
| 방수 등급 | IP68 (스테인리스 프로브) |

#### SEN0161 (pH 센서)
| 항목 | 사양 |
|------|------|
| 측정 범위 | pH 0 ~ 14 |
| 정확도 | ±0.1 pH @ 25°C |
| 응답 시간 | < 1분 |
| 통신 방식 | 아날로그 (0~5V) |
| 전원 전압 | 5V DC |

#### SEN0244 (TDS 센서)
| 항목 | 사양 |
|------|------|
| 측정 범위 | 0 ~ 1000 ppm |
| 정확도 | ±10% @ 25°C |
| 응답 시간 | < 10초 |
| 온도 보상 | 자동 (알고리즘) |
| 통신 방식 | 아날로그 (0~5V) |

#### SEN0451 Pro (EC 센서)
| 항목 | 사양 |
|------|------|
| 측정 범위 | 0 ~ 20 mS/cm |
| 정확도 | ±5% @ 25°C |
| 응답 시간 | < 10초 |
| 온도 보상 | 자동 (알고리즘) |
| 통신 방식 | 아날로그 (0~5V) |

---

## ⚡ 하드웨어 연결

### 배선도

```
Arduino Uno R4 WiFi         센서
====================        =====

[DS18B20 (1-Wire)]
D4        ---------->       Data (노랑) + 4.7kΩ 풀업 저항 (D4 ~ 5V)
5V        ---------->       VCC (빨강)
GND       ---------->       GND (검정)

[SEN0161 (pH)]
A0        <----------       Signal (파랑)
5V        ---------->       VCC (빨강)
GND       ---------->       GND (검정)

[SEN0244 (TDS)]
A1        <----------       Signal (파랑)
5V        ---------->       VCC (빨강)
GND       ---------->       GND (검정)

[SEN0451 Pro (EC)]
A2        <----------       Signal (파랑)
5V        ---------->       VCC (빨강)
GND       ---------->       GND (검정)
```

### 중요 사항
1. **DS18B20 풀업 저항**: D4와 5V 사이에 4.7kΩ 저항 필수!
2. **전원 공급**: 모든 센서는 5V 전원 사용
3. **GND 공통**: 모든 GND는 함께 연결
4. **프로브 설치**: 모든 센서 프로브를 수조에 담그기

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
| `OneWire` | 2.3+ | 1-Wire 프로토콜 |
| `DallasTemperature` | 3.9+ | DS18B20 온도 센서 |

### 2. config.h 설정

#### 2.1 WiFi 설정

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
```

#### 2.2 MQTT 설정

```cpp
#define MQTT_SERVER "192.168.0.100"   // Node-RED 서버 IP
#define MQTT_PORT 1883
```

#### 2.3 센서 보정 (선택사항)

센서 보정이 필요한 경우:

```cpp
// pH 센서 보정
#define PH_CALIBRATION_OFFSET 0.0    // pH 오프셋 (예: +0.2)
#define PH_CALIBRATION_SLOPE 3.5     // 기울기 (기본: 3.5)

// TDS 센서 보정
#define TDS_K_VALUE 1.0              // K 값 (기본: 1.0)

// EC 센서 보정
#define EC_K_VALUE 1.0               // K 값 (기본: 1.0)
```

### 3. 펌웨어 업로드

1. Arduino IDE에서 `water_tank_sensor_node.ino` 파일 열기
2. `도구` → `보드` → `Arduino Uno R4 WiFi` 선택
3. `도구` → `포트` → 연결된 포트 선택
4. `config.h`에서 WiFi/MQTT 설정 확인
5. `업로드` 버튼 클릭

---

## 📡 MQTT 토픽 및 메시지

### 센서 데이터 (10초 주기)

**Topic**: `sensor/water_tank/data`

**Payload (JSON)**:
```json
{
  "water_temp": 18.5,
  "water_ph": 6.8,
  "water_tds": 450,
  "water_ec": 1.2,
  "timestamp": 1702284000000,
  "rssi": -65
}
```

| 필드 | 타입 | 단위 | 설명 |
|------|------|------|------|
| `water_temp` | float | °C | 수온 (소수점 1자리) |
| `water_ph` | float | pH | pH (소수점 2자리) |
| `water_tds` | int | ppm | TDS (정수) |
| `water_ec` | float | mS/cm | EC (소수점 2자리) |
| `timestamp` | long | ms | 측정 시간 (밀리초) |
| `rssi` | int | dBm | WiFi 신호 강도 |

### 하트비트 (60초 주기)

**Topic**: `sensor/water_tank/heartbeat`

**Payload (JSON)**:
```json
{
  "status": "alive",
  "uptime": 3600000,
  "rssi": -65
}
```

### 상태 메시지

**Topic**: `sensor/water_tank/status`

**Payload (JSON)**:
```json
{
  "status": "initialized",
  "timestamp": 1702284000000
}
```

---

## 🔍 테스트 및 디버그

### 시리얼 모니터 출력 확인

1. Arduino IDE → `도구` → `시리얼 모니터` 열기
2. 보드레이트: **115200 bps** 설정
3. 출력 예시:

```
========================================
  Wasabi SmartFarm
  Water Tank Sensor Node
========================================
  Version: v1.0.0
  Author: 서준원
  Date: 2024-12-11
========================================
  Sensors:
    - DS18B20 (Water Temp, 1-Wire D4)
    - SEN0161 (pH, Analog A0)
    - SEN0244 (TDS, Analog A1)
    - SEN0451 Pro (EC, Analog A2)
  WiFi SSID: your_wifi_ssid
  MQTT Broker: 192.168.0.100:1883
  Sensor Read Interval: 10 sec
  Heartbeat Interval: 60 sec
========================================

[SETUP] Initializing water tank sensors...
[SENSORS] DS18B20 devices found: 1
[SENSORS] DS18B20 initialized successfully
[SENSORS] Analog sensors initialized
[SETUP] Water sensors initialized successfully

[MQTT] Initializing MQTT Handler...
[WiFi] Connecting to: your_wifi_ssid
...........
[WiFi] Connected!
[WiFi] IP Address: 192.168.0.104
[MQTT] Connected to broker!

========================================
  Water Tank Sensor Node Ready!
========================================

--- Reading Water Tank Sensors ---
[SENSORS] Water Temperature: 18.50°C
[SENSORS] pH: 6.80 (voltage: 2.50V)
[SENSORS] TDS: 450 ppm (voltage: 1.20V, temp: 18.5°C)
[SENSORS] EC: 1.20 mS/cm (voltage: 1.50V, temp: 18.5°C)
[SENSOR] Water tank sensor data:
  Water Temperature: 18.5 °C
  Water pH: 6.80
  Water TDS: 450 ppm
  Water EC: 1.20 mS/cm
[MQTT] Sensor data published:
  Topic: sensor/water_tank/data
  Payload: {"water_temp":18.5,"water_ph":6.80,"water_tds":450,"water_ec":1.20,"timestamp":10234,"rssi":-65}
[SENSOR] Data published successfully
----------------------------------
```

### LED 상태 표시

| LED 패턴 | 의미 |
|---------|------|
| 깜빡임 (1초 주기) | 정상 동작 중 |
| 빠른 깜빡임 (0.2초) | 센서 오류 |
| 느린 깜빡임 (1초) | WiFi/MQTT 연결 오류 |
| 1초간 켜짐 | 초기화 완료 |

---

## 🛠️ 문제 해결

### DS18B20 센서 감지 안 됨

**증상**: `[WARNING] No DS18B20 temperature sensor found!`

**해결 방법**:
1. 풀업 저항 확인 (4.7kΩ, D4 ~ 5V)
2. 배선 확인:
   - Data → D4
   - VCC → 5V
   - GND → GND
3. 1-Wire Scanner 테스트:
   ```cpp
   #include <OneWire.h>
   
   OneWire ds(4);  // D4
   
   void setup() {
     Serial.begin(115200);
     byte addr[8];
     
     if (ds.search(addr)) {
       Serial.print("Found device: ");
       for(byte i = 0; i < 8; i++) {
         Serial.print(addr[i], HEX);
         Serial.print(" ");
       }
       Serial.println();
     } else {
       Serial.println("No devices found");
     }
   }
   
   void loop() {}
   ```

### pH 값이 부정확함

**증상**: pH 값이 예상과 다름

**해결 방법**:
1. **pH 보정액 사용** (pH 4.0, 7.0, 10.0)
2. pH 7.0 용액에 담그고 전압 확인
3. `config.h`에서 보정값 조정:
   ```cpp
   #define PH_CALIBRATION_OFFSET 0.0  // 측정값 - 7.0
   ```
4. 센서 전극 세척 (증류수)
5. 전극 수명 확인 (6개월~1년)

### TDS/EC 값이 불안정함

**증상**: TDS/EC 값이 계속 변동

**해결 방법**:
1. 수온 안정화 대기 (5분)
2. 센서 프로브 세척
3. 샘플링 횟수 증가:
   ```cpp
   #define ANALOG_SAMPLE_COUNT 20  // 기본: 10
   ```
4. K 값 보정 (표준 용액 사용)

### WiFi 연결 실패

**증상**: `[WiFi] ERROR: Connection timeout!`

**해결 방법**:
1. SSID/비밀번호 재확인
2. 2.4GHz WiFi 사용 확인 (5GHz 지원 안 됨)
3. WiFi 신호 강도 확인 (RSSI > -80 dBm)

---

## 📊 성능 및 리소스

| 항목 | 값 |
|------|-----|
| 메모리 사용 (프로그램) | ~28KB / 256KB (11%) |
| 메모리 사용 (SRAM) | ~10KB / 32KB (31%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 센서 측정 시간 | ~1.5초 |
| 평균 전력 소비 | ~180mA @ 5V |

---

## 🧪 센서 보정 가이드

### pH 센서 보정 (2점 보정)

1. **준비물**:
   - pH 4.0 표준 용액
   - pH 7.0 표준 용액 (또는 10.0)
   - 증류수

2. **보정 절차**:
   ```
   Step 1: 센서를 증류수로 세척
   Step 2: pH 7.0 용액에 담그고 2분 대기
   Step 3: 시리얼 모니터에서 전압 확인 (2.5V가 정상)
   Step 4: pH 4.0 용액에 담그고 전압 확인
   Step 5: config.h에서 보정값 입력
   ```

### TDS/EC 센서 보정

1. **준비물**:
   - TDS 표준 용액 (예: 500 ppm)
   - EC 표준 용액 (예: 1.413 mS/cm)

2. **보정 절차**:
   ```
   Step 1: 표준 용액에 센서 담그기
   Step 2: 측정값과 표준값 비교
   Step 3: K 값 계산: K = 표준값 / 측정값
   Step 4: config.h에서 K_VALUE 수정
   ```

---

## 📁 파일 구조

```
water_tank_sensor_node/
├── water_tank_sensor_node.ino  # 메인 펌웨어
├── config.h                     # 설정 파일
├── water_sensors.h              # 수조 센서 헤더
├── water_sensors.cpp            # 수조 센서 구현
├── mqtt_handler.h               # MQTT 핸들러 헤더
├── mqtt_handler.cpp             # MQTT 핸들러 구현
├── README.md                    # 이 파일
└── CHANGELOG.md                 # 변경 이력
```

---

## 🔄 업그레이드 가이드

### 다른 센서로 교체

#### pH 센서 교체 (SEN0161 → 다른 모델)
- Analog pH 센서: 전압-pH 변환 공식 수정
- I2C pH 센서: `water_sensors.cpp` 수정 필요

#### TDS 센서 교체 (SEN0244 → SEN0798)
- SEN0798: 더 넓은 범위 (0-1000, 1000-2000 ppm)
- 변환 공식만 수정하면 호환 가능

#### EC 센서 업그레이드
- 더 정밀한 EC 센서 사용 시 K 값만 재보정

---

## 📞 지원

- **GitHub**: https://github.com/phdsjw/WasabiSmartFarm
- **작성자**: 서준원

---

## 📜 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다.
