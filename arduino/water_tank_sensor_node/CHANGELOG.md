# 변경 이력 (Changelog)

## v1.0.0 (2024-12-11) - 초기 릴리스

### ✨ 주요 기능

#### 센서 기능
- ✅ **DS18B20 수온 센서** (1-Wire 디지털)
  - 측정 범위: -55~125°C
  - 정확도: ±0.5°C
  - 12비트 해상도 (0.0625°C)
  - 방수 스테인리스 프로브

- ✅ **SEN0161 pH 센서** (아날로그)
  - 측정 범위: pH 0~14
  - 정확도: ±0.1 pH
  - 자동 보정 지원
  - 경고 알림 (정상 범위: 5.5~8.5)

- ✅ **SEN0244 TDS 센서** (아날로그)
  - 측정 범위: 0~1000 ppm
  - 정확도: ±10%
  - 온도 자동 보상
  - 경고 알림 (정상 범위: 300~600 ppm)

- ✅ **SEN0451 Pro EC 센서** (아날로그)
  - 측정 범위: 0~20 mS/cm
  - 정확도: ±5%
  - 온도 자동 보상
  - 경고 알림 (정상 범위: 0.8~2.0 mS/cm)

#### 통신 기능
- ✅ WiFi 연결 및 자동 재연결
- ✅ MQTT 프로토콜 기반 데이터 전송
- ✅ JSON 포맷 데이터 직렬화
- ✅ 10초 주기 센서 데이터 전송
- ✅ 60초 주기 하트비트 전송
- ✅ 상태 메시지 전송

#### 시스템 기능
- ✅ 다중 샘플링 및 평균 필터링 (노이즈 제거)
- ✅ 온도 보상 자동 적용 (TDS, EC)
- ✅ 센서 오류 감지 및 자동 재초기화
- ✅ LED 상태 표시
- ✅ 시리얼 디버그 출력
- ✅ WiFi 신호 강도(RSSI) 모니터링

### 📊 측정 데이터

| 센서 | 측정 항목 | 범위 | 정확도 | 온도 보상 |
|------|----------|------|--------|----------|
| DS18B20 | 수온 | -55~125°C | ±0.5°C | - |
| SEN0161 | pH | 0~14 | ±0.1 pH | - |
| SEN0244 | TDS | 0~1000 ppm | ±10% | ✅ |
| SEN0451 Pro | EC | 0~20 mS/cm | ±5% | ✅ |

### 🔧 하드웨어

- **MCU**: Arduino Uno R4 WiFi (1개)
- **센서**:
  - DS18B20 (수온, 1-Wire)
  - SEN0161 (pH, 아날로그)
  - SEN0244 (TDS, 아날로그)
  - SEN0451 Pro (EC, 아날로그)
- **통신**: WiFi (2.4GHz)
- **프로토콜**: MQTT over WiFi

### 📡 MQTT 토픽

| 토픽 | 설명 | 전송 주기 |
|------|------|----------|
| `sensor/water_tank/data` | 센서 데이터 | 10초 |
| `sensor/water_tank/heartbeat` | 하트비트 | 60초 |
| `sensor/water_tank/status` | 상태 메시지 | 이벤트 |

### 📦 데이터 포맷 (JSON)

#### 센서 데이터
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

#### 하트비트
```json
{
  "status": "alive",
  "uptime": 3600000,
  "rssi": -65
}
```

### 🛠️ 설정 파일 (config.h)

#### 필수 설정
- `WIFI_SSID`: WiFi SSID
- `WIFI_PASSWORD`: WiFi 비밀번호
- `MQTT_SERVER`: MQTT 브로커 IP
- `MQTT_PORT`: MQTT 브로커 포트 (기본: 1883)

#### 센서 보정 (선택)
- `PH_CALIBRATION_OFFSET`: pH 보정 오프셋
- `PH_CALIBRATION_SLOPE`: pH 보정 기울기
- `TDS_K_VALUE`: TDS 보정 계수
- `EC_K_VALUE`: EC 보정 계수

#### 필터링 설정
- `ANALOG_SAMPLE_COUNT`: 아날로그 샘플링 횟수 (기본: 10)
- `ANALOG_SAMPLE_DELAY`: 샘플 간 딜레이 (기본: 10ms)

### 📈 성능

