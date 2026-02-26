# 변경 이력 (Changelog)

## v1.0.0 (2024-12-11) - 초기 릴리스

### ✨ 주요 기능

#### 센서 기능
- ✅ SHT30 온습도 센서 지원 (I2C 통신)
- ✅ 대기 온도 측정 (-40~125°C, 정확도 ±0.2°C)
- ✅ 대기 습도 측정 (0~100%, 정확도 ±2%RH)
- ✅ CRC8 체크섬 검증 (데이터 무결성 보장)
- ✅ 센서 소프트 리셋 기능
- ✅ 센서 오류 감지 및 자동 재초기화

#### 통신 기능
- ✅ WiFi 연결 및 자동 재연결
- ✅ MQTT 프로토콜 기반 데이터 전송
- ✅ JSON 포맷 데이터 직렬화
- ✅ 10초 주기 센서 데이터 전송
- ✅ 60초 주기 하트비트 전송
- ✅ 상태 메시지 전송 (초기화, 온라인 등)

#### 시스템 기능
- ✅ Zone ID 기반 센서 노드 식별 (01~03)
- ✅ LED 상태 표시 (정상/오류)
- ✅ 시리얼 디버그 출력
- ✅ WiFi 신호 강도(RSSI) 모니터링
- ✅ 메모리 사용량 모니터링

### 📊 측정 데이터

| 센서 | 측정 항목 | 범위 | 정확도 | 해상도 |
|------|----------|------|--------|--------|
| SHT30 | 대기 온도 | -40~125°C | ±0.2°C | 0.01°C |
| SHT30 | 대기 습도 | 0~100% | ±2%RH | 0.01% |

### 🔧 하드웨어

- **MCU**: Arduino Uno R4 WiFi (3개)
- **센서**: SHT30 온습도 센서 (I2C, 3개)
- **통신**: WiFi (2.4GHz)
- **프로토콜**: MQTT over WiFi

### 📡 MQTT 토픽

| 토픽 | 설명 | 전송 주기 |
|------|------|----------|
| `sensor/air/zone{01~03}/data` | 센서 데이터 | 10초 |
| `sensor/air/zone{01~03}/heartbeat` | 하트비트 | 60초 |
| `sensor/air/zone{01~03}/status` | 상태 메시지 | 이벤트 |

### 📦 데이터 포맷 (JSON)

#### 센서 데이터
```json
{
  "zone_id": "01",
  "air_temp": 22.5,
  "air_humidity": 65.3,
  "timestamp": 1702284000000,
  "rssi": -65
}
```

#### 하트비트
```json
{
  "zone_id": "01",
  "status": "alive",
  "uptime": 3600000,
  "rssi": -65,
  "free_memory": 12345
}
```

### 🛠️ 설정 파일 (config.h)

#### 필수 설정
- `ZONE_ID`: 센서 노드 고유 ID ("01", "02", "03")
- `WIFI_SSID`: WiFi SSID
- `WIFI_PASSWORD`: WiFi 비밀번호
- `MQTT_SERVER`: MQTT 브로커 IP
- `MQTT_PORT`: MQTT 브로커 포트 (기본: 1883)

#### 선택 설정
- `SHT30_I2C_ADDRESS`: SHT30 I2C 주소 (기본: 0x44)
- `SENSOR_READ_INTERVAL`: 센서 읽기 주기 (기본: 10000ms)
- `HEARTBEAT_INTERVAL`: 하트비트 주기 (기본: 60000ms)
- `DEBUG_MODE`: 디버그 모드 (기본: true)

### 📈 성능

| 항목 | 값 |
|------|-----|
| 프로그램 메모리 | ~25KB / 256KB (10%) |
| SRAM 사용량 | ~8KB / 32KB (25%) |
| WiFi 연결 시간 | ~5초 |
| MQTT 연결 시간 | ~1초 |
| 센서 측정 시간 | ~20ms |
| 평균 전력 소비 | ~150mA @ 5V |

---

## 📌 참고 자료

### SHT30 센서
- **제조사**: Sensirion
- **데이터시트**: https://www.sensirion.com/en/environmental-sensors/humidity-sensors/digital-humidity-sensors-for-various-applications/
- **통신**: I2C (주소: 0x44 또는 0x45)
- **특징**:
  - CRC8 체크섬 지원
  - 내부 히터 (드리프트 방지)
  - 낮은 전력 소비
  - 긴 수명 (> 10년)

### I2C 통신
- **Arduino Uno R4 WiFi I2C 핀**:
  - SDA: A4
  - SCL: A5
- **풀업 저항**: 4.7kΩ (선택사항, 긴 케이블 사용 시 권장)
- **버스 속도**: 100kHz (표준 모드)

### MQTT
- **프로토콜 버전**: MQTT 3.1.1
- **QoS**: 0 (At most once)
- **Retain**: false
- **클라이언트 ID**: `WasabiAir_Zone{01~03}`

---

## 🔄 업그레이드 계획

### v1.1.0 (예정)
- [ ] BME280 센서 지원 추가 (온습도 + 기압)
- [ ] MQTT QoS 1/2 지원
- [ ] OTA (Over-The-Air) 펌웨어 업데이트
- [ ] SD 카드 로컬 로깅
- [ ] 알람 임계값 설정 기능

### v1.2.0 (예정)
- [ ] 다중 센서 지원 (1개 노드에 여러 SHT30)
- [ ] 센서 보정 기능
- [ ] 웹 서버 기반 설정 UI
- [ ] MQTT TLS/SSL 암호화
- [ ] 저전력 모드 (배터리 운영)

---

## 🐛 알려진 이슈

### v1.0.0
- 없음 (현재까지 발견된 버그 없음)

---

## 🔧 마이그레이션 가이드

### v1.0.0 초기 설치

#### 1. 하드웨어 준비
- Arduino Uno R4 WiFi × 3
- SHT30 센서 × 3
- Dupont 케이블 (F-F) × 12
- 5V 전원 어댑터 × 3

#### 2. 배선
```
Arduino         SHT30
========        =====
5V      ---->   VCC
GND     ---->   GND
A4      <--->   SDA
A5      <--->   SCL
```

#### 3. 펌웨어 설정
1. `config.h` 파일 열기
2. WiFi SSID/비밀번호 입력
3. MQTT 브로커 IP 입력
4. **각 노드마다 `ZONE_ID` 변경**:
   - 노드 1: `#define ZONE_ID "01"`
   - 노드 2: `#define ZONE_ID "02"`
   - 노드 3: `#define ZONE_ID "03"`

#### 4. 라이브러리 설치
Arduino IDE → 라이브러리 관리:
- `WiFiS3` (최신)
- `PubSubClient` (2.8+)
- `ArduinoJson` (6.21+)

#### 5. 업로드
- Arduino IDE에서 업로드 버튼 클릭
- 시리얼 모니터(115200 bps)에서 로그 확인

#### 6. 테스트
- MQTT 클라이언트(mosquitto_sub)로 토픽 구독:
  ```bash
  mosquitto_sub -h 192.168.0.100 -t "sensor/air/+/data" -v
  ```

---

## 📞 문의 및 지원

### 문제 보고
- **GitHub Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues
- 버그 리포트 시 다음 정보 포함:
  - 펌웨어 버전
  - 하드웨어 구성
  - 시리얼 모니터 로그
  - 증상 및 재현 방법

### 기여
- Pull Request 환영합니다
- 코드 스타일: Arduino 표준 스타일 가이드

---

**문서 정보**  
**작성자**: 서준원  
**최종 수정**: 2024-12-11  
**버전**: v1.0.0
