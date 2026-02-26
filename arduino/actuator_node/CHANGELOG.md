# 변경 이력 (Changelog)

## v1.0.0 (2024-12-11) - 초기 릴리스

### ✨ 주요 기능

#### 액추에이터 제어
- ✅ **4채널 릴레이 제어**
  - CH1: 관수 펌프 (2HP)
  - CH2: 배수 펌프 (1HP)
  - CH3: 천장 팬 (예비)
  - CH4: LED 조명 (예비)

- ✅ **MQTT 명령 수신**
  - 관수/배수 펌프 ON/OFF
  - 팬/LED ON/OFF
  - 긴급 정지
  - 리셋

#### 안전 기능
- ✅ **타임아웃 자동 종료**
  - 관수 펌프: 5분 (300초)
  - 배수 펌프: 5분 (300초)
  - 팬: 60분 (1시간)
  - LED: 12시간

- ✅ **동시 작동 방지** (인터록)
  - 관수/배수 펌프 동시 작동 차단
  - 설정 가능 (`ALLOW_SIMULTANEOUS_PUMPS`)

- ✅ **긴급 정지**
  - 모든 액추에이터 즉시 정지
  - 5초 쿨다운 타임
  - 리셋 명령으로 해제

- ✅ **최소 ON 시간** (1초)
  - 채터링 방지
  - 장비 보호

#### 통신 기능
- ✅ **WiFi 연결 및 자동 재연결**
- ✅ **MQTT 프로토콜 기반 통신**
- ✅ **명령 토픽 구독** (10개)
- ✅ **상태 리포트 전송** (5초 주기)
- ✅ **하트비트 전송** (10초 주기)
- ✅ **상태 메시지 전송** (이벤트)

#### 모니터링 기능
- ✅ **실시간 상태 피드백**
  - 각 액추에이터 ON/OFF 상태
  - 긴급 정지 상태
  - 작동 중인 액추에이터 수

- ✅ **작동 통계**
  - 총 관수 시간
  - 총 배수 시간
  - 관수 횟수
  - 배수 횟수

- ✅ **LED 상태 표시**
  - 긴급 정지: 빠른 깜빡임 (200ms)
  - 작동 중: 느린 깜빡임 (500ms)
  - 대기 중: 매우 느린 깜빡임 (2초)

### 🔧 하드웨어

- **MCU**: Arduino Uno R4 WiFi (1개)
- **릴레이**: 4채널 릴레이 모듈 (5V, 10A)
- **고전력 제어**: 
  - SSR × 2 (40A)
  - LS MC-18b (2HP용)
  - LS MC-12b (1HP용)
- **통신**: WiFi (2.4GHz)
- **프로토콜**: MQTT over WiFi

### 📡 MQTT 토픽

#### 명령 토픽 (구독)
| 토픽 | 설명 |
|------|------|
| `actuator/irrigation_pump/on` | 관수 펌프 켜기 |
| `actuator/irrigation_pump/off` | 관수 펌프 끄기 |
| `actuator/drainage_pump/on` | 배수 펌프 켜기 |
| `actuator/drainage_pump/off` | 배수 펌프 끄기 |
| `actuator/fan/on` | 팬 켜기 |
| `actuator/fan/off` | 팬 끄기 |
| `actuator/led/on` | LED 켜기 |
| `actuator/led/off` | LED 끄기 |
| `actuator/emergency_stop` | 긴급 정지 |
| `actuator/reset` | 긴급 정지 해제 |

#### 상태 토픽 (발행)
| 토픽 | 설명 | 주기 |
|------|------|------|
| `actuator/state` | 상태 리포트 | 5초 |
| `actuator/heartbeat` | 하트비트 | 10초 |
| `actuator/status` | 상태 메시지 | 이벤트 |

### 📦 데이터 포맷 (JSON)

#### 상태 리포트
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

#### 하트비트
```json
{
  "status": "alive",
  "emergency_stop": false,
  "active_count": 1,
  "uptime": 86400000,
  "rssi": -65
}
```

### 🛠️ 설정 파일 (config.h)

#### 필수 설정
- `WIFI_SSID`: WiFi SSID
- `WIFI_PASSWORD`: WiFi 비밀번호
- `MQTT_SERVER`: MQTT 브로커 IP
- `MQTT_PORT`: MQTT 브로커 포트 (기본: 1883)

#### 릴레이 핀
- `RELAY_CH1_PIN`: 관수 펌프 (기본: D7)
- `RELAY_CH2_PIN`: 배수 펌프 (기본: D8)
- `RELAY_CH3_PIN`: 팬 (기본: D9)
- `RELAY_CH4_PIN`: LED (기본: D10)

#### 안전 설정
- `IRRIGATION_TIMEOUT`: 관수 타임아웃 (기본: 5분)
- `DRAINAGE_TIMEOUT`: 배수 타임아웃 (기본: 5분)
- `ALLOW_SIMULTANEOUS_PUMPS`: 동시 작동 허용 (기본: false)
- `MIN_ON_TIME`: 최소 ON 시간 (기본: 1초)
- `EMERGENCY_COOLDOWN`: 긴급 정지 쿨다운 (기본: 5초)

### 📈 성능

