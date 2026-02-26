# ⚙️ 와사비 스마트팜 - 액추에이터 노드

**작성자**: 서준원  
**버전**: v1.0.0  
**날짜**: 2024-12-11

---

## 📋 개요

스마트팜의 관수/배수 펌프 및 기타 액추에이터를 제어하는 중앙 제어 노드입니다.  
MQTT 명령을 수신하여 4채널 릴레이를 통해 고전력 장비를 안전하게 제어합니다.

---

## 🎯 주요 기능

### 제어 기능
- ✅ **관수 펌프 제어** (2HP, CH1)
- ✅ **배수 펌프 제어** (1HP, CH2)
- ✅ **천장 팬 제어** (예비, CH3)
- ✅ **LED 조명 제어** (예비, CH4)

### 안전 기능
- ✅ **타임아웃 자동 종료** (관수: 5분, 배수: 5분)
- ✅ **긴급 정지** (MQTT 명령 또는 수동)
- ✅ **동시 작동 방지** (관수/배수 펌프)
- ✅ **최소 ON 시간** (채터링 방지)
- ✅ **쿨다운 타임** (긴급 정지 후 5초)

### 통신 기능
- ✅ **MQTT 명령 수신** (구독)
- ✅ **상태 리포트 전송** (5초 주기)
- ✅ **하트비트 전송** (10초 주기)
- ✅ **WiFi 자동 재연결**

### 모니터링 기능
- ✅ **실시간 상태 피드백**
- ✅ **작동 시간 통계**
- ✅ **작동 횟수 카운터**
- ✅ **LED 상태 표시**

---

## 🔧 하드웨어 구성

### 필수 부품

| 부품명 | 수량 | 사양 | 용도 |
|--------|------|------|------|
| Arduino Uno R4 WiFi | 1개 | MCU | 메인 컨트롤러 |
| 4채널 릴레이 모듈 | 1개 | 5V, 10A | 액추에이터 제어 |
| SSR (40A) | 2개 | Solid State Relay | 고전력 펌프용 |
| LS MC-18b | 1개 | 2HP용 전자접촉기 | 관수 펌프 |
| LS MC-12b | 1개 | 1HP용 전자접촉기 | 배수 펌프 |
| 5V 전원 어댑터 | 1개 | 5V/3A | Arduino 전원 |
| 12V 전원 어댑터 | 1개 | 12V/10A | 릴레이/솔레노이드 |
| 220V 전원 | - | 고전력 | 펌프 전원 |
| Dupont 케이블 | 다수 | M-M | 신호 연결 |

### 액추에이터 사양

#### 관수 펌프 (CH1)
| 항목 | 사양 |
|------|------|
| 전력 | 2HP (1.5kW) |
| 전압 | 220V AC |
| 제어 | SSR + MC-18b |
| 타임아웃 | 5분 (300초) |

#### 배수 펌프 (CH2)
| 항목 | 사양 |
|------|------|
| 전력 | 1HP (0.75kW) |
| 전압 | 220V AC |
| 제어 | SSR + MC-12b |
| 타임아웃 | 5분 (300초) |

#### 천장 팬 (CH3, 예비)
| 항목 | 사양 |
|------|------|
| 전력 | ~200W |
| 전압 | 220V AC |
| 제어 | 릴레이 직접 |
| 타임아웃 | 60분 (1시간) |

#### LED 조명 (CH4, 예비)
| 항목 | 사양 |
|------|------|
| 전력 | ~100W |
| 전압 | DC 12V/24V |
| 제어 | 릴레이 직접 |
| 타임아웃 | 12시간 |

---

## ⚡ 하드웨어 연결

### 배선도 (3단 절연)