| 항목 | 값 |
|------|-----|
| 프로그램 메모리 | ~28KB / 256KB (11%) |
| SRAM 사용량 | ~10KB / 32KB (31%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 센서 측정 시간 | ~1.5초 |
| 평균 전력 소비 | ~180mA @ 5V |

---

## 📌 참고 자료

### DS18B20 수온 센서
- **제조사**: Maxim Integrated
- **데이터시트**: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
- **통신**: 1-Wire (4.7kΩ 풀업 저항 필요)
- **특징**:
  - 방수 프로브
  - 긴 케이블 지원 (최대 100m)
  - 버스에 여러 센서 연결 가능

### SEN0161 pH 센서
- **제조사**: DFRobot
- **Wiki**: https://wiki.dfrobot.com/PH_meter_SKU__SEN0161_
- **통신**: 아날로그 (0~5V)
- **보정**: 2점 보정 (pH 4.0, 7.0 또는 10.0)
- **유의사항**:
  - 전극을 항상 습윤 상태 유지
  - 정기적인 보정 필요 (1개월)
  - 수명: 6개월~1년

### SEN0244 TDS 센서
- **제조사**: DFRobot
- **Wiki**: https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
- **통신**: 아날로그 (0~2.3V)
- **특징**:
  - 온도 보상 알고리즘 내장
  - 수경 재배 최적화
  - 저전력 설계

### SEN0451 Pro EC 센서
- **제조사**: DFRobot
- **통신**: 아날로그 (0~5V)
- **특징**:
  - 고정밀 측정
  - 온도 보상 알고리즘
  - K=1 전극 사용

### MQTT
- **프로토콜 버전**: MQTT 3.1.1
- **QoS**: 0 (At most once)
- **Retain**: false
- **클라이언트 ID**: `WasabiWaterTank`

---

## 🔄 업그레이드 계획

### v1.1.0 (예정)
- [ ] ORP (산화환원전위) 센서 추가
- [ ] 용존산소(DO) 센서 추가
- [ ] 센서 자동 보정 기능
- [ ] SD 카드 로컬 로깅
- [ ] 알람 임계값 자동 조정

### v1.2.0 (예정)
- [ ] MQTT QoS 1/2 지원
- [ ] OTA (Over-The-Air) 펌웨어 업데이트
- [ ] 웹 서버 기반 설정 UI
- [ ] MQTT TLS/SSL 암호화
- [ ] 센서 수명 모니터링

### v2.0.0 (예정)
- [ ] Modbus RTU 센서 지원
- [ ] 다중 수조 지원
- [ ] AI 기반 이상 감지
- [ ] 클라우드 데이터 백업

---

## 🐛 알려진 이슈

### v1.0.0
- 없음 (현재까지 발견된 버그 없음)

### 제한 사항
- TDS/EC 센서는 25°C 기준 온도 보상 (±5°C 이내 정확)
- pH 센서는 정기적인 보정 필요 (1개월)
- DS18B20은 긴 케이블 사용 시 노이즈 가능성

---

## 🔧 마이그레이션 가이드

### v1.0.0 초기 설치

#### 1. 하드웨어 준비
- Arduino Uno R4 WiFi × 1
- DS18B20 수온 센서 × 1
- SEN0161 pH 센서 × 1
- SEN0244 TDS 센서 × 1
- SEN0451 Pro EC 센서 × 1
- 4.7kΩ 저항 × 1 (1-Wire 풀업)
- Dupont 케이블

#### 2. 배선
```
Arduino         DS18B20
========        =======
D4      ---->   Data + 4.7kΩ 풀업 (D4 ~ 5V)
5V      ---->   VCC
GND     ---->   GND

Arduino         SEN0161 (pH)
========        ============
A0      <----   Signal
5V      ---->   VCC
GND     ---->   GND

Arduino         SEN0244 (TDS)
========        =============
A1      <----   Signal
5V      ---->   VCC
GND     ---->   GND

Arduino         SEN0451 Pro (EC)
========        ================
A2      <----   Signal
5V      ---->   VCC
GND     ---->   GND
```

#### 3. 펌웨어 설정
1. `config.h` 파일 열기
2. WiFi SSID/비밀번호 입력
3. MQTT 브로커 IP 입력

#### 4. 라이브러리 설치
Arduino IDE → 라이브러리 관리:
- `WiFiS3` (최신)
- `PubSubClient` (2.8+)
- `ArduinoJson` (6.21+)
- `OneWire` (2.3+)
- `DallasTemperature` (3.9+)

#### 5. 업로드
- Arduino IDE에서 업로드 버튼 클릭
- 시리얼 모니터(115200 bps)에서 로그 확인

#### 6. 센서 보정
- pH 센서: pH 4.0, 7.0 표준 용액 사용
- TDS/EC 센서: 표준 용액으로 K 값 보정

#### 7. 테스트
- MQTT 클라이언트로 토픽 구독:
  ```bash
  mosquitto_sub -h 192.168.0.100 -t "sensor/water_tank/data" -v
  ```

---

## 📞 문의 및 지원

### 문제 보고
- **GitHub Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues
- 버그 리포트 시 다음 정보 포함:
  - 펌웨어 버전
  - 하드웨어 구성
  - 시리얼 모니터 로그
  - 센서 보정 상태

### 기여
- Pull Request 환영합니다
- 코드 스타일: Arduino 표준 스타일 가이드

---

**문서 정보**  
**작성자**: 서준원  
**최종 수정**: 2024-12-11  
**버전**: v1.0.0
