# 센서 노드 재구성 변경사항 요약

**버전**: v2.0.0  
**변경일**: 2025-12-27  
**작성자**: 서준원

---

## 변경 개요

센서 노드 물리적 할당을 최적화하여 Arduino 보드 사용을 효율화했습니다.

---

## 주요 변경사항

### AS-IS (변경 전)
- 대기 센서 노드: 1개 (Zone 01, SHT30만)
- 토양 센서 노드: 18개 (Tank 01~18, SEN0604만)
- 물탱크 센서 노드: 1개 (DS18B20 + pH/TDS/EC)
- 수위 센서: 시스템 컨트롤러에 통합
- **총 Arduino Uno R4 WiFi: 20개**

### TO-BE (변경 후)
- **대기+토양 통합 노드: 3개** (노드 13, 14, 15)
  - SHT30 + SEN0604 통합
  - Zone 13, 14, 15
  - Tank 13, 14, 15
- **토양 전용 노드: 15개** (노드 01~12, 16~18)
  - SEN0604만
- **물탱크 노드: 1개** (노드 19)
  - DS18B20 + pH/TDS/EC
- **수위 센서 노드: 1개** (노드 20, 신규)
  - HC-SR04
  - **Wemos D1 R1 (ESP8266)** 사용
- **액추에이터 노드: 1개** (노드 21)
- **총 보드: 20개 (Uno R4 WiFi 19개 + Wemos D1 R1 1개)**

---

## 노드 번호 할당표

| 노드 번호 | 노드 유형 | MCU | 센서/역할 | Zone/Tank ID |
|----------|----------|-----|----------|-------------|
| **01~12** | 토양 전용 | Uno R4 WiFi | SEN0604 | Tank 01~12 |
| **13** | 대기+토양 통합 | Uno R4 WiFi | SHT30 + SEN0604 | Zone 13, Tank 13 |
| **14** | 대기+토양 통합 | Uno R4 WiFi | SHT30 + SEN0604 | Zone 14, Tank 14 |
| **15** | 대기+토양 통합 | Uno R4 WiFi | SHT30 + SEN0604 | Zone 15, Tank 15 |
| **16~18** | 토양 전용 | Uno R4 WiFi | SEN0604 | Tank 16~18 |
| **19** | 물탱크 | Uno R4 WiFi | DS18B20 + pH/TDS/EC | - |
| **20** | 수위 센서 | Wemos D1 R1 | HC-SR04 × 1 | 물탱크 |
| **21** | 액추에이터 | Uno R4 WiFi | 릴레이 제어 | - |
| **총 21개** | - | **19 × R4 + 1 × ESP8266** | - | - |

---

## 신규 생성 파일

### 1. 대기+토양 통합 노드 (노드 13, 14, 15)
```
arduino/air_soil_combined_node/
├── air_soil_combined_node.ino    # 메인 코드
├── config.h                       # 설정 (Zone/Tank ID 변경 가능)
├── sht30_sensor.h                 # SHT30 센서 헤더
├── sht30_sensor.cpp               # SHT30 센서 구현
├── sen0604_modbus.h               # SEN0604 센서 헤더
├── sen0604_modbus.cpp             # SEN0604 센서 구현
├── mqtt_handler.h                 # MQTT 핸들러 헤더 (통합)
├── mqtt_handler.cpp               # MQTT 핸들러 구현 (통합)
└── README.md                      # 설치 가이드
```

**특징:**
- SHT30 (I2C, A4/A5) + SEN0604 (RS485, D0/D1/D2) 통합
- 대기 데이터: `sensor/air/zone{13-15}/data`
- 토양 데이터: `sensor/soil/tank{13-15}/data`
- 하트비트: `sensor/combined/zone{13-15}/heartbeat`

### 2. 수위 센서 노드 (노드 20)
```
arduino/water_level_sensor_node/
├── water_level_sensor_node.ino   # 메인 코드 (ESP8266)
├── config.h                       # 설정
├── hcsr04_sensor.h                # HC-SR04 센서 헤더
├── hcsr04_sensor.cpp              # HC-SR04 센서 구현
├── mqtt_handler.h                 # MQTT 핸들러 헤더 (ESP8266)
├── mqtt_handler.cpp               # MQTT 핸들러 구현 (ESP8266)
└── README.md                      # 설치 가이드
```

**특징:**
- ESP8266 기반 (Wemos D1 R1)
- HC-SR04 초음파 센서 (Trig: D1, Echo: D2)
- 다중 샘플 평균 필터링 (5샘플)
- 데이터: `sensor/water_level/data`
- 하트비트: `sensor/water_level/heartbeat`
- 3초 주기 측정

---

## MQTT 토픽 변경

### 제거된 토픽
- `sensor/air/zone01/data`
- `sensor/air/zone01/heartbeat`