```
Arduino Uno R4 WiFi         4채널 릴레이         SSR         MC         펌프
====================        ============       =====       ====       ======

[CH1 - 관수 펌프]
D7        ---------->       IN1
                            NO1   ------>    SSR1  ----> MC-18b ---> 2HP 펌프
                            COM1  ------>     +     (220V)       (220V)
                            GND   <------     GND

[CH2 - 배수 펌프]
D8        ---------->       IN2
                            NO2   ------>    SSR2  ----> MC-12b ---> 1HP 펌프
                            COM2  ------>     +     (220V)       (220V)
                            GND   <------     GND

[CH3 - 천장 팬]
D9        ---------->       IN3
                            NO3   ------>    팬 (220V AC)
                            COM3  ------>    전원

[CH4 - LED 조명]
D10       ---------->       IN4
                            NO4   ------>    LED (12V DC)
                            COM4  ------>    +12V

전원:
5V        ---------->       VCC (릴레이 모듈)
GND       ---------->       GND (릴레이 모듈)
```

### ⚠️ 안전 경고

1. **고전압 주의**: 220V는 치명적일 수 있습니다. 전기 작업은 전문가가 수행하세요.
2. **3단 절연**: Arduino → 릴레이 → SSR → MC → 펌프 (물리적 분리)
3. **접지 필수**: 모든 고전력 장비를 접지하세요.
4. **퓨즈/차단기**: 과전류 보호 장치 설치 필수.
5. **방수**: 릴레이 모듈은 방수 케이스에 설치.

---

## 💻 소프트웨어 설정

### 1. Arduino IDE 설정

#### 필수 라이브러리 설치

Arduino IDE → `도구` → `라이브러리 관리`:

| 라이브러리 | 버전 | 용도 |
|-----------|------|------|
| `WiFiS3` | 최신 | Arduino R4 WiFi 전용 |
| `PubSubClient` | 2.8+ | MQTT 통신 |
| `ArduinoJson` | 6.21+ | JSON 직렬화 |

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

#### 2.3 안전 설정 (선택사항)

```cpp
// 타임아웃 설정
#define IRRIGATION_TIMEOUT 300000    // 5분 (기본값)
#define DRAINAGE_TIMEOUT 300000      // 5분 (기본값)

// 동시 작동 방지
#define ALLOW_SIMULTANEOUS_PUMPS false  // 관수/배수 동시 작동 방지
```

### 3. 펌웨어 업로드

1. Arduino IDE에서 `actuator_node.ino` 파일 열기
2. `도구` → `보드` → `Arduino Uno R4 WiFi` 선택
3. `도구` → `포트` → 연결된 포트 선택
4. `config.h`에서 WiFi/MQTT 설정 확인
5. `업로드` 버튼 클릭

---

## 📡 MQTT 명령 및 상태

### 명령 토픽 (구독)

#### 관수 펌프
```
actuator/irrigation_pump/on      # 관수 펌프 켜기
actuator/irrigation_pump/off     # 관수 펌프 끄기
```

#### 배수 펌프
```
actuator/drainage_pump/on        # 배수 펌프 켜기
actuator/drainage_pump/off       # 배수 펌프 끄기
```

#### 팬 제어
```
actuator/fan/on                  # 팬 켜기
actuator/fan/off                 # 팬 끄기
```

#### LED 제어
```
actuator/led/on                  # LED 켜기
actuator/led/off                 # LED 끄기
```

#### 안전 제어
```
actuator/emergency_stop          # 긴급 정지 (모든 액추에이터 정지)
actuator/reset                   # 긴급 정지 해제
```

### 상태 토픽 (발행)

#### 상태 리포트 (5초 주기)

**Topic**: `actuator/state`

**Payload (JSON)**:
```json
{
  "irrigation_pump": true,
  "drainage_pump": false,
  "fan": false,
  "led": false,
  "emergency_stop": false,
  "total_irrigation_time": 3600,
  "total_drainage_time": 1800,
  "irrigation_count": 12,
  "drainage_count": 8,
  "uptime": 86400000,
  "rssi": -65
}
```

#### 하트비트 (10초 주기)

**Topic**: `actuator/heartbeat`

**Payload (JSON)**:
```json
{
  "status": "alive",
  "emergency_stop": false,
  "active_count": 1,
  "uptime": 86400000,
  "rssi": -65
}
```

#### 상태 메시지 (이벤트)

**Topic**: `actuator/status`

**Payload (JSON)**:
```json
{
  "status": "irrigation_pump_started",
  "timestamp": 1702284000000
}
```

