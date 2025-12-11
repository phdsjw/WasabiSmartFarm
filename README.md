# 🌱 Wasabi Smart Farm Control System

**와사비(Wasabi) 재배 스마트팜 제어 시스템**

Arduino Uno R4 WiFi + Node-RED 기반 IoT 스마트팜 자동화 시스템

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.1-blue.svg)](https://github.com/phdsjw/WasabiSmartFarm)
[![Status](https://img.shields.io/badge/status-in%20development-orange.svg)](https://github.com/phdsjw/WasabiSmartFarm)

---

## 📖 프로젝트 개요

와사비는 재배 환경에 매우 민감한 작물로, 수온, 수질, 토양 통기성, 광량 등의 조건이 최적화되지 않으면 생육이 불량하거나 병충해에 취약해집니다. 본 프로젝트는 **IoT 센서 기반 실시간 모니터링**과 **자동 제어 시스템**을 통해 와사비 재배 환경을 최적화하는 것을 목표로 합니다.

### ✨ 주요 특징

- 🎯 **실시간 환경 모니터링**: 대기, 토양, 물탱크 센서 데이터 실시간 수집
- 🤖 **자동 제어**: 급수/퇴수, 온도 조절, LED 조명 자동 제어
- 🖥️ **웹 대시보드**: Node-RED 기반 반응형 웹 UI
- 📊 **데이터 저장**: InfluxDB (시계열) + Google Sheets (백업)
- 🔒 **안전 설계**: 비상정지 시스템, 과전류 보호, 3단계 절연
- 📱 **원격 제어**: WiFi 기반 MQTT 통신, 원격 모니터링
- 💰 **비용 효율**: 약 200만원 하드웨어 + 오픈소스 소프트웨어

---

## 🏗️ 시스템 아키텍처

```
┌────────────────────────────────────┐
│     Cloud / Remote Access          │
│  (Google Sheets, Monitoring)       │
└──────────────┬─────────────────────┘
               │ HTTP/HTTPS
┌──────────────┴─────────────────────┐
│   Server Layer (Raspberry Pi)      │
│  ┌──────────────────────────────┐  │
│  │  Node-RED                    │  │
│  │  (Control Logic + Dashboard) │  │
│  └──────┬───────────────┬────────┘  │
│         │               │            │
│  ┌──────┴──────┐  ┌────┴─────────┐  │
│  │ MQTT Broker │  │  InfluxDB    │  │
│  │ (Mosquitto) │  │ (Time-series)│  │
│  └──────┬──────┘  └──────────────┘  │
└─────────┼──────────────────────────┘
          │ MQTT (WiFi)
┌─────────┴──────────────────────────┐
│  Edge Layer (Arduino Uno R4 WiFi)  │
│  - Sensor Data Collection          │
│  - Actuator Control                │
└────────────────────────────────────┘
```

---

## 🛠️ 하드웨어 구성

### 센서
- **대기**: SHT30 (온습도)
- **토양**: SEN0604 (온도, 수분, EC, pH) × 18개
- **물탱크**: DS18B20 (수온), pH/TDS/EC 센서
- **수위**: 초음파/압력 센서 × 18개

### 제어 구동기
- **펌프**: 급수 펌프 (2HP), 퇴수 펌프 (1HP)
- **솔레노이드**: 급수/퇴수 각 18개 (탱크별)
- **환풍기**: 천장 환풍기 (220V)
- **조명**: LED 보광등 (PWM 제어)
- **측창 모터**: 개폐용 4개

### 제어 보드
- **MCU**: Arduino Uno R4 WiFi
- **통신**: RS485 확장보드 (Modbus RTU)
- **릴레이**: 8채널 릴레이 모듈 × 2
- **안전**: SSR, 전자개폐기 (고전력 장비용)

---

## 📋 제어 로직

### 자동 제어 조건

| 제어 항목 | 조건 | 임계값 | 동작 |
|---------|-----|-------|------|
| **관수장비** | 토양습도 | ≤ 95% | 4분/시간 작동 |
| | 토양EC | ≥ 5.0 μS/cm | 4분/시간 작동 |
| | **토양온도** | **≥ 22°C** | **4분/시간 작동** |
| **냉각수 모터** | 대기온도 | ≥ 22°C | 자동 ON |
| **온수 모터** | 대기온도 | ≤ 18°C | 자동 ON |
| **LED 보광등** | 시간 주기 | 2시간마다 | 10분 OFF |
| **퇴수모터** | 수위 | ≥ 80% | 자동 퇴수 |

> **Note**: 관수장비는 3가지 조건 중 **하나라도 만족하면** 작동합니다 (OR 조건).

---

## 💻 소프트웨어 스택

### 펌웨어 (Arduino)
- **언어**: C++ (Arduino Framework)
- **통신**: WiFi (WiFiS3), MQTT (PubSubClient)
- **센서**: Modbus RTU, I2C, Analog, 1-Wire
- **주요 라이브러리**:
  - `ArduinoModbus`, `ArduinoRS485`
  - `OneWire`, `DallasTemperature`
  - `Adafruit_SHT31`
  - `ArduinoJson`

### 서버 (Raspberry Pi / PC)
- **플랫폼**: Raspberry Pi 4 (4GB) / Windows 11 (프로토타입)
- **제어 엔진**: Node-RED (Flow-based Programming)
- **MQTT Broker**: Mosquitto
- **Database**: InfluxDB (시계열), Google Sheets (백업)
- **UI**: Node-RED Dashboard (반응형 웹)

---

## 📂 프로젝트 구조

```
WasabiSmartFarm/
├── docs/
│   └── WASABI_SMARTFARM_PROJECT_PROPOSAL.md  # 📘 프로젝트 기획서
├── arduino/
│   └── wasabi_controller/                     # Arduino 펌웨어 (예정)
│       ├── wasabi_controller.ino
│       ├── config.h
│       ├── mqtt.cpp
│       ├── sensors.cpp
│       └── actuators.cpp
├── nodered/
│   ├── flows.json                             # Node-RED Flow (예정)
│   └── settings.js
├── hardware/
│   ├── schematics/                            # 회로도 (예정)
│   └── wiring_diagram/                        # 배선도 (예정)
└── README.md                                  # 📖 본 문서
```

---

## 🚀 시작하기

### 1. 문서 확인

프로젝트의 전체 구조와 기술 명세는 **[프로젝트 기획서](docs/WASABI_SMARTFARM_PROJECT_PROPOSAL.md)**를 참조하세요.

기획서 내용:
- 시스템 아키텍처 상세
- 하드웨어/센서 명세
- 제어 로직 알고리즘 (코드 예제 포함)
- 소프트웨어 개발 가이드
- 개발 로드맵 (3단계)
- 예산 산정 (하드웨어 품목별)
- 안전 설계 및 보안
- 유지보수 가이드

### 2. 개발 환경 설정 (예정)

#### Arduino 개발 환경
```bash
# Arduino IDE 2.x 설치
# https://www.arduino.cc/en/software

# 필수 라이브러리 설치
# - WiFiS3
# - PubSubClient
# - ArduinoModbus
# - ArduinoRS485
# - OneWire
# - DallasTemperature
# - Adafruit_SHT31
# - ArduinoJson
```

#### 서버 환경 설정 (Raspberry Pi)
```bash
# Node-RED 설치
sudo apt update
sudo apt install -y nodejs npm
sudo npm install -g --unsafe-perm node-red

# Mosquitto (MQTT Broker) 설치
sudo apt install -y mosquitto mosquitto-clients

# InfluxDB 설치
# (기획서의 8.1.1 섹션 참조)

# Node-RED 시작
node-red
# 브라우저에서 http://localhost:1880 접속
```

---

## 📊 개발 로드맵

### Phase 1: 프로토타입 개발 (2개월)
- ✅ 프로젝트 기획서 작성 완료
- ⏳ Arduino 펌웨어 개발
- ⏳ Node-RED Flow 개발
- ⏳ 센서 통신 테스트
- ⏳ 제어 로직 검증

### Phase 2: 실제 환경 배포 (2개월)
- ⏳ Raspberry Pi 환경 구축
- ⏳ 하드웨어 설치 및 배선
- ⏳ 현장 테스트 및 안정화
- ⏳ 데이터베이스 구축
- ⏳ 사용자 교육

### Phase 3: 고도화 (3~6개월)
- ⏳ AI 생육 예측 모델
- ⏳ 모바일 앱 개발
- ⏳ 다중 농장 관리
- ⏳ 이상 탐지 시스템

---

## 💰 예산

| 항목 | 금액 |
|------|------|
| **하드웨어** | 약 432만원 |
| **소프트웨어** | 무료 (오픈소스) |
| **운영 비용** | 월 약 12만원 |

> 상세 예산 내역은 [프로젝트 기획서 섹션 13](docs/WASABI_SMARTFARM_PROJECT_PROPOSAL.md#13-예산-및-리소스)을 참조하세요.

---

## 🤝 기여하기

이 프로젝트는 와사비 재배 자동화를 위한 오픈소스 프로젝트입니다. 기여를 환영합니다!

### 기여 방법
1. 이 저장소를 Fork
2. Feature 브랜치 생성 (`git checkout -b feature/AmazingFeature`)
3. 변경사항 커밋 (`git commit -m 'Add some AmazingFeature'`)
4. 브랜치에 Push (`git push origin feature/AmazingFeature`)
5. Pull Request 생성

---

## 📝 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 `LICENSE` 파일을 참조하세요.

---

## 📧 연락처

**프로젝트 관리자**: Wasabi SmartFarm Team

**GitHub**: [https://github.com/phdsjw/WasabiSmartFarm](https://github.com/phdsjw/WasabiSmartFarm)

---

## 🙏 참고 자료

- [Arduino Official Documentation](https://docs.arduino.cc/)
- [Node-RED Documentation](https://nodered.org/docs/)
- [InfluxDB Documentation](https://docs.influxdata.com/)
- [Mosquitto MQTT Broker](https://mosquitto.org/documentation/)
- [와사비 재배 가이드](https://www.rda.go.kr/) (농촌진흥청)

---

## 📈 프로젝트 진행 상황

### v1.1 (2024-12-11)
- ✅ 프로젝트 기획서 v1.1 작성 완료
- ✅ 제어 로직 설계 (관수장비 토양온도 조건 추가)
- ✅ 시스템 아키텍처 설계
- ✅ 하드웨어 명세 작성
- ✅ 예산 산정 완료

### 다음 작업
- ⏳ Arduino 펌웨어 개발 시작
- ⏳ Node-RED Flow 프로토타입 개발
- ⏳ 센서 통신 테스트

---

**Made with 💚 for Wasabi Cultivation**