### 추가된 토픽

#### 대기 센서 (Zone 13, 14, 15)
- `sensor/air/zone13/data`
- `sensor/air/zone14/data`
- `sensor/air/zone15/data`

#### 토양 센서 (Tank 13, 14, 15)
- `sensor/soil/tank13/data`
- `sensor/soil/tank14/data`
- `sensor/soil/tank15/data`

#### 통합 노드 하트비트
- `sensor/combined/zone13/heartbeat`
- `sensor/combined/zone14/heartbeat`
- `sensor/combined/zone15/heartbeat`

#### 수위 센서
- `sensor/water_level/data`
- `sensor/water_level/heartbeat`

---

## Node-RED 수정 필요사항

### 1. 하트비트 모니터링 (21개 노드)
- 기존: Zone 01, Tank 01~18, 물탱크, 액추에이터
- 변경: Zone 13/14/15, Tank 01~12/16~18, 물탱크, 수위센서, 액추에이터

### 2. 센서 데이터 수신
- Zone 13, 14, 15 대기 데이터 추가
- Tank 13, 14, 15 토양 데이터 추가
- 수위 센서 데이터 추가

### 3. Dashboard UI
- 대기 센서 그룹: Zone 01 → Zone 13, 14, 15
- 토양 센서: Tank 13, 14, 15 추가
- 수위 센서 그룹 신규 생성

**상세 가이드**: `nodered/NODE_RED_UPDATE_GUIDE.md` 참조

---

## 설치 순서

### 1. 대기+토양 통합 노드 (노드 13, 14, 15)
1. `arduino/air_soil_combined_node/config.h`에서 Zone/Tank ID 설정
   - 노드 13: ZONE_ID="13", TANK_ID="13"
   - 노드 14: ZONE_ID="14", TANK_ID="14"
   - 노드 15: ZONE_ID="15", TANK_ID="15"
2. WiFi/MQTT 설정
3. Arduino Uno R4 WiFi에 업로드

### 2. 토양 전용 노드 (노드 01~12, 16~18)
1. 기존 `arduino/soil_sensor_node/` 코드 사용
2. `config.h`에서 TANK_ID만 변경
3. Arduino Uno R4 WiFi에 업로드

### 3. 수위 센서 노드 (노드 20)
1. `arduino/water_level_sensor_node/config.h`에서 설정
   - TANK_HEIGHT_CM: 실제 물탱크 높이 (cm)
   - SENSOR_OFFSET_CM: 센서 오프셋 (cm)
2. WiFi/MQTT 설정
3. ESP8266 보드 패키지 설치
4. Wemos D1 R1에 업로드

### 4. Node-RED 수정
1. `flows_wasabi_03.json` 백업
2. 하트비트 모니터링 수정 (21개 노드)
3. 센서 데이터 수신 노드 추가
4. Dashboard UI 수정
5. 배포 및 테스트

---

## 기술적 개선사항

### 1. 대기+토양 통합 노드
- **장점**: 보드 절약, 케이블 간소화
- **기술**: I2C (SHT30) + RS485 (SEN0604) 동시 사용
- **센서 독립성**: 각 센서 개별 MQTT 토픽으로 분리 전송

### 2. 수위 센서 노드
- **ESP8266 사용 이유**: 비용 절감, WiFi 내장, 충분한 성능
- **다중 샘플 평균**: 5샘플 평균으로 노이즈 감소
- **자동 수위 계산**: 거리 → 수위(%) 자동 변환

---

## 예상 이점

1. **비용 절감**: Arduino Uno R4 WiFi 1개 절약 (약 4만원)
2. **설치 간소화**: 통합 노드로 케이블 감소
3. **유지보수 개선**: 노드 수 감소로 관리 용이
4. **확장성**: 수위 센서 독립 노드로 향후 추가 용이

---

## 다음 단계

1. Node-RED Flow 수정 (2.5시간 예상)
2. 전체 시스템 통합 테스트
3. 7일 연속 운영 테스트
4. 문서화 완료

---

## 관련 문서

- `arduino/air_soil_combined_node/README.md` - 대기+토양 통합 노드 가이드
- `arduino/water_level_sensor_node/README.md` - 수위 센서 노드 가이드
- `nodered/NODE_RED_UPDATE_GUIDE.md` - Node-RED 수정 가이드
- `SENSOR_SPECIFICATION.md` - 센서 명세서 (업데이트 필요)

---

## 작성자

서준원

## 변경 이력

- **v2.0.0** (2025-12-27): 센서 노드 재구성
  - 대기+토양 통합 노드 추가 (노드 13, 14, 15)
  - 수위 센서 노드 추가 (노드 20, ESP8266)
  - 노드 번호 재할당
- **v1.0.0** (2025-12-23): 최초 센서 구성
