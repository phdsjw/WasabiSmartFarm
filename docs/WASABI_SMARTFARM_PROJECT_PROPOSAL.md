# 와사비(Wasabi) 재배 스마트팜 제어 시스템 개발 프로젝트 기획서

## 📋 목차
1. [프로젝트 개요](#1-프로젝트-개요)
2. [시스템 아키텍처](#2-시스템-아키텍처)
3. [하드웨어 구성](#3-하드웨어-구성)
4. [센서 시스템 명세](#4-센서-시스템-명세)
5. [제어 로직 및 알고리즘](#5-제어-로직-및-알고리즘)
6. [소프트웨어 아키텍처](#6-소프트웨어-아키텍처)
7. [통신 프로토콜](#7-통신-프로토콜)
8. [데이터베이스 설계](#8-데이터베이스-설계)
9. [사용자 인터페이스](#9-사용자-인터페이스)
10. [개발 로드맵](#10-개발-로드맵)
11. [안전 및 보안](#11-안전-및-보안)
12. [확장성 및 유지보수](#12-확장성-및-유지보수)
13. [예산 및 리소스](#13-예산-및-리소스)

---

## 1. 프로젝트 개요

### 1.1 배경 및 목적
와사비는 일본 고유의 향신채소로, 재배 환경에 매우 민감한 작물입니다. 특히 수온, 수질, 토양 통기성, 광량 등의 조건이 최적화되지 않으면 생육이 불량하거나 병충해에 취약해집니다.

**프로젝트 목적:**
- 실시간 환경 모니터링 및 데이터 기반 의사결정 지원
- 급수/퇴수 시스템의 자동화를 통한 노동력 절감
- 환경 조건 최적화를 통한 와사비 생산성 및 품질 향상
- 원격 모니터링 및 제어를 통한 관리 편의성 증대

### 1.2 프로젝트 범위
- **Phase 1 (초기)**: Windows 11 PC 기반 프로토타입 개발 및 검증
- **Phase 2 (운영)**: Raspberry Pi 기반 실제 생산 환경 배포
- **Phase 3 (확장)**: AI 기반 생육 예측 및 최적화 기능 추가

### 1.3 기대 효과
1. **생산성 향상**: 최적 환경 유지를 통한 수확량 20~30% 증가 예상
2. **노동력 절감**: 자동화를 통한 현장 관리 시간 50% 이상 감소
3. **품질 개선**: 일관된 환경 관리로 와사비 품질 균일화
4. **데이터 축적**: 생육 데이터 분석을 통한 재배 노하우 체계화

---

## 2. 시스템 아키텍처

### 2.1 전체 시스템 구조

```
┌─────────────────────────────────────────────────────────────┐
│                     Cloud/Remote Access                     │
│  (Google Sheets, External Monitoring)                      │
└────────────────────────┬───────────────────────────────────┘
                         │ HTTP/HTTPS
┌────────────────────────┴───────────────────────────────────┐
│              Server Layer (PC / Raspberry Pi)              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Node-RED (Control Logic + Dashboard)                │  │
│  │  - Flow-based Programming                            │  │
│  │  - Web UI Dashboard                                  │  │
│  │  - Business Logic Processing                         │  │
│  └────────────┬──────────────────────┬──────────────────┘  │
│               │                      │                      │
│  ┌────────────▼──────────┐  ┌───────▼──────────────────┐  │
│  │  Mosquitto MQTT Broker│  │   Database Layer          │  │
│  │  (Message Queue)      │  │  - InfluxDB (Time-series) │  │
│  │                       │  │  - Google Sheets (Backup) │  │
│  └────────────┬──────────┘  └───────────────────────────┘  │
└───────────────┼────────────────────────────────────────────┘
                │ MQTT (WiFi)
┌───────────────┴────────────────────────────────────────────┐
│              Edge Layer (Arduino Uno R4 WiFi)              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Sensor Data Collection                              │  │
│  │  - Modbus RTU (토양 센서)                            │  │
│  │  - I2C (온습도 센서)                                 │  │
│  │  - Analog (pH, TDS, EC 센서)                         │  │
│  │  - 1-Wire (수온 센서)                                │  │
│  └──────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Actuator Control                                    │  │
│  │  - Relay Control (펌프, 솔레노이드)                 │  │
│  │  - PWM Control (LED 조명)                           │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 계층별 역할

#### 2.2.1 Edge Layer (Arduino Uno R4 WiFi)
- **센서 데이터 수집**: 주기적(1~5초) 센서 polling
- **1차 데이터 가공**: 센서값 필터링, 이상치 제거
- **MQTT 통신**: JSON 형식으로 센서 데이터 전송
- **구동기 제어**: MQTT Subscribe를 통한 실시간 제어 수신

#### 2.2.2 Server Layer (Node-RED + Database)
- **비즈니스 로직**: 제어 알고리즘 실행
- **데이터 저장**: 시계열 데이터베이스 저장
- **사용자 인터페이스**: 웹 대시보드 제공
- **알림 기능**: 이상 상황 알림 (Telegram, Email 등)

#### 2.2.3 Cloud Layer (Optional)
- **데이터 백업**: 일일 리포트 Google Sheets 저장
- **원격 접근**: VPN/DDNS를 통한 외부 접속

---

## 3. 하드웨어 구성

### 3.1 센서 하드웨어

| 측정 대상 | 센서 모델명 | 통신 방식 | 수량 | 설치 위치 | 비고 |
|---------|----------|----------|-----|---------|------|
| **대기 온습도** | SHT30 (P4422-3) | I2C | 1 | 재배상 중앙 상단 | 고정밀 측정 |
| **토양 온도** | SEN0604 (4-in-1) | RS485 (Modbus) | 18 | 각 재배상 토양 | 탱크별 개별 설치 |
| **토양 수분** | SEN0604 (4-in-1) | RS485 (Modbus) | 18 | 각 재배상 토양 | 토양 온도와 동일 센서 |
| **토양 EC** | SEN0604 (4-in-1) | RS485 (Modbus) | 18 | 각 재배상 토양 | 토양 온도와 동일 센서 |
| **토양 pH** | SEN0604 (4-in-1) | RS485 (Modbus) | 18 | 각 재배상 토양 | 토양 온도와 동일 센서 |
| **수온** | DS18B20 (DFR0198) | Digital (1-Wire) | 1 | 물탱크 내부 | 방수 타입 |
| **물탱크 pH** | SEN0161 | Analog | 1 | 물탱크 내부 | 주기적 보정 필요 |
| **물탱크 TDS** | SEN0244 | Analog | 1 | 물탱크 내부 | 전도도 측정 |
| **물탱크 EC (정밀)** | SEN0451 (Pro) | Analog | 1 | 물탱크 내부 | 고정밀 측정 |
| **수위 센서** | 초음파/압력 센서 | Analog/Digital | 18 | 각 재배상 탱크 | 급수/퇴수 제어용 |

### 3.2 제어 구동기 하드웨어

| 구동기 명칭 | 전원 규격 | 제어 방식 | 수량 | 제어 하드웨어 | 안전 조치 |
|-----------|---------|----------|-----|-------------|----------|
| **급수 펌프** | 220V, 2HP | ON/OFF | 1 | 전자개폐기(MC) + SSR | 과전류 차단기 |
| **퇴수 펌프** | 220V, 1HP | ON/OFF | 1 | 전자개폐기(MC) + SSR | 과전류 차단기 |
| **보조 펌프** | 220V, 0.5HP | ON/OFF | 1 | 릴레이 모듈 (10A) | 수압 조절용 |
| **급수 솔레노이드** | 12V DC | ON/OFF | 18 | 릴레이 모듈 (각 탱크) | 개별 제어 |
| **퇴수 솔레노이드** | 12V DC | ON/OFF | 18 | 릴레이 모듈 (각 탱크) | 개별 제어 |
| **천장 환풍기** | 220V AC | ON/OFF | 1 | 릴레이 모듈 (10A) | 온습도 연동 |
| **측창 모터 (개폐)** | 220V AC | 정/역회전 | 4 | 4채널 릴레이 + 타이머 | 개폐 시간 제어 |
| **LED 보광등** | DC 12V/24V | PWM Dimming | 1 | MOSFET 드라이버 | 광주기 제어 |

### 3.3 제어 보드 및 통신 모듈

| 구분 | 모델명 | 용도 | 수량 |
|-----|-------|------|-----|
| **메인 MCU** | Arduino Uno R4 WiFi | 센서 데이터 수집 및 제어 신호 송수신 | 1 |
| **RS485 확장보드** | DFR0259 | Modbus RTU 통신 | 1 |
| **릴레이 모듈** | 8채널 릴레이 모듈 (5V) | 저전력 구동기 제어 | 2 |
| **SSR (Solid State Relay)** | 40A SSR | 고전력 펌프 제어 | 2 |
| **전자개폐기 (MC)** | LS MC-18b | 2HP 펌프 제어 | 1 |
| **전자개폐기 (MC)** | LS MC-12b | 1HP 펌프 제어 | 1 |
| **전원 공급장치** | 12V/5A, 5V/3A | Arduino 및 센서/솔레노이드 전원 | 2 |

### 3.4 하드웨어 안전 설계

#### 3.4.1 전기 안전
1. **고전력 구동기 절연**
   - Arduino → SSR → 전자개폐기 → 펌프 (3단계 절연)
   - 제어 회로(5V/12V)와 전원 회로(220V) 물리적 분리

2. **과전류 보호**
   - 각 220V 라인에 누전차단기 및 과전류 차단기 설치
   - 퓨즈 및 서지 보호 장치 설치

3. **접지**
   - 모든 금속 케이스 및 220V 장비 접지
   - 센서 케이블 실드 접지

#### 3.4.2 물리적 안전
- 방수 등급: 센서 IP67 이상, 제어 박스 IP54 이상
- 환기: 제어 박스 내부 팬 설치 (열 방출)
- 배선: 전선관 배선, 케이블 타이 정리

---

## 4. 센서 시스템 명세

### 4.1 센서 핀맵 (Arduino Uno R4 WiFi)

```
Arduino Uno R4 WiFi Pin Assignment
=========================================
Digital Pins:
  D0, D1       : UART (USB Serial) - 디버깅용
  D2, D3       : Hardware Serial1 (RS485 - Modbus)
  D4           : 1-Wire (DS18B20 - 수온 센서)
  D5~D12       : 릴레이 제어 (8채널)
  D13          : 상태 LED

Analog Pins:
  A0           : pH 센서 (SEN0161)
  A1           : TDS 센서 (SEN0244)
  A2           : EC 센서 (SEN0451)
  A3           : 예비
  A4 (SDA)     : I2C (SHT30 온습도 센서)
  A5 (SCL)     : I2C (SHT30 온습도 센서)

I2C Address:
  0x44         : SHT30 (온습도 센서)

Modbus RTU (RS485):
  Slave ID 1~6 : 토양 센서 (SEN0604) - 각 탱크별
```

### 4.2 센서 데이터 사양

| 센서 | 측정 범위 | 정확도 | 분해능 | 샘플링 주기 |
|-----|----------|--------|--------|------------|
| SHT30 (온습도) | -40~125°C, 0~100%RH | ±0.2°C, ±2%RH | 0.01°C, 0.1%RH | 5초 |
| SEN0604 (토양 온도) | -40~80°C | ±0.5°C | 0.1°C | 10초 |
| SEN0604 (토양 수분) | 0~100% | ±3% | 0.1% | 10초 |
| SEN0604 (토양 EC) | 0~20 mS/cm | ±5% | 0.01 mS/cm | 10초 |
| SEN0604 (토양 pH) | 3~9 pH | ±0.3 pH | 0.01 pH | 10초 |
| DS18B20 (수온) | -55~125°C | ±0.5°C | 0.0625°C | 5초 |
| SEN0161 (물탱크 pH) | 0~14 pH | ±0.1 pH | 0.01 pH | 30초 |
| SEN0244 (TDS) | 0~1000 ppm | ±10% | 1 ppm | 30초 |
| SEN0451 (EC Pro) | 0~20 mS/cm | ±5% | 0.01 mS/cm | 30초 |
| 수위 센서 | 0~100% | ±2% | 1% | 3초 |

### 4.3 센서 보정 (Calibration)

#### 4.3.1 아날로그 센서 보정
- **pH 센서**: 표준 완충액(pH 4.0, 7.0, 10.0) 사용 2점/3점 보정
- **TDS 센서**: 표준 용액(1413 μS/cm) 사용 보정
- **EC 센서**: 표준 용액(12.88 mS/cm) 사용 보정
- **보정 주기**: 월 1회 또는 측정값 이상 시

#### 4.3.2 디지털 센서
- SHT30, DS18B20: 공장 보정값 사용 (추가 보정 불필요)
- SEN0604 (Modbus): 공장 보정값 사용

---

## 5. 제어 로직 및 알고리즘

### 5.1 제어 흐름도 상세 분석

업로드된 제어 흐름도를 바탕으로 한 상세 제어 로직:

```
┌─────────────────────────────────────────────────────────────┐
│     생장상 환경 측정 (온도, 습도, 광도, 수위, EC)            │
└──────────────────┬──────────────────────────────────────────┘
                   │
        ┌──────────┴──────────┬────────────────────────┐
        │                     │                        │
┌───────▼─────────┐  ┌────────▼────────┐  ┌───────────▼──────┐
│ 관수장비 작동    │  │ 생장상 내부     │  │ LED 보조광원     │
│ (1hr당 4분)     │  │ 냉각수/온수     │  │ (2시간마다       │
│                 │  │ 모터 작동       │  │  10분 OFF)       │
│ 조건1: 토양습도 │  │                 │  │                  │
│        95% 이하 │  │ 조건1: 대기온도 │  │                  │
│ 조건2: 토양EC   │  │        22°C이상 │  │                  │
│    5.0μs/cm이상│  │ 조건2: 대기온도 │  │                  │
│ 조건3: 토양온도 │  │        18°C이하 │  │                  │
│        22°C이상 │  │                 │  │                  │
└────────┬────────┘  └────────┬────────┘  └──────────────────┘
         │                    │
         └────────────────────┴──────────────┐
                                              │
                                   ┌──────────▼─────────┐
                                   │ 퇴수모터 작동       │
                                   │                    │
                                   │ 조건: 수위 센서    │
                                   │       일정 수위 이상│
                                   │                    │
                                   │ 퇴수 후 필터링 및  │
                                   │ UV살균 (별도 제어) │
                                   └────────────────────┘
```

### 5.2 자동 제어 알고리즘 (Node-RED 기반)

#### 5.2.1 급수 제어 알고리즘

```javascript
// 급수 시작 조건
if (운전모드 === 'auto' && 비상정지 === false) {
    // 1. 모든 탱크 수위 확인
    let needWatering = false;
    for (let tank = 1; tank <= 6; tank++) {
        if (수위[tank] < 67%) {  // 목표 수위 75% (여유 8%)
            needWatering = true;
            break;
        }
    }
    
    // 2. 급수 시작
    if (needWatering) {
        // 급수 펌프 ON
        MQTT.publish('watering/watering_pump/on', 'ON');
        
        // 모든 탱크 솔레노이드 ON
        for (let tank = 1; tank <= 6; tank++) {
            let tankNum = ('0' + tank).slice(-2);
            MQTT.publish(`watering/tank${tankNum}/watering_sol_${tankNum}/on`, 'ON');
        }
        
        // 상태 저장
        flow.set('phase', 'filling');
        flow.set('watering_start_time', Date.now());
    }
}

// 급수 종료 조건 (개별 탱크)
setInterval(() => {
    if (flow.get('phase') === 'filling') {
        for (let tank = 1; tank <= 18; tank++) {
            let tankNum = ('0' + tank).slice(-2);
            
            // 목표 수위 도달 시 해당 탱크 솔레노이드만 OFF
            if (수위[tank] >= 75% && !closedSolenoids.includes(tankNum)) {
                MQTT.publish(`watering/tank${tankNum}/watering_sol_${tankNum}/off`, 'OFF');
                closedSolenoids.push(tankNum);
                node.warn(`Tank ${tankNum} 급수 완료 (수위: ${수위[tank]}%)`);
            }
        }
        
        // 모든 탱크 완료 시 펌프 OFF
        if (closedSolenoids.length >= 18) {
            MQTT.publish('watering/watering_pump/off', 'OFF');
            node.warn('전체 급수 완료. 30초 후 퇴수 시작');
            
            // 30초 대기 후 퇴수 시작
            setTimeout(() => {
                startDraining();
            }, 30000);
        }
    }
}, 3000);  // 3초마다 체크
```

#### 5.2.2 퇴수 제어 알고리즘

```javascript
// 퇴수 시작 로직
function startDraining() {
    if (flow.get('emergency')) {
        node.warn('비상정지 상태 - 퇴수 차단');
        return;
    }
    
    // 퇴수 펌프 및 솔레노이드 ON
    MQTT.publish('draining/draining_pump/on', 'ON');
    MQTT.publish('draining/extra_pump/on', 'ON');
    
    for (let tank = 1; tank <= 6; tank++) {
        let tankNum = ('0' + tank).slice(-2);
        MQTT.publish(`draining/tank${tankNum}/draining_sol_${tankNum}/on`, 'ON');
    }
    
    // 상태 저장
    flow.set('phase', 'draining');
    flow.set('draining_start_time', Date.now());
}

// 퇴수 종료 조건 (개별 탱크)
setInterval(() => {
    if (flow.get('phase') === 'draining') {
        let closedSolenoids = flow.get('draining_solenoids_closed') || [];
        
        for (let tank = 1; tank <= 18; tank++) {
            let tankNum = ('0' + tank).slice(-2);
            
            // 목표 수위 도달 시 해당 탱크 솔레노이드만 OFF
            if (수위[tank] <= 33% && !closedSolenoids.includes(tankNum)) {
                MQTT.publish(`draining/tank${tankNum}/draining_sol_${tankNum}/off`, 'OFF');
                closedSolenoids.push(tankNum);
                node.warn(`Tank ${tankNum} 퇴수 완료 (수위: ${수위[tank]}%)`);
            }
        }
        
        flow.set('draining_solenoids_closed', closedSolenoids);
        
        // 모든 탱크 완료 시 펌프 OFF
        if (closedSolenoids.length >= 18) {
            MQTT.publish('draining/draining_pump/off', 'OFF');
            MQTT.publish('draining/extra_pump/off', 'OFF');
            node.warn('전체 퇴수 완료. 30초 후 다음 급수 시작');
            
            // 30초 대기 후 다음 급수 시작 (루프)
            setTimeout(() => {
                // 상태 초기화
                flow.set('sensor_map', {});
                flow.set('missed_tanks', []);
                flow.set('watering_solenoids_closed', []);
                flow.set('draining_solenoids_closed', []);
                
                // 다음 사이클 시작
                startWatering({ loop: true });
            }, 30000);
        }
    }
}, 3000);  // 3초마다 체크
```

#### 5.2.3 환풍기 제어 알고리즘

```javascript
// 환풍기 제어 (온도 기반)
setInterval(() => {
    let currentTemp = 대기온도;
    let currentHumidity = 대기습도;
    
    // 환풍기 ON 조건
    if (currentTemp >= 22 || currentTemp <= 18) {
        if (!flow.get('fan_running')) {
            MQTT.publish('ventilation/fan/on', 'ON');
            flow.set('fan_running', true);
            node.warn(`환풍기 ON (온도: ${currentTemp}°C)`);
        }
    } else {
        // 환풍기 OFF 조건 (적정 온도 범위 내)
        if (flow.get('fan_running')) {
            MQTT.publish('ventilation/fan/off', 'OFF');
            flow.set('fan_running', false);
            node.warn(`환풍기 OFF (온도: ${currentTemp}°C)`);
        }
    }
}, 5000);  // 5초마다 체크
```

#### 5.2.4 관수장비 제어 알고리즘 (환경 기반)

```javascript
// 관수장비 제어 (토양 습도, EC, 온도 기반)
setInterval(() => {
    for (let tank = 1; tank <= 18; tank++) {
        let 토양습도 = sensor_data[`tank${tank}`].soil_moisture;
        let 토양EC = sensor_data[`tank${tank}`].soil_ec;
        let 토양온도 = sensor_data[`tank${tank}`].soil_temp;
        
        // 관수 필요 조건 (OR 조건: 하나라도 만족하면 실행)
        let needIrrigation = false;
        let reason = [];
        
        // 조건 1: 토양 습도 95% 이하
        if (토양습도 <= 95) {
            needIrrigation = true;
            reason.push(`토양 습도 낮음 (${토양습도}%)`);
        }
        
        // 조건 2: 토양 EC 5.0 μS/cm 이상
        if (토양EC >= 5.0) {
            needIrrigation = true;
            reason.push(`토양 EC 높음 (${토양EC} μS/cm)`);
        }
        
        // 조건 3: 토양 온도 22°C 이상 (NEW)
        if (토양온도 >= 22) {
            needIrrigation = true;
            reason.push(`토양 온도 높음 (${토양온도}°C)`);
        }
        
        // 관수 실행 (1시간당 4분)
        if (needIrrigation) {
            let tankNum = ('0' + tank).slice(-2);
            
            // 현재 시간의 분 단위 확인
            let now = new Date();
            let minute = now.getMinutes();
            
            // 매 시간 0~4분 사이에만 관수
            if (minute >= 0 && minute < 4) {
                MQTT.publish(`irrigation/tank${tankNum}/on`, 'ON');
                node.warn(`Tank ${tankNum} 관수 실행: ${reason.join(', ')}`);
            } else {
                MQTT.publish(`irrigation/tank${tankNum}/off`, 'OFF');
            }
        }
        
        // 관수 후 수위 확인 → 퇴수 로직 연동
        if (needIrrigation && sensor_data[`tank${tank}`].water_level >= 80) {
            // 수위가 80% 이상이면 퇴수 필요
            node.warn(`Tank ${tank} 수위 높음 (${sensor_data[`tank${tank}`].water_level}%) - 퇴수 예약`);
            flow.set(`tank${tank}_needs_draining`, true);
        }
    }
}, 60000);  // 1분마다 체크
```

#### 5.2.5 LED 보광등 제어 알고리즘

```javascript
// LED 보광등 제어 (2시간마다 10분 OFF)
let ledSchedule = {
    onDuration: 110 * 60 * 1000,   // 110분 (1시간 50분)
    offDuration: 10 * 60 * 1000    // 10분
};

function ledControl() {
    // LED ON
    MQTT.publish('lighting/led/on', 'ON');
    node.warn('LED 보광등 ON (110분 동안)');
    
    setTimeout(() => {
        // LED OFF
        MQTT.publish('lighting/led/off', 'OFF');
        node.warn('LED 보광등 OFF (10분 동안)');
        
        setTimeout(() => {
            ledControl();  // 다시 ON
        }, ledSchedule.offDuration);
    }, ledSchedule.onDuration);
}

// 시스템 시작 시 LED 제어 시작
ledControl();
```

#### 5.2.6 냉각수/온수 모터 제어 알고리즘

```javascript
// 냉각수/온수 모터 제어 (대기 온도 기반)
setInterval(() => {
    let 대기온도 = sensor_data.air_temp;
    
    // 냉각수 모터 제어 (온도 22°C 이상)
    if (대기온도 >= 22) {
        if (!flow.get('cooling_motor_running')) {
            MQTT.publish('climate/cooling_motor/on', 'ON');
            flow.set('cooling_motor_running', true);
            node.warn(`냉각수 모터 ON (온도: ${대기온도}°C)`);
        }
        // 온수 모터는 OFF
        if (flow.get('heating_motor_running')) {
            MQTT.publish('climate/heating_motor/off', 'OFF');
            flow.set('heating_motor_running', false);
        }
    }
    // 온수 모터 제어 (온도 18°C 이하)
    else if (대기온도 <= 18) {
        if (!flow.get('heating_motor_running')) {
            MQTT.publish('climate/heating_motor/on', 'ON');
            flow.set('heating_motor_running', true);
            node.warn(`온수 모터 ON (온도: ${대기온도}°C)`);
        }
        // 냉각수 모터는 OFF
        if (flow.get('cooling_motor_running')) {
            MQTT.publish('climate/cooling_motor/off', 'OFF');
            flow.set('cooling_motor_running', false);
        }
    }
    // 적정 온도 범위 (18~22°C) - 모든 모터 OFF
    else {
        if (flow.get('cooling_motor_running')) {
            MQTT.publish('climate/cooling_motor/off', 'OFF');
            flow.set('cooling_motor_running', false);
            node.warn(`냉각수 모터 OFF (온도 정상: ${대기온도}°C)`);
        }
        if (flow.get('heating_motor_running')) {
            MQTT.publish('climate/heating_motor/off', 'OFF');
            flow.set('heating_motor_running', false);
            node.warn(`온수 모터 OFF (온도 정상: ${대기온도}°C)`);
        }
    }
}, 5000);  // 5초마다 체크
```

### 5.3 비상정지 (Emergency Stop) 로직

```javascript
// 비상정지 활성화
function emergencyStop() {
    flow.set('emergency', true);
    
    // 모든 구동기 즉시 정지
    let stopCommands = [
        'watering/watering_pump/off',
        'draining/draining_pump/off',
        'draining/extra_pump/off',
        'ventilation/fan/off'
    ];
    
    // 모든 탱크 솔레노이드 정지
    for (let tank = 1; tank <= 18; tank++) {
        let tankNum = ('0' + tank).slice(-2);
        stopCommands.push(`watering/tank${tankNum}/watering_sol_${tankNum}/off`);
        stopCommands.push(`draining/tank${tankNum}/draining_sol_${tankNum}/off`);
    }
    
    // MQTT 명령 전송
    stopCommands.forEach(topic => {
        MQTT.publish(topic, 'OFF');
    });
    
    // 비상정지 신호 브로드캐스트
    MQTT.publish('emergency/stop', 'ON');
    
    // 상태 초기화
    flow.set('watering_manual_active', 0);
    flow.set('draining_manual_active', 0);
    
    node.warn('🚨 비상정지 활성화: 모든 구동기 정지');
}

// 비상정지 해제
function emergencyRelease() {
    flow.set('emergency', false);
    
    // 비상정지 해제 신호 전송
    MQTT.publish('emergency/stop', 'OFF');
    
    // 상태 초기화
    flow.set('watering_manual_active', 0);
    flow.set('draining_manual_active', 0);
    flow.set('missed_tanks', []);
    flow.set('sensor_map', {});
    
    node.warn('✅ 비상정지 해제: 시스템 정상');
}
```

### 5.4 수동 제어 로직

```javascript
// 수동 제어 신호 처리
function manualControl(msg) {
    // 비상정지 상태 확인
    if (flow.get('emergency')) {
        node.warn('비상정지 상태 - 수동 제어 차단');
        return null;
    }
    
    // 운전 모드 확인 (manual 모드일 때만 동작)
    if (flow.get('op_mode') !== 'manual') {
        node.warn('수동 모드가 아닙니다. 제어 불가');
        return null;
    }
    
    // 급수/퇴수 상호 배제 로직
    let wateringCount = flow.get('watering_manual_active') || 0;
    let drainingCount = flow.get('draining_manual_active') || 0;
    
    let isWateringCmd = msg.topic.includes('watering');
    let isDrainingCmd = msg.topic.includes('draining');
    let isTurningOn = msg.payload === 'ON';
    let isTurningOff = msg.payload === 'OFF';
    
    // 상호 배제: 급수 중에는 퇴수 불가, 퇴수 중에는 급수 불가
    if (isTurningOn) {
        if (isWateringCmd && drainingCount > 0) {
            node.warn(`[차단] 퇴수 작업(${drainingCount}개) 진행 중 - 급수 차단`);
            return null;
        }
        if (isDrainingCmd && wateringCount > 0) {
            node.warn(`[차단] 급수 작업(${wateringCount}개) 진행 중 - 퇴수 차단`);
            return null;
        }
    }
    
    // 카운터 업데이트
    if (isWateringCmd) {
        if (isTurningOn) wateringCount++;
        else if (isTurningOff) wateringCount--;
    } else if (isDrainingCmd) {
        if (isTurningOn) drainingCount++;
        else if (isTurningOff) drainingCount--;
    }
    
    // 카운터 보정 (음수 방지)
    if (wateringCount < 0) wateringCount = 0;
    if (drainingCount < 0) drainingCount = 0;
    
    // 카운터 저장
    flow.set('watering_manual_active', wateringCount);
    flow.set('draining_manual_active', drainingCount);
    
    // MQTT 명령 전송
    return { topic: msg.topic, payload: msg.payload };
}
```

---

## 6. 소프트웨어 아키텍처

### 6.1 펌웨어 (Arduino Uno R4 WiFi)

#### 6.1.1 개발 환경
- **IDE**: Arduino IDE 2.x
- **언어**: C++ (Arduino Framework)
- **보드 패키지**: Arduino UNO R4 WiFi Board Package

#### 6.1.2 주요 라이브러리

| 라이브러리 | 용도 | 버전 |
|----------|------|------|
| `WiFiS3` | WiFi 연결 (R4 전용) | 최신 |
| `PubSubClient` | MQTT 통신 | 2.8+ |
| `ArduinoModbus` | Modbus RTU 프로토콜 | 1.0+ |
| `ArduinoRS485` | RS485 통신 | 1.0+ |
| `OneWire` | 1-Wire 프로토콜 (DS18B20) | 2.3+ |
| `DallasTemperature` | DS18B20 온도 센서 | 3.9+ |
| `Adafruit_SHT31` | SHT30 온습도 센서 (I2C) | 2.2+ |
| `ArduinoJson` | JSON 직렬화/역직렬화 | 6.21+ |

#### 6.1.3 펌웨어 구조

```cpp
// 전역 변수 및 객체
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
SHT31 sht31;
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature dallas(&oneWire);

// 센서 데이터 구조체
struct SensorData {
    float air_temp;
    float air_humidity;
    float water_temp;
    float water_ph;
    float water_tds;
    float water_ec;
    float soil_temp[6];
    float soil_moisture[6];
    float soil_ec[6];
    float soil_ph[6];
    int water_level[6];
};

void setup() {
    // 1. 시리얼 통신 초기화
    Serial.begin(115200);
    Serial1.begin(9600);  // RS485 (Modbus)
    
    // 2. WiFi 연결
    connectWiFi();
    
    // 3. MQTT 연결
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    // 4. 센서 초기화
    sht31.begin(0x44);
    dallas.begin();
    
    // 5. 릴레이 핀 초기화
    for (int i = 5; i <= 12; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);  // 모든 릴레이 OFF
    }
}

void loop() {
    // 1. WiFi/MQTT 연결 확인
    if (!wifiClient.connected()) {
        connectWiFi();
    }
    if (!mqttClient.connected()) {
        connectMQTT();
    }
    mqttClient.loop();
    
    // 2. 센서 데이터 수집 (주기적)
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
        SensorData data = readAllSensors();
        publishSensorData(data);
        lastSensorRead = millis();
    }
    
    // 3. 제어 명령 수신 (MQTT Callback 처리)
    // mqttCallback() 함수에서 처리됨
}

// 센서 데이터 수집
SensorData readAllSensors() {
    SensorData data;
    
    // 대기 온습도 (I2C - SHT30)
    data.air_temp = sht31.readTemperature();
    data.air_humidity = sht31.readHumidity();
    
    // 수온 (1-Wire - DS18B20)
    dallas.requestTemperatures();
    data.water_temp = dallas.getTempCByIndex(0);
    
    // 물탱크 pH, TDS, EC (Analog)
    data.water_ph = readPH(A0);
    data.water_tds = readTDS(A1);
    data.water_ec = readEC(A2);
    
    // 토양 센서 (Modbus RTU - RS485)
    for (int tank = 0; tank < 6; tank++) {
        ModbusRTUClient.begin(9600);
        
        // Slave ID = tank + 1
        int slaveId = tank + 1;
        
        // 레지스터 읽기 (예시: 0x00부터 4개 레지스터)
        if (ModbusRTUClient.requestFrom(slaveId, HOLDING_REGISTERS, 0x00, 4)) {
            data.soil_temp[tank] = ModbusRTUClient.read() / 10.0;
            data.soil_moisture[tank] = ModbusRTUClient.read() / 10.0;
            data.soil_ec[tank] = ModbusRTUClient.read() / 100.0;
            data.soil_ph[tank] = ModbusRTUClient.read() / 10.0;
        }
        
        delay(100);  // Modbus 통신 간격
    }
    
    // 수위 센서 (Analog 또는 Digital)
    for (int tank = 0; tank < 6; tank++) {
        data.water_level[tank] = readWaterLevel(tank);
    }
    
    return data;
}

// MQTT로 센서 데이터 전송
void publishSensorData(SensorData data) {
    // JSON 형식으로 변환
    StaticJsonDocument<1024> doc;
    doc["air_temp"] = data.air_temp;
    doc["air_humidity"] = data.air_humidity;
    doc["water_temp"] = data.water_temp;
    doc["water_ph"] = data.water_ph;
    doc["water_tds"] = data.water_tds;
    doc["water_ec"] = data.water_ec;
    
    // 토양 센서 데이터
    JsonArray soil_temps = doc.createNestedArray("soil_temp");
    JsonArray soil_moistures = doc.createNestedArray("soil_moisture");
    JsonArray soil_ecs = doc.createNestedArray("soil_ec");
    JsonArray soil_phs = doc.createNestedArray("soil_ph");
    
    for (int i = 0; i < 6; i++) {
        soil_temps.add(data.soil_temp[i]);
        soil_moistures.add(data.soil_moisture[i]);
        soil_ecs.add(data.soil_ec[i]);
        soil_phs.add(data.soil_ph[i]);
    }
    
    // 수위 데이터
    for (int i = 0; i < 6; i++) {
        char topic[50];
        sprintf(topic, "watering/tank%02d/level", i+1);
        char payload[10];
        sprintf(payload, "%d", data.water_level[i]);
        mqttClient.publish(topic, payload);
    }
    
    // 환경 데이터 전체 전송
    char jsonBuffer[1024];
    serializeJson(doc, jsonBuffer);
    mqttClient.publish("sensor/environment", jsonBuffer);
}

// MQTT 제어 명령 수신 (Callback)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr = "";
    for (int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    
    // 비상정지 명령
    if (topicStr == "emergency/stop") {
        if (payloadStr == "ON") {
            emergencyStopAll();
        } else if (payloadStr == "OFF") {
            // 비상정지 해제 (필요 시 처리)
        }
        return;
    }
    
    // 급수 펌프 제어
    if (topicStr == "watering/watering_pump/on") {
        digitalWrite(RELAY_WATERING_PUMP, HIGH);
    } else if (topicStr == "watering/watering_pump/off") {
        digitalWrite(RELAY_WATERING_PUMP, LOW);
    }
    
    // 퇴수 펌프 제어
    if (topicStr == "draining/draining_pump/on") {
        digitalWrite(RELAY_DRAINING_PUMP, HIGH);
    } else if (topicStr == "draining/draining_pump/off") {
        digitalWrite(RELAY_DRAINING_PUMP, LOW);
    }
    
    // 탱크별 솔레노이드 제어 (예시: watering/tank01/watering_sol_01/on)
    if (topicStr.startsWith("watering/tank") || topicStr.startsWith("draining/tank")) {
        // 토픽 파싱하여 해당 릴레이 제어
        int tankNum = extractTankNumber(topicStr);
        bool isWatering = topicStr.indexOf("watering") >= 0;
        bool turnOn = topicStr.endsWith("/on");
        
        int relayPin = getRelayPin(tankNum, isWatering);
        digitalWrite(relayPin, turnOn ? HIGH : LOW);
    }
}

// 비상정지: 모든 릴레이 OFF
void emergencyStopAll() {
    for (int i = 5; i <= 12; i++) {
        digitalWrite(i, LOW);
    }
    Serial.println("🚨 Emergency Stop: All relays OFF");
}
```

### 6.2 서버 소프트웨어 (Node-RED)

#### 6.2.1 Node-RED 설치 (Raspberry Pi / Windows)

```bash
# Raspberry Pi (Debian/Ubuntu)
sudo apt update
sudo apt install -y nodejs npm
sudo npm install -g --unsafe-perm node-red

# Node-RED 서비스 등록 (자동 시작)
sudo systemctl enable node-red
sudo systemctl start node-red

# 브라우저에서 접속: http://<IP>:1880
```

```powershell
# Windows (PowerShell)
# 1. Node.js 설치 (https://nodejs.org/)
# 2. Node-RED 설치
npm install -g --unsafe-perm node-red

# 3. Node-RED 실행
node-red

# 브라우저에서 접속: http://localhost:1880
```

#### 6.2.2 필수 Node-RED 노드 설치

```bash
# Node-RED 대시보드
npm install node-red-dashboard

# MQTT 노드 (기본 포함, 필요 시 재설치)
npm install node-red-contrib-aedes  # 내장 MQTT Broker

# Google Sheets 연동
npm install node-red-contrib-google-sheets

# InfluxDB 연동
npm install node-red-contrib-influxdb

# 알림 (Telegram Bot)
npm install node-red-contrib-telegrambot

# 기타 유틸리티
npm install node-red-contrib-moment   # 날짜/시간 처리
npm install node-red-node-email       # 이메일 알림
```

#### 6.2.3 Node-RED Flow 구조

업로드된 `flows_MK_ver03_with_monitoring.json` 파일 분석 결과:

**주요 탭 (Tab) 구성:**
1. **플로우 1 (978561409b7448ef)**: 메인 제어 로직
2. **플로우 2 (990d8b587cc21b97)**: 자동/수동 제어 및 모니터링

**주요 그룹 (UI Group) 구성:**
1. **Auto Control Tab (631bd3537a68a9ce)**
   - System Control (0d70ac1641b76499): 급수 시작, 비상정지, 운전 모드 선택
   - Tank Water Level (85a23e62fc978441): 수위 표시

2. **Manual Control Tab (2e1c2b071d0d754c)**
   - Pump Control (40d5938bb9a220d3): 급수/퇴수 펌프 수동 제어
   - Solenoid Control (4435e2977e8f7d0b): 탱크별 솔레노이드 수동 제어

3. **MCU Status Tab (fc9fb678385fa336)**
   - MAC 갱신, 로그 다운로드, MCU 선택

**주요 Function 노드:**
- `급수 시작 로직` (eafbec1bfbcff38e)
- `펌프 종료 판단 (75%)` (de06da5b59b6d844)
- `퇴수 시작 로직` (86c8e0433122b66e)
- `퇴수펌프 종료 판단` (c22e913458e36ee5)
- `루프(다음 급수시작)` (c5a96211b4223f6b)
- `센서값 저장 및 자동 OFF 제어` (a424698140f4fa64)
- `수동제어 신호 처리` (59f54a111521b410)
- `비상정지` / `비상정지 해제` (5830f20594d40515, dbfce2ee2ace60b9)

---

## 7. 통신 프로토콜

### 7.1 MQTT 토픽 설계

#### 7.1.1 센서 데이터 (Publish by Arduino)

| Topic | Payload 예시 | QoS | Retain | 설명 |
|-------|-------------|-----|--------|------|
| `sensor/environment` | `{"air_temp":22.5,"air_humidity":65.3,...}` | 1 | false | 전체 환경 센서 데이터 (JSON) |
| `watering/tank01/level` | `75` | 1 | false | Tank 01 수위 (%) |
| `watering/tank02/level` | `68` | 1 | false | Tank 02 수위 (%) |
| ... | ... | ... | ... | ... |
| `watering/tank06/level` | `72` | 1 | false | Tank 06 수위 (%) |
| `sensor/water/temp` | `18.5` | 1 | false | 물탱크 수온 (°C) |
| `sensor/water/ph` | `6.8` | 1 | false | 물탱크 pH |
| `sensor/water/tds` | `450` | 1 | false | 물탱크 TDS (ppm) |
| `sensor/water/ec` | `1.2` | 1 | false | 물탱크 EC (mS/cm) |
| `sensor/soil/tank01/temp` | `21.3` | 1 | false | Tank 01 토양 온도 (°C) |
| `sensor/soil/tank01/moisture` | `92.5` | 1 | false | Tank 01 토양 수분 (%) |
| `sensor/soil/tank01/ec` | `3.2` | 1 | false | Tank 01 토양 EC (μS/cm) |
| `sensor/soil/tank01/ph` | `6.5` | 1 | false | Tank 01 토양 pH |

#### 7.1.2 제어 명령 (Subscribe by Arduino)

| Topic | Payload | QoS | Retain | 설명 |
|-------|---------|-----|--------|------|
| `watering/watering_pump/on` | `ON` | 1 | false | 급수 펌프 ON |
| `watering/watering_pump/off` | `OFF` | 1 | false | 급수 펌프 OFF |
| `watering/watering_pump/manual` | `ON/OFF` | 1 | false | 급수 펌프 수동 제어 |
| `draining/draining_pump/on` | `ON` | 1 | false | 퇴수 펌프 ON |
| `draining/draining_pump/off` | `OFF` | 1 | false | 퇴수 펌프 OFF |
| `draining/draining_pump/manual` | `ON/OFF` | 1 | false | 퇴수 펌프 수동 제어 |
| `draining/extra_pump/on` | `ON` | 1 | false | 보조 펌프 ON |
| `draining/extra_pump/off` | `OFF` | 1 | false | 보조 펌프 OFF |
| `watering/tank01/watering_sol_01/on` | `ON` | 1 | false | Tank 01 급수 솔레노이드 ON |
| `watering/tank01/watering_sol_01/off` | `OFF` | 1 | false | Tank 01 급수 솔레노이드 OFF |
| `draining/tank01/draining_sol_01/on` | `ON` | 1 | false | Tank 01 퇴수 솔레노이드 ON |
| `draining/tank01/draining_sol_01/off` | `OFF` | 1 | false | Tank 01 퇴수 솔레노이드 OFF |
| `manual/control/tank01/watering` | `ON/OFF` | 1 | false | Tank 01 급수 수동 제어 |
| `manual/control/tank01/draining` | `ON/OFF` | 1 | false | Tank 01 퇴수 수동 제어 |
| `ventilation/fan/on` | `ON` | 1 | false | 환풍기 ON |
| `ventilation/fan/off` | `OFF` | 1 | false | 환풍기 OFF |
| `lighting/led/on` | `ON` | 1 | false | LED 보광등 ON |
| `lighting/led/off` | `OFF` | 1 | false | LED 보광등 OFF |
| `emergency/stop` | `ON/OFF` | 2 | true | 비상정지 명령 (QoS 2, Retain) |

#### 7.1.3 시스템 상태 (Publish by Node-RED)

| Topic | Payload 예시 | QoS | Retain | 설명 |
|-------|-------------|-----|--------|------|
| `system/mode` | `auto/manual` | 1 | true | 운전 모드 |
| `system/status` | `running/stopped/emergency` | 1 | true | 시스템 상태 |
| `system/phase` | `filling/draining/idle` | 1 | true | 현재 단계 |

### 7.2 MQTT Broker 설정 (Mosquitto)

#### 7.2.1 Mosquitto 설치 (Raspberry Pi)

```bash
# Mosquitto 설치
sudo apt update
sudo apt install -y mosquitto mosquitto-clients

# 자동 시작 설정
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# 상태 확인
sudo systemctl status mosquitto
```

#### 7.2.2 Mosquitto 설정 파일

```bash
# /etc/mosquitto/mosquitto.conf 편집
sudo nano /etc/mosquitto/mosquitto.conf
```

```conf
# Mosquitto Configuration
pid_file /var/run/mosquitto/mosquitto.pid

# Network
listener 1883
protocol mqtt

# Security (Optional: 인증 설정)
allow_anonymous true
# password_file /etc/mosquitto/passwd

# Persistence
persistence true
persistence_location /var/lib/mosquitto/

# Logging
log_dest file /var/log/mosquitto/mosquitto.log
log_type all
log_timestamp true
```

```bash
# 설정 적용
sudo systemctl restart mosquitto
```

#### 7.2.3 MQTT 인증 설정 (Optional)

```bash
# 사용자 추가 (예: wasabi_user)
sudo mosquitto_passwd -c /etc/mosquitto/passwd wasabi_user
# 비밀번호 입력

# mosquitto.conf 수정
sudo nano /etc/mosquitto/mosquitto.conf
```

```conf
allow_anonymous false
password_file /etc/mosquitto/passwd
```

```bash
# 재시작
sudo systemctl restart mosquitto
```

### 7.3 네트워크 구성

```
┌────────────────────────────────────────┐
│         Local Network (192.168.0.0/24) │
│                                        │
│  ┌──────────────────┐                 │
│  │  WiFi Router     │                 │
│  │  (192.168.0.1)   │                 │
│  └─────────┬────────┘                 │
│            │                           │
│  ┌─────────┼────────────┬─────────┐  │
│  │         │            │         │  │
│  ▼         ▼            ▼         ▼  │
│ Arduino  Node-RED    Database   PC  │
│  R4 WiFi  Server      Server         │
│ (.10)    (.100)      (.101)          │
│                                       │
│ MQTT: 192.168.0.100:1883             │
└───────────────────────────────────────┘
```

**IP 주소 할당 (권장):**
- WiFi Router: `192.168.0.1`
- Node-RED Server (Raspberry Pi): `192.168.0.100` (고정 IP)
- Arduino Uno R4 WiFi: `192.168.0.10` (고정 IP 또는 DHCP 예약)
- Database Server (InfluxDB): `192.168.0.101` (고정 IP)
- 개발용 PC: DHCP

---

## 8. 데이터베이스 설계

### 8.1 InfluxDB (시계열 데이터베이스)

#### 8.1.1 InfluxDB 설치 (Raspberry Pi / Linux)

```bash
# InfluxDB 2.x 설치
wget -q https://repos.influxdata.com/influxdata-archive_compat.key
echo '393e8779c89ac8d958f81f942f9ad7fb82a25e133faddaf92e15b16e6ac9ce4c influxdata-archive_compat.key' | sha256sum -c && cat influxdata-archive_compat.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg > /dev/null
echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list

sudo apt update
sudo apt install -y influxdb2

# 서비스 시작
sudo systemctl enable influxdb
sudo systemctl start influxdb

# 초기 설정 (웹 브라우저에서 http://<IP>:8086)
# - Organization: wasabi-farm
# - Bucket: sensor_data
# - Username: admin
# - Password: <설정>
```

#### 8.1.2 데이터 스키마 (Measurement 설계)

**1. 환경 센서 데이터 (environment)**

```
Measurement: environment
Tags:
  - location: greenhouse / outdoor
  - sensor_type: air / water / soil

Fields:
  - air_temp (float): 대기 온도 (°C)
  - air_humidity (float): 대기 습도 (%)
  - water_temp (float): 물탱크 수온 (°C)
  - water_ph (float): 물탱크 pH
  - water_tds (float): 물탱크 TDS (ppm)
  - water_ec (float): 물탱크 EC (mS/cm)

Timestamp: 측정 시각 (자동)
```

**2. 토양 센서 데이터 (soil_sensors)**

```
Measurement: soil_sensors
Tags:
  - tank_id: tank01, tank02, ..., tank06

Fields:
  - soil_temp (float): 토양 온도 (°C)
  - soil_moisture (float): 토양 수분 (%)
  - soil_ec (float): 토양 EC (μS/cm)
  - soil_ph (float): 토양 pH

Timestamp: 측정 시각 (자동)
```

**3. 수위 센서 데이터 (water_level)**

```
Measurement: water_level
Tags:
  - tank_id: tank01, tank02, ..., tank06

Fields:
  - level (int): 수위 (%)

Timestamp: 측정 시각 (자동)
```

**4. 제어 이벤트 로그 (control_events)**

```
Measurement: control_events
Tags:
  - device_type: pump / solenoid / fan / led
  - device_id: watering_pump / tank01_watering_sol / ...
  - action: on / off

Fields:
  - status (string): "ON" / "OFF"
  - mode (string): "auto" / "manual"

Timestamp: 제어 시각 (자동)
```

**5. 시스템 상태 로그 (system_status)**

```
Measurement: system_status
Tags:
  - event_type: mode_change / emergency_stop / phase_change

Fields:
  - status (string): "auto" / "manual" / "emergency" / "filling" / "draining"
  - message (string): 상태 메시지

Timestamp: 이벤트 시각 (자동)
```

#### 8.1.3 Node-RED InfluxDB 연동

```javascript
// Node-RED Function 노드: InfluxDB에 센서 데이터 저장
let payload = msg.payload;  // Arduino에서 수신한 JSON 데이터

// InfluxDB Line Protocol 형식으로 변환
let lineProtocol = `environment,location=greenhouse,sensor_type=air air_temp=${payload.air_temp},air_humidity=${payload.air_humidity} ${Date.now()}000000\n`;
lineProtocol += `environment,location=greenhouse,sensor_type=water water_temp=${payload.water_temp},water_ph=${payload.water_ph},water_tds=${payload.water_tds},water_ec=${payload.water_ec} ${Date.now()}000000\n`;

// 토양 센서 데이터
for (let i = 0; i < 6; i++) {
    let tankId = `tank${('0' + (i+1)).slice(-2)}`;
    lineProtocol += `soil_sensors,tank_id=${tankId} soil_temp=${payload.soil_temp[i]},soil_moisture=${payload.soil_moisture[i]},soil_ec=${payload.soil_ec[i]},soil_ph=${payload.soil_ph[i]} ${Date.now()}000000\n`;
}

msg.payload = lineProtocol;
return msg;
```

**InfluxDB Write 노드 설정:**
- Server: `http://192.168.0.101:8086` (InfluxDB 서버 주소)
- Organization: `wasabi-farm`
- Bucket: `sensor_data`
- Token: (InfluxDB에서 발급한 API Token)

### 8.2 Google Sheets (일일 리포트 백업)

#### 8.2.1 Google Sheets API 설정

1. **Google Cloud Console**에서 프로젝트 생성
2. **Google Sheets API** 활성화
3. **서비스 계정** 생성 및 JSON 키 다운로드
4. **Google Sheets** 파일 생성 후 서비스 계정 이메일에 편집 권한 부여

#### 8.2.2 Node-RED Google Sheets 연동

```bash
# Node-RED에 Google Sheets 노드 설치
cd ~/.node-red
npm install node-red-contrib-google-sheets
```

**Node-RED Function 노드: 일일 리포트 생성**

```javascript
// 매일 자정에 실행되는 Function 노드
let today = new Date().toISOString().split('T')[0];  // YYYY-MM-DD

// InfluxDB에서 오늘 하루 평균값 조회 (예시)
let averageData = {
    date: today,
    avg_air_temp: 21.5,
    avg_air_humidity: 68.3,
    avg_water_temp: 18.7,
    avg_water_ph: 6.8,
    // ... (나머지 평균값)
};

// Google Sheets에 추가할 데이터 (배열 형식)
msg.payload = [[
    averageData.date,
    averageData.avg_air_temp,
    averageData.avg_air_humidity,
    averageData.avg_water_temp,
    averageData.avg_water_ph,
    // ...
]];

return msg;
```

**Google Sheets Append 노드 설정:**
- Credentials: (다운로드한 JSON 키 파일 업로드)
- Spreadsheet: (Google Sheets ID)
- Sheet Name: `Daily Report`
- Range: `A:Z` (자동 추가)

---

## 9. 사용자 인터페이스

### 9.1 Node-RED Dashboard 구성

#### 9.1.1 Dashboard 탭 구조

**1. Overview (홈 화면)**
- 현재 운전 모드 (Auto / Manual)
- 시스템 상태 (Running / Stopped / Emergency)
- 현재 단계 (Filling / Draining / Idle)
- 주요 센서값 요약 (대기 온습도, 물탱크 수온/pH/EC)

**2. Auto Control (자동 제어)**
- 급수 시작 버튼
- 비상정지 ON/OFF 버튼
- 운전 모드 선택 (Auto / Manual)
- 탱크별 수위 표시 (게이지 차트)
- 제어 로그 (최근 20개)

**3. Manual Control (수동 제어)**
- 급수 펌프 ON/OFF 스위치
- 퇴수 펌프 ON/OFF 스위치
- 탱크별 급수 솔레노이드 ON/OFF (6개)
- 탱크별 퇴수 솔레노이드 ON/OFF (6개)
- 환풍기 ON/OFF 스위치
- LED 보광등 ON/OFF 스위치

**4. Monitoring (모니터링)**
- 실시간 센서 데이터 차트
  - 대기 온습도 (라인 차트)
  - 물탱크 수온/pH/EC (라인 차트)
  - 토양 온도/수분/EC/pH (탱크별 바 차트)
  - 수위 (탱크별 게이지)
- 알림 메시지 (이상 상황 알림)

**5. History (이력 조회)**
- 날짜 범위 선택
- 센서 데이터 히스토리 차트
- 제어 이벤트 로그 테이블
- CSV 다운로드 버튼

**6. Settings (설정)**
- MQTT Broker 주소 설정
- InfluxDB 연결 설정
- 알림 설정 (Telegram Bot Token, Email 등)
- 제어 파라미터 설정 (목표 수위, 대기 시간 등)

#### 9.1.2 UI 컴포넌트 예시

**대시보드 레이아웃 (Node-RED Dashboard 노드 사용):**

```
┌─────────────────────────────────────────────────────────────┐
│  📊 Wasabi Smart Farm Control System                       │
├─────────────────────────────────────────────────────────────┤
│  Mode: [Auto ▼]    Status: ● Running    Phase: Filling     │
├─────────────────────────────────────────────────────────────┤
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐│
│  │ 🌡️ Air Temp    │  │ 💧 Water Temp  │  │ 📈 Water Level ││
│  │    22.5°C      │  │    18.7°C      │  │    Tank01: 75% ││
│  └────────────────┘  └────────────────┘  │    Tank02: 68% ││
│  ┌────────────────┐  ┌────────────────┐  │    Tank03: 72% ││
│  │ 💨 Air Humidity│  │ 🧪 Water pH    │  │    Tank04: 70% ││
│  │    68.3%       │  │    6.8         │  │    Tank05: 69% ││
│  └────────────────┘  └────────────────┘  │    Tank06: 71% ││
│                                           └────────────────┘│
├─────────────────────────────────────────────────────────────┤
│  Control Panel                                              │
│  [▶️ Start Watering]  [🛑 Emergency Stop]  [🔄 Reset]      │
├─────────────────────────────────────────────────────────────┤
│  Recent Logs                                                │
│  [2024-12-11 10:35:22] 급수 시작                           │
│  [2024-12-11 10:35:25] Tank01 급수 솔레노이드 ON           │
│  [2024-12-11 10:40:18] Tank01 수위 75% 도달                │
│  [2024-12-11 10:40:19] Tank01 급수 솔레노이드 OFF          │
│  ...                                                        │
└─────────────────────────────────────────────────────────────┘
```

### 9.2 모바일 앱 (선택 사항 - Phase 3)

#### 9.2.1 접근 방법
1. **웹 기반 반응형 UI**: Node-RED Dashboard는 기본적으로 반응형 디자인 지원
2. **모바일 앱 개발**: React Native 또는 Flutter를 사용한 네이티브 앱 개발 (추후)

#### 9.2.2 모바일 기능
- 실시간 모니터링 (센서 데이터 조회)
- 푸시 알림 (이상 상황 알림)
- 원격 제어 (수동 모드에서 구동기 제어)
- 이력 조회 (일/주/월 단위)

---

## 10. 개발 로드맵

### 10.1 Phase 1: 프로토타입 개발 (1~2개월)

**목표**: Windows 11 PC 환경에서 기본 기능 개발 및 검증

| 주차 | 작업 내용 | 담당 | 산출물 |
|------|----------|------|-------|
| 1주 | - Arduino 펌웨어 기본 구조 작성<br>- 센서 데이터 수집 테스트 (I2C, Analog, 1-Wire)<br>- MQTT 통신 테스트 (Arduino ↔ Mosquitto) | 펌웨어 개발자 | - Arduino 펌웨어 v0.1<br>- 센서 테스트 보고서 |
| 2주 | - Modbus RTU 통신 구현 (RS485 - 토양 센서)<br>- 릴레이 제어 로직 구현<br>- WiFi 연결 안정화 | 펌웨어 개발자 | - Arduino 펌웨어 v0.2<br>- Modbus 통신 테스트 보고서 |
| 3주 | - Node-RED 설치 및 기본 Flow 작성<br>- MQTT Broker 설정 (Mosquitto)<br>- 센서 데이터 수신 및 시각화 | 서버 개발자 | - Node-RED Flow v0.1<br>- Dashboard 프로토타입 |
| 4주 | - 급수 제어 알고리즘 구현 (Node-RED Function)<br>- 퇴수 제어 알고리즘 구현<br>- 비상정지 로직 구현 | 서버 개발자 | - Node-RED Flow v0.2<br>- 제어 로직 검증 보고서 |
| 5주 | - 수동 제어 UI 구현 (Dashboard)<br>- 자동/수동 모드 전환 기능 구현<br>- 로그 기록 기능 구현 | 서버 개발자 | - Node-RED Flow v0.3<br>- Dashboard UI v0.1 |
| 6주 | - InfluxDB 설치 및 연동<br>- 센서 데이터 저장 로직 구현<br>- 히스토리 차트 구현 | 서버 개발자 | - InfluxDB 스키마<br>- 데이터 저장 검증 보고서 |
| 7주 | - 전체 시스템 통합 테스트<br>- 버그 수정 및 안정화<br>- 사용자 매뉴얼 작성 | 전체 팀 | - 통합 테스트 보고서<br>- 사용자 매뉴얼 v1.0 |
| 8주 | - 실제 센서/구동기 연결 테스트<br>- 현장 설치 준비<br>- 최종 검토 | 전체 팀 | - 현장 설치 가이드<br>- Phase 1 완료 보고서 |

### 10.2 Phase 2: 실제 생산 환경 배포 (1~2개월)

**목표**: Raspberry Pi 기반 실제 재배 환경 적용 및 안정화

| 주차 | 작업 내용 | 담당 | 산출물 |
|------|----------|------|-------|
| 1주 | - Raspberry Pi 설정 (OS 설치, 네트워크 구성)<br>- Node-RED 및 Mosquitto 설치<br>- InfluxDB 설치 | 시스템 관리자 | - Raspberry Pi 설정 가이드<br>- 서버 구성 완료 |
| 2주 | - 하드웨어 설치 (센서, 릴레이, 배선)<br>- 전기 안전 점검 (접지, 차단기 등)<br>- Arduino 펌웨어 배포 | 현장 설치 팀 | - 하드웨어 설치 완료<br>- 안전 점검 보고서 |
| 3주 | - Node-RED Flow 배포<br>- MQTT 통신 확인<br>- 센서 데이터 수집 확인 | 서버 개발자 | - 시스템 가동 확인<br>- 센서 데이터 수집 보고서 |
| 4주 | - 자동 제어 로직 현장 테스트<br>- 급수/퇴수 사이클 동작 확인<br>- 비상정지 기능 테스트 | 전체 팀 | - 제어 로직 검증 보고서<br>- 동작 영상/로그 |
| 5주 | - 장기 안정성 테스트 (1주일 연속 운영)<br>- 이상 상황 대응 훈련<br>- 사용자 교육 | 전체 팀 | - 안정성 테스트 보고서<br>- 사용자 교육 자료 |
| 6주 | - Google Sheets 백업 기능 구현<br>- 알림 기능 구현 (Telegram Bot)<br>- 원격 접속 설정 (VPN/DDNS) | 서버 개발자 | - 백업 시스템 구축<br>- 알림 기능 테스트 완료 |
| 7주 | - 데이터 분석 및 최적화<br>- 제어 파라미터 튜닝 (목표 수위, 대기 시간 등)<br>- 성능 개선 | 데이터 분석가 | - 데이터 분석 보고서<br>- 최적화 가이드 |
| 8주 | - 최종 검수 및 인수인계<br>- 유지보수 계획 수립<br>- Phase 2 완료 보고서 작성 | 전체 팀 | - 인수인계 문서<br>- 유지보수 매뉴얼<br>- Phase 2 완료 보고서 |

### 10.3 Phase 3: 고도화 및 확장 (3~6개월)

**목표**: AI 기반 생육 예측, 모바일 앱, 다중 농장 관리 등 고급 기능 추가

| 기능 | 설명 | 우선순위 |
|------|------|----------|
| **AI 생육 예측** | 머신러닝 모델을 사용한 와사비 생육 예측 (수확 시기, 품질 예측) | 높음 |
| **이상 탐지** | Anomaly Detection을 통한 센서 고장, 병충해 조기 감지 | 높음 |
| **모바일 앱** | React Native 또는 Flutter 기반 네이티브 모바일 앱 개발 | 중간 |
| **다중 농장 관리** | 여러 재배 시설을 하나의 대시보드에서 통합 관리 | 중간 |
| **에너지 모니터링** | 전력 소비 모니터링 및 최적화 | 낮음 |
| **영상 모니터링** | IP 카메라 연동을 통한 실시간 영상 확인 | 낮음 |
| **자동 보고서 생성** | 주간/월간 리포트 자동 생성 및 이메일 전송 | 낮음 |

---

## 11. 안전 및 보안

### 11.1 전기 안전

#### 11.1.1 고전력 구동기 안전 대책
1. **3단계 절연 구조**
   - Arduino (5V) → SSR (제어 신호) → 전자개폐기 (220V 코일) → 펌프 (220V 주전원)
   - 각 단계마다 물리적 절연 보장

2. **과전류 보호**
   - 각 220V 라인에 누전차단기 (30mA) 설치
   - 과전류 차단기 (MCB) 설치 (펌프 용량에 맞게)
   - 퓨즈 설치 (백업 보호)

3. **접지 (Grounding)**
   - 모든 금속 케이스, 제어 박스 접지
   - 센서 케이블 실드 접지
   - 접지 저항 측정 (10Ω 이하 권장)

4. **방수 및 방진**
   - 센서: IP67 이상 (물 속 침수 가능)
   - 제어 박스: IP54 이상 (분진 및 물방울 차단)
   - 릴레이 박스: 밀폐형 박스 사용

5. **과열 방지**
   - 제어 박스 내부 팬 설치 (열 방출)
   - 온도 센서 설치 (제어 박스 내부)
   - 과열 시 자동 차단 로직 구현

#### 11.1.2 비상정지 시스템
1. **하드웨어 비상정지 버튼**
   - 제어 박스에 물리적 비상정지 버튼 설치
   - 버튼 누르면 모든 220V 전원 차단 (메인 차단기 연동)

2. **소프트웨어 비상정지**
   - Node-RED Dashboard에 비상정지 버튼 배치
   - MQTT 명령으로 Arduino에 비상정지 신호 전송
   - Arduino는 모든 릴레이 즉시 OFF

3. **이중 안전 장치**
   - 하드웨어 + 소프트웨어 비상정지 병행
   - 어느 하나라도 작동 시 전체 시스템 정지

### 11.2 네트워크 보안

#### 11.2.1 MQTT 보안
1. **인증 (Authentication)**
   - Mosquitto 사용자 인증 설정 (username/password)
   - Arduino 펌웨어에 인증 정보 암호화 저장

2. **암호화 (Encryption)**
   - TLS/SSL 적용 (MQTT over TLS)
   - 인증서 발급 (Let's Encrypt 또는 자체 서명 인증서)

3. **접근 제어 (ACL)**
   - 토픽별 읽기/쓰기 권한 설정
   - Arduino는 센서 토픽만 Publish, 제어 토픽만 Subscribe 가능

#### 11.2.2 네트워크 분리
1. **로컬 네트워크 우선**
   - 스마트팜 시스템은 로컬 네트워크 내에서만 동작
   - 외부 인터넷 접속 최소화

2. **VPN 또는 DDNS**
   - 원격 접속이 필요한 경우 VPN 사용 (WireGuard, OpenVPN)
   - 또는 DDNS + 포트 포워딩 (보안 강화 필요)

3. **방화벽 (Firewall)**
   - Raspberry Pi에 UFW (Uncomplicated Firewall) 설정
   - 필요한 포트만 열기 (1880: Node-RED, 1883: MQTT, 8086: InfluxDB)

```bash
# UFW 설정 예시
sudo apt install ufw
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 1880/tcp  # Node-RED
sudo ufw allow 1883/tcp  # MQTT
sudo ufw allow 8086/tcp  # InfluxDB
sudo ufw allow 22/tcp    # SSH (로컬 네트워크만)
sudo ufw enable
```

#### 11.2.3 펌웨어 보안
1. **코드 난독화**
   - Arduino 펌웨어 컴파일 시 최적화 옵션 사용

2. **OTA (Over-The-Air) 업데이트**
   - WiFi를 통한 펌웨어 원격 업데이트 기능 구현
   - 업데이트 시 인증 및 암호화 적용

### 11.3 데이터 백업 및 복구

#### 11.3.1 백업 전략
1. **실시간 백업 (InfluxDB)**
   - InfluxDB 데이터는 매일 자정에 자동 백업
   - 백업 파일은 외부 스토리지 (USB, NAS) 저장

2. **일일 리포트 백업 (Google Sheets)**
   - 매일 평균 센서 데이터를 Google Sheets에 자동 저장
   - 클라우드 백업으로 데이터 손실 방지

3. **설정 파일 백업**
   - Node-RED Flow, Arduino 펌웨어, 설정 파일을 Git으로 버전 관리
   - GitHub 또는 GitLab에 비공개 저장소 생성

#### 11.3.2 복구 계획
1. **시스템 장애 시**
   - Raspberry Pi SD 카드 백업 이미지 준비 (주 1회)
   - 장애 시 SD 카드 교체 후 즉시 복구

2. **데이터 손실 시**
   - InfluxDB 백업 파일에서 복구
   - Google Sheets 데이터로 일일 평균값 복구

3. **하드웨어 고장 시**
   - 예비 부품 확보 (Arduino, 센서, 릴레이 등)
   - 센서 고장 시 자동 알림 (이상치 탐지)

---

## 12. 확장성 및 유지보수

### 12.1 시스템 확장성

#### 12.1.1 센서 추가
- **Modbus RTU**: 토양 센서는 최대 247개까지 확장 가능 (Slave ID 1~247)
- **I2C**: 주소 변경을 통해 동일 센서 여러 개 연결 가능 (SHT30 최대 2개)
- **Analog**: Arduino Uno R4는 아날로그 핀이 제한적 (A0~A5, 6개)
  - 확장이 필요한 경우 ADC 확장 모듈 (ADS1115 등) 사용

#### 12.1.2 구동기 추가
- **릴레이 모듈 확장**: 8채널 릴레이 모듈을 추가로 연결하여 제어 채널 확장
- **PWM 채널 확장**: PCA9685 (I2C PWM 드라이버)를 사용하여 LED 조명 다채널 제어

#### 12.1.3 다중 Arduino 구성
- 센서/구동기가 많아지면 Arduino를 여러 개로 분산
- 각 Arduino는 고유한 MQTT Client ID로 연결
- Node-RED에서 토픽 분리 (예: `sensor/arduino1/...`, `sensor/arduino2/...`)

### 12.2 유지보수 계획

#### 12.2.1 정기 점검 (주간)
- [ ] 센서 데이터 정상 수신 확인
- [ ] 제어 로직 동작 확인 (급수/퇴수 사이클)
- [ ] 릴레이 접점 상태 확인 (소음, 발열 등)
- [ ] MQTT Broker 연결 상태 확인

#### 12.2.2 정기 점검 (월간)
- [ ] 센서 보정 (pH, TDS, EC 센서)
- [ ] 배선 및 커넥터 점검 (느슨한 연결 확인)
- [ ] 제어 박스 내부 청소 (먼지 제거)
- [ ] 데이터베이스 백업 확인
- [ ] 로그 파일 정리 (오래된 로그 삭제)

#### 12.2.3 정기 점검 (분기)
- [ ] 센서 교체 필요 여부 확인 (수명, 고장 등)
- [ ] 릴레이 교체 필요 여부 확인 (수명, 접점 마모 등)
- [ ] 소프트웨어 업데이트 (Node-RED, InfluxDB 등)
- [ ] 펌웨어 업데이트 (Arduino)
- [ ] 보안 점검 (비밀번호 변경, 방화벽 규칙 검토)

#### 12.2.4 문제 해결 가이드

**1. 센서 데이터가 수신되지 않을 때**
- Arduino WiFi 연결 확인 (시리얼 모니터로 디버깅)
- MQTT Broker 연결 상태 확인 (`mosquitto_sub -v -t '#'`로 전체 토픽 모니터링)
- 센서 전원 및 배선 확인

**2. 제어 명령이 동작하지 않을 때**
- MQTT 명령 전송 확인 (Node-RED Debug 노드로 확인)
- Arduino가 해당 토픽을 Subscribe하고 있는지 확인
- 릴레이 동작 확인 (LED 점등, 딸깍 소리)

**3. 시스템이 응답하지 않을 때**
- Raspberry Pi 전원 확인
- SSH로 접속하여 프로세스 상태 확인 (`systemctl status node-red mosquitto influxdb`)
- 로그 파일 확인 (`journalctl -u node-red -f`)

---

## 13. 예산 및 리소스

### 13.1 하드웨어 비용 (예상)

| 품목 | 모델명 | 단가 (원) | 수량 | 합계 (원) | 비고 |
|------|-------|----------|-----|----------|------|
| **메인 MCU** | Arduino Uno R4 WiFi | 45,000 | 1 | 45,000 | WiFi 내장 |
| **RS485 확장보드** | DFR0259 | 25,000 | 1 | 25,000 | Modbus RTU |
| **대기 온습도 센서** | SHT30 (P4422-3) | 35,000 | 1 | 35,000 | 고정밀 I2C |
| **토양 센서 (4-in-1)** | SEN0604 | 120,000 | 18 | 2,160,000 | Modbus, 온도/수분/EC/pH |
| **수온 센서** | DS18B20 (DFR0198) | 12,000 | 1 | 12,000 | 방수 1-Wire |
| **물탱크 pH 센서** | SEN0161 | 60,000 | 1 | 60,000 | 아날로그 |
| **물탱크 TDS 센서** | SEN0244 | 25,000 | 1 | 25,000 | 아날로그 |
| **물탱크 EC 센서 (Pro)** | SEN0451 | 80,000 | 1 | 80,000 | 고정밀 아날로그 |
| **수위 센서** | 초음파/압력 센서 | 15,000 | 18 | 270,000 | 탱크별 |
| **릴레이 모듈 (8채널)** | 5V 릴레이 모듈 | 20,000 | 5 | 100,000 | 솔레노이드 제어용 (36개 제어) |
| **SSR (Solid State Relay)** | 40A SSR | 30,000 | 2 | 60,000 | 펌프 제어용 |
| **전자개폐기 (MC)** | LS MC-18b | 50,000 | 1 | 50,000 | 2HP 펌프용 |
| **전자개폐기 (MC)** | LS MC-12b | 40,000 | 1 | 40,000 | 1HP 펌프용 |
| **솔레노이드 밸브** | 12V DC 솔레노이드 | 25,000 | 36 | 900,000 | 급수/퇴수 각 18개 |
| **전원 공급장치** | 12V/5A | 25,000 | 1 | 25,000 | Arduino, 센서, 솔레노이드 |
| **전원 공급장치** | 5V/3A | 15,000 | 1 | 15,000 | Arduino 백업 |
| **제어 박스** | 방수 박스 (IP54) | 50,000 | 1 | 50,000 | 릴레이 수납 |
| **배선 자재** | 전선, 커넥터, 전선관 등 | - | - | 100,000 | 일괄 구매 |
| **Raspberry Pi** | Raspberry Pi 4 (4GB) | 80,000 | 1 | 80,000 | 서버용 |
| **SD 카드** | 64GB Class 10 | 15,000 | 2 | 30,000 | OS + 백업 |
| **기타 부품** | 예비 부품, 공구 등 | - | - | 100,000 | 일괄 |
| **총계** | | | | **4,322,000** | 약 432만원 |

### 13.2 소프트웨어 비용

| 품목 | 비용 | 비고 |
|------|------|------|
| **Arduino IDE** | 무료 | 오픈소스 |
| **Node-RED** | 무료 | 오픈소스 |
| **Mosquitto (MQTT Broker)** | 무료 | 오픈소스 |
| **InfluxDB** | 무료 (OSS 버전) | 클라우드는 유료 |
| **Google Sheets API** | 무료 | 일일 할당량 내 |
| **총계** | **무료** | 모두 오픈소스 또는 무료 |

### 13.3 인력 및 시간 (예상)

| 역할 | 인원 | 투입 기간 | 비고 |
|------|------|----------|------|
| **펌웨어 개발자** | 1 | 2개월 | Arduino C++ 개발 |
| **서버 개발자** | 1 | 2개월 | Node-RED Flow 개발 |
| **시스템 관리자** | 1 | 1개월 | Raspberry Pi 설정, DB 구축 |
| **현장 설치 팀** | 2 | 1개월 | 하드웨어 설치, 배선 작업 |
| **데이터 분석가** | 1 | 1개월 (Phase 2 이후) | 데이터 분석, 최적화 |

### 13.4 운영 비용 (월간)

| 품목 | 비용 (원/월) | 비고 |
|------|--------------|------|
| **전기료** | 약 30,000 | 펌프, 센서, 서버 운영 |
| **인터넷 (WiFi)** | 약 40,000 | 기존 사용 중이면 추가 비용 없음 |
| **클라우드 백업 (선택)** | 0~10,000 | Google Drive 무료 용량 사용 가능 |
| **유지보수** | 약 50,000 | 센서 보정, 부품 교체 등 |
| **총계** | **약 120,000** | 월 약 12만원 |

---

## 14. 기술 지원 및 문의

### 14.1 기술 문서
- **Arduino 공식 문서**: https://docs.arduino.cc/
- **Node-RED 공식 문서**: https://nodered.org/docs/
- **InfluxDB 공식 문서**: https://docs.influxdata.com/
- **Mosquitto 공식 문서**: https://mosquitto.org/documentation/

### 14.2 커뮤니티
- **Arduino Forum**: https://forum.arduino.cc/
- **Node-RED Forum**: https://discourse.nodered.org/
- **InfluxDB Community**: https://community.influxdata.com/

### 14.3 오픈소스 라이브러리
- **PubSubClient (MQTT)**: https://github.com/knolleary/pubsubclient
- **ArduinoModbus**: https://github.com/arduino-libraries/ArduinoModbus
- **DallasTemperature**: https://github.com/milesburton/Arduino-Temperature-Control-Library

---

## 15. 결론

본 프로젝트는 **Arduino Uno R4 WiFi**를 엣지 디바이스로, **Node-RED**를 메인 서버로 활용하여 와사비 재배 환경을 정밀하게 제어하는 **IoT 기반 스마트팜 시스템**을 구축하는 것을 목표로 합니다.

### 주요 특징
1. ✅ **실시간 모니터링**: 대기, 토양, 물탱크 센서 데이터를 실시간으로 수집 및 시각화
2. ✅ **자동 제어**: 급수/퇴수 사이클, 환풍기, LED 보광등을 자동으로 제어
3. ✅ **수동 제어**: 필요 시 사용자가 직접 구동기를 제어 (상호 배제 로직 적용)
4. ✅ **비상정지**: 하드웨어 + 소프트웨어 이중 안전 장치
5. ✅ **데이터 저장**: InfluxDB (시계열) + Google Sheets (일일 리포트)
6. ✅ **확장성**: 센서/구동기 추가 용이, 다중 농장 관리 가능
7. ✅ **비용 효율**: 약 200만원 하드웨어 + 오픈소스 소프트웨어

### 기대 효과
- 🌱 **와사비 생육 최적화**: 정밀한 환경 제어로 품질 향상
- 💰 **노동력 절감**: 자동화를 통한 관리 시간 50% 이상 감소
- 📊 **데이터 기반 의사결정**: 센서 데이터 분석을 통한 재배 노하우 체계화
- 🚀 **확장 가능**: AI 생육 예측, 모바일 앱 등 고급 기능 추가 가능

### 다음 단계
1. **Phase 1 시작**: Arduino 펌웨어 및 Node-RED Flow 개발 착수
2. **하드웨어 구매**: 센서, 릴레이, 제어 보드 일괄 구매
3. **프로토타입 테스트**: Windows 11 PC 환경에서 기능 검증
4. **현장 배포**: Raspberry Pi 기반 실제 재배 환경 적용

---

## 부록

### A. 참고 자료
- 업로드된 Node-RED Flow JSON 파일 (`flows_MK_ver03_with_monitoring.json`)
- 제어 흐름도 이미지 (업로드된 이미지)
- 기존 AI 제안 기획서 (제공된 텍스트)

### B. 용어 설명
- **MQTT**: Message Queuing Telemetry Transport (IoT 통신 프로토콜)
- **Modbus RTU**: 산업용 통신 프로토콜 (RS485 기반)
- **InfluxDB**: 시계열 데이터베이스 (Time-series Database)
- **Node-RED**: Flow 기반 시각적 프로그래밍 툴
- **SSR**: Solid State Relay (반도체 릴레이)
- **MC**: Magnetic Contactor (전자개폐기)
- **QoS**: Quality of Service (MQTT 메시지 전송 품질)
- **Retain**: MQTT 메시지 보존 기능 (새 클라이언트 접속 시 마지막 메시지 전달)

### C. 코드 저장소
- GitHub Repository (추천): https://github.com/your-repo/wasabi-smartfarm
- 폴더 구조 예시:
```
wasabi-smartfarm/
├── arduino/
│   ├── wasabi_controller/
│   │   ├── wasabi_controller.ino
│   │   ├── config.h
│   │   ├── mqtt.cpp
│   │   ├── sensors.cpp
│   │   └── actuators.cpp
│   └── libraries/
├── nodered/
│   ├── flows.json
│   └── settings.js
├── docs/
│   ├── USER_MANUAL.md
│   ├── INSTALLATION_GUIDE.md
│   └── API_REFERENCE.md
└── README.md
```

### D. 제어 로직 파라미터 요약표

| 제어 항목 | 조건 | 임계값 | 동작 시간 | 비고 |
|---------|-----|-------|----------|------|
| **관수장비 (조건1)** | 토양습도 | ≤ 95% | 4분/시간 | OR 조건 |
| **관수장비 (조건2)** | 토양EC | ≥ 5.0 μS/cm | 4분/시간 | OR 조건 |
| **관수장비 (조건3)** | 토양온도 | ≥ 22°C | 4분/시간 | OR 조건 (신규 추가) |
| **냉각수 모터** | 대기 온도 | ≥ 22°C | 온도 정상화까지 | 자동 ON/OFF |
| **온수 모터** | 대기 온도 | ≤ 18°C | 온도 정상화까지 | 자동 ON/OFF |
| **LED 보조광원** | 시간 주기 | 2시간마다 | 10분 OFF | 주기적 OFF |
| **퇴수모터** | 수위 센서 | 일정 수위 이상 (80%) | 수위 정상화까지 | 관수 후 연동 |
| **필터링 & UV살균** | 퇴수 후 | 자동 실행 | - | 별도 제어 |

**조건 로직 설명:**
- **관수장비**: 3가지 조건 중 **하나라도** 만족하면 작동 (OR 조건)
  - 토양 습도 95% 이하
  - 토양 EC 5.0 μS/cm 이상
  - **토양 온도 22°C 이상** ⭐ (와사비 재배 최적 온도 유지)
- **냉각/온수 모터**: 대기 온도 기준 (18~22°C 적정 범위)
- **LED**: 2시간 주기로 10분 OFF (광주기 관리)
- **퇴수**: 관수 후 수위가 80% 이상일 때 자동 실행

---

**문서 버전**: v1.1  
**작성일**: 2024-12-11  
**작성자**: AI Assistant  
**최종 수정일**: 2024-12-11  
**변경 이력**:
- v1.1 (2024-12-11): 관수장비 작동 조건에 "토양 온도 22°C 이상" 추가
- v1.0 (2024-12-11): 초기 버전 작성