| 항목 | 값 |
|------|-----|
| 프로그램 메모리 | ~32KB / 256KB (12.5%) |
| SRAM 사용량 | ~12KB / 32KB (37.5%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 명령 응답 시간 | < 100ms |
| 평균 전력 소비 | ~200mA @ 5V (대기) |
| 최대 전력 소비 | ~300mA @ 5V (모든 릴레이 ON) |

---

## 📌 참고 자료

### 릴레이 모듈
- **타입**: 4채널 5V 릴레이
- **접점 용량**: 10A @ 250V AC / 10A @ 30V DC
- **제어 전압**: 5V DC
- **활성화 레벨**: LOW (대부분 모듈)
- **특징**: 
  - 광학 격리 (옵토커플러)
  - 프리휠 다이오드
  - LED 상태 표시

### SSR (Solid State Relay)
- **용량**: 40A
- **제어 전압**: 3~32V DC
- **부하 전압**: 24~480V AC
- **특징**:
  - 무음 스위칭
  - 긴 수명
  - 빠른 응답
  - 바운스 없음

### 전자접촉기 (Magnetic Contactor)
- **LS MC-18b**: 2HP (1.5kW) 펌프용
- **LS MC-12b**: 1HP (0.75kW) 펌프용
- **제어 전압**: 220V AC (코일)
- **특징**:
  - 고전력 부하 제어
  - 보조 접점
  - 과부하 보호
  - 인터록 가능

### 안전 규격
- **절연**: 3단 절연 (Arduino → 릴레이 → SSR → MC)
- **접지**: 모든 고전력 장비 접지 필수
- **퓨즈**: 과전류 보호 장치
- **MCCB**: 누전차단기 (RCD)
- **IP 등급**: 제어반 IP54 이상 권장

### MQTT
- **프로토콜 버전**: MQTT 3.1.1
- **QoS**: 0 (At most once)
- **Retain**: false
- **클라이언트 ID**: `WasabiActuator`
- **Keep Alive**: 60초

---

## 🔄 업그레이드 계획

### v1.1.0 (예정)
- [ ] 8채널 릴레이 지원 확장
- [ ] PWM 디밍 지원 (LED 조명)
- [ ] 전류 센서 통합 (전력 모니터링)
- [ ] 온도 센서 통합 (과열 보호)
- [ ] 수동 버튼 제어 추가

### v1.2.0 (예정)
- [ ] MQTT QoS 1/2 지원
- [ ] OTA (Over-The-Air) 펌웨어 업데이트
- [ ] 웹 서버 기반 설정 UI
- [ ] SD 카드 로그 저장
- [ ] 알람 기능 (Telegram, Email)

### v2.0.0 (예정)
- [ ] Modbus RTU 슬레이브 모드
- [ ] 다중 액추에이터 노드 지원
- [ ] AI 기반 예측 제어
- [ ] 클라우드 백업

---

## 🐛 알려진 이슈

### v1.0.0
- 없음 (현재까지 발견된 버그 없음)

### 제한 사항
- 최대 4개 액추에이터 제어 (4채널 릴레이)
- 동시 작동 방지는 관수/배수 펌프에만 적용
- MQTT QoS 0만 지원 (메시지 유실 가능성)
- 오프라인 시 명령 대기열 없음

---

## 🔧 마이그레이션 가이드

### v1.0.0 초기 설치

#### 1. 하드웨어 준비
- Arduino Uno R4 WiFi × 1
- 4채널 릴레이 모듈 × 1
- SSR (40A) × 2
- LS MC-18b × 1
- LS MC-12b × 1
- Dupont 케이블

#### 2. 배선 (⚠️ 전기 전문가 권장)
```
Arduino → 릴레이:
  D7 → IN1 (관수 펌프)
  D8 → IN2 (배수 펌프)
  D9 → IN3 (팬)
  D10 → IN4 (LED)
  5V → VCC
  GND → GND

릴레이 → SSR → MC → 펌프:
  NO1 → SSR1 → MC-18b → 2HP 펌프
  NO2 → SSR2 → MC-12b → 1HP 펌프
```

#### 3. 펌웨어 설정
1. `config.h` 파일 열기
2. WiFi SSID/비밀번호 입력
3. MQTT 브로커 IP 입력
4. 안전 설정 확인

#### 4. 라이브러리 설치
Arduino IDE → 라이브러리 관리:
- `WiFiS3` (최신)
- `PubSubClient` (2.8+)
- `ArduinoJson` (6.21+)

#### 5. 업로드
- Arduino IDE에서 업로드 버튼 클릭
- 시리얼 모니터(115200 bps)에서 로그 확인

#### 6. 테스트
- MQTT 명령 테스트:
  ```bash
  mosquitto_pub -h 192.168.0.100 -t "actuator/irrigation_pump/on" -m ""
  ```
- 릴레이 동작 확인 (LED, 클릭 소리)
- 타임아웃 테스트 (5분 후 자동 종료)
- 긴급 정지 테스트

---

## 📞 문의 및 지원

### 문제 보고
- **GitHub Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues
- 버그 리포트 시 다음 정보 포함:
  - 펌웨어 버전
  - 하드웨어 구성
  - 시리얼 모니터 로그
  - MQTT 명령 및 응답

### 기여
- Pull Request 환영합니다
- 코드 스타일: Arduino 표준 스타일 가이드

---

**문서 정보**  
**작성자**: 서준원  
**최종 수정**: 2024-12-11  
**버전**: v1.0.0