**Status 값**:
- `"initialized"`: 시스템 초기화 완료
- `"online"`: MQTT 연결 성공
- `"irrigation_pump_started"`: 관수 펌프 시작
- `"irrigation_pump_stopped"`: 관수 펌프 정지
- `"drainage_pump_started"`: 배수 펌프 시작
- `"drainage_pump_stopped"`: 배수 펌프 정지
- `"emergency_stop_activated"`: 긴급 정지 활성화
- `"emergency_stop_reset"`: 긴급 정지 해제

---

## 🔍 테스트 및 디버그

### 시리얼 모니터 출력 확인

1. Arduino IDE → `도구` → `시리얼 모니터` 열기
2. 보드레이트: **115200 bps** 설정
3. 출력 예시:

```
========================================
  Wasabi SmartFarm
  Actuator Control Node
========================================
  Version: v1.0.0
  Author: 서준원
  Date: 2024-12-11
========================================
  Actuators:
    - CH1: Irrigation Pump (2HP)
    - CH2: Drainage Pump (1HP)
    - CH3: Ceiling Fan (Reserve)
    - CH4: LED Light (Reserve)
  WiFi SSID: your_wifi_ssid
  MQTT Broker: 192.168.0.100:1883
  Safety Features:
    - Irrigation Timeout: 300 sec
    - Drainage Timeout: 300 sec
    - Emergency Cooldown: 5 sec
    - Simultaneous Pumps: BLOCKED
========================================

[ACTUATOR] Initializing actuator control...
[ACTUATOR] Relay pins configured:
[ACTUATOR]   CH1 (Irrigation): D7
[ACTUATOR]   CH2 (Drainage): D8
[ACTUATOR]   CH3 (Fan): D9
[ACTUATOR]   CH4 (LED): D10
[ACTUATOR] All actuators initialized (OFF state)

[MQTT] Initializing MQTT Handler...
[WiFi] Connecting to: your_wifi_ssid
...........
[WiFi] Connected!
[WiFi] IP Address: 192.168.0.105
[MQTT] Connected to broker!
[MQTT] Subscribed to all command topics

========================================
  Actuator Node Ready!
  Waiting for MQTT commands...
========================================

[MQTT] Message received: actuator/irrigation_pump/on -> {}
[ACTUATOR] Relay D7 set to ON
[ACTUATOR] ✓ Irrigation pump STARTED
[ACTUATOR]   Timeout: 300 seconds
[MQTT] Heartbeat sent

[ACTUATOR] ⚠️  Irrigation pump TIMEOUT!
[ACTUATOR] Relay D7 set to OFF
[ACTUATOR] ✓ Irrigation pump STOPPED
[ACTUATOR]   Total irrigation time: 300 seconds
```

### LED 상태 표시

| LED 패턴 | 의미 |
|---------|------|
| 빠른 깜빡임 (0.2초) | 긴급 정지 상태 |
| 느린 깜빡임 (0.5초) | 액추에이터 작동 중 |
| 매우 느린 깜빡임 (2초) | 대기 중 (정상) |

---

## 🧪 테스트 방법

### 1. MQTT 명령 테스트

터미널에서 mosquitto_pub 사용:

```bash
# 관수 펌프 켜기
mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/on" -m ""

# 관수 펌프 끄기
mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/off" -m ""

# 배수 펌프 켜기
mosquitto_pub -h 192.168.0.100 -t "actuator/drainage_pump/on" -m ""

# 긴급 정지
mosquitto_pub -h 192.168.0.100 -t "actuator/emergency_stop" -m ""

# 긴급 정지 해제
mosquitto_pub -h 192.168.0.100 -t "actuator/reset" -m ""
```

### 2. 상태 모니터링

```bash
# 상태 리포트 구독
mosquitto_sub -h 192.168.0.100 -t "actuator/state" -v

# 하트비트 구독
mosquitto_sub -h 192.168.0.100 -t "actuator/heartbeat" -v

# 모든 액추에이터 토픽 구독
mosquitto_sub -h 192.168.0.100 -t "actuator/#" -v
```

### 3. 안전 기능 테스트

```bash
# 타임아웃 테스트 (5분 후 자동 종료 확인)
mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/on" -m ""
# 5분 대기...

# 동시 작동 방지 테스트
mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/on" -m ""
mosquitto_pub -h 192.168.0.100 -t "actuator/drainage_pump/on" -m ""
# 시리얼 모니터에서 "ERROR: Cannot start drainage pump" 확인

# 긴급 정지 테스트
mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/on" -m ""
mosquitto_pub -h 192.168.0.100 -t "actuator/emergency_stop" -m ""
# 모든 액추에이터 즉시 정지 확인
```

---

## 🛠️ 문제 해결

### 릴레이가 작동하지 않음

**증상**: MQTT 명령을 보내도 릴레이가 동작하지 않음

**해결 방법**:
1. 릴레이 모듈 전원 확인 (5V, GND)
2. 신호선 연결 확인 (D7~D10)
3. 릴레이 활성화 레벨 확인:
   ```cpp
   #define RELAY_ON LOW   // 대부분의 릴레이 모듈
   // #define RELAY_ON HIGH  // 일부 모듈
   ```
4. 릴레이 LED 확인 (켜져야 함)
5. 멀티미터로 NO-COM 단자 확인

### 긴급 정지가 해제되지 않음

**증상**: `actuator/reset` 명령 후에도 긴급 정지 상태 유지

**해결 방법**:
1. 5초 쿨다운 대기 확인
2. 시리얼 모니터에서 "Cannot reset yet" 메시지 확인
3. 쿨다운 시간 조정:
   ```cpp
   #define EMERGENCY_COOLDOWN 5000  // 5초 → 다른 값
   ```

### MQTT 명령이 수신되지 않음

**증상**: mosquitto_pub로 명령을 보내도 반응 없음

**해결 방법**:
1. MQTT 브로커 실행 확인:
   ```bash
   sudo systemctl status mosquitto
   ```
2. Arduino WiFi 연결 확인
3. MQTT 클라이언트 ID 중복 확인
4. 토픽 이름 정확히 확인
5. QoS 레벨 확인 (기본: 0)

### WiFi 연결 실패

**증상**: `[WiFi] ERROR: Connection timeout!`

**해결 방법**:
1. SSID/비밀번호 재확인
2. 2.4GHz WiFi 사용 확인
3. WiFi 신호 강도 확인 (RSSI > -80 dBm)

---

## 📊 성능 및 리소스

| 항목 | 값 |
|------|-----|
| 메모리 사용 (프로그램) | ~32KB / 256KB (12.5%) |
| 메모리 사용 (SRAM) | ~12KB / 32KB (37.5%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 명령 응답 시간 | < 100ms |
| 평균 전력 소비 | ~200mA @ 5V (대기) |
| 최대 전력 소비 | ~300mA @ 5V (모든 릴레이 ON) |

---

## 📁 파일 구조

```
actuator_node/
├── actuator_node.ino          # 메인 펌웨어
├── config.h                    # 설정 파일
├── actuator_control.h          # 액추에이터 제어 헤더
├── actuator_control.cpp        # 액추에이터 제어 구현
├── mqtt_handler.h              # MQTT 핸들러 헤더
├── mqtt_handler.cpp            # MQTT 핸들러 구현
├── README.md                   # 이 파일
└── CHANGELOG.md                # 변경 이력
```

---

## 🔄 업그레이드 가이드

### 릴레이 채널 추가

더 많은 액추에이터 추가 시:
1. 8채널 릴레이 모듈 사용
2. `config.h`에 핀 추가:
   ```cpp
   #define RELAY_CH5_PIN 11
   #define RELAY_CH6_PIN 12
   ```
3. `actuator_control.cpp`에 제어 함수 추가

### PWM 디밍 지원

LED 조명 밝기 조절:
1. PWM 핀 사용 (D3, D5, D6, D9, D10, D11)
2. `analogWrite()` 함수 사용
3. MQTT 명령에 밝기 값 추가

---

## 📞 지원

- **GitHub**: https://github.com/phdsjw/WasabiSmartFarm
- **작성자**: 서준원

---

## 📜 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다.
