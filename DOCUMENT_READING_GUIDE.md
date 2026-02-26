# Wasabi Smart Farm 문서 읽기 순서 가이드

**버전**: v1.0.0  
**작성일**: 2025-12-27  
**작성자**: 서준원  
**목적**: 프로젝트 문서들을 효율적으로 읽기 위한 순서 및 위치 안내

---

## 문서 읽기 순서 개요

Wasabi Smart Farm 프로젝트는 **4단계 학습 경로**로 구성되어 있습니다:

```
단계 1: 프로젝트 이해 (기획 및 시스템 구조)
   ↓
단계 2: 시스템 구성 (센서, 노드, 아키텍처)
   ↓
단계 3: 설치 및 설정 (실제 구축)
   ↓
단계 4: 안정성 및 유지보수 (운영 관리)
```

---

## 단계 1: 프로젝트 이해 (시작점)

### 목적
프로젝트 전체 개요, 목표, 시스템 아키텍처를 이해합니다.

### 읽기 순서

#### 1-1. README.md
**위치**: `/README.md`  
**크기**: 9.5KB  
**소요 시간**: 10분

**내용**:
- 프로젝트 소개 및 주요 특징
- 시스템 아키텍처 다이어그램
- 하드웨어 구성 요약
- 소프트웨어 스택 (Arduino, Node-RED)
- 시작하기 (Getting Started)
- 개발 로드맵 및 진행 상황

**읽어야 하는 이유**: 프로젝트 전체 그림을 가장 빠르게 파악할 수 있는 문서

---

#### 1-2. 와사비 재배 스마트팜 제어 시스템 개발 프로젝트 기획서
**위치**: `/docs/WASABI_SMARTFARM_PROJECT_PROPOSAL.md`  
**크기**: 약 50KB (추정)  
**소요 시간**: 40분

**내용**:
- 프로젝트 배경 및 목적
- 시스템 아키텍처 상세 설명
- 하드웨어 구성 (센서, 액추에이터, 제어보드)
- 센서 시스템 명세 (SHT30, SEN0604, DS18B20 등)
- 제어 로직 및 알고리즘 (급수, 온도, LED 제어)
- 통신 프로토콜 (MQTT, Modbus RTU, I2C)
- 데이터베이스 설계 (InfluxDB, Google Sheets)
- 개발 로드맵 (Phase 1~3)
- 예산 및 리소스

**읽어야 하는 이유**: 시스템 설계 철학과 기술적 결정 배경을 이해할 수 있음

---

## 단계 2: 시스템 구성 이해

### 목적
실제 센서 노드 구성, 센서 명세, 시스템 변경 이력을 이해합니다.

### 읽기 순서

#### 2-1. 센서 구성 명세서
**위치**: `/SENSOR_SPECIFICATION.md`  
**크기**: 11KB  
**소요 시간**: 15분

**내용**:
- 전체 센서 개요 (38개 센서)
- 대기 센서 (SHT30 × 1)
- 토양 센서 (SEN0604 × 18)
- 물탱크 센서 (DS18B20, pH, TDS, EC)
- 수위 센서 (HC-SR04 × 1)
- 센서 노드 구성 (21개 노드)
- 센서 읽기 주기 및 통신 방식
- 센서 보정 방법
- 유지보수 가이드

**읽어야 하는 이유**: 실제 사용되는 센서 종류와 개수, 통신 방식을 상세히 파악

---

#### 2-2. 센서 노드 재구성 변경사항 요약
**위치**: `/SENSOR_NODE_REORGANIZATION.md`  
**크기**: 7.2KB  
**소요 시간**: 10분

**내용**:
- AS-IS vs TO-BE 비교
- 대기+토양 통합 노드 (노드 13, 14, 15) 추가
- 토양 전용 노드 (노드 01~12, 16~18)
- 수위 센서 노드 (노드 20) 신규 추가
- 노드 번호 할당표 (21개 노드)
- MQTT 토픽 변경 사항
- Node-RED 수정 필요사항
- 설치 순서

**읽어야 하는 이유**: 최신 센서 노드 구성 (v2.0.0)을 반영한 변경 내역 파악

---

## 단계 3: 설치 및 설정 (실습 단계)

### 목적
실제로 시스템을 구축하고 설정하는 방법을 학습합니다.

### 읽기 순서

#### 3-1. Wasabi Smart Farm 전체 설정 매뉴얼 (가장 중요!)
**위치**: `/WASABI_SMARTFARM_SETTING.md`  
**크기**: 48KB (1,794줄)  
**소요 시간**: 2시간 (읽기) + 8~10시간 (실습)

**내용**:
1. 시스템 개요
2. 시스템 요구사항
3. 사전 준비사항
4. Windows 서버 환경 구축 (Node.js 설치)
5. Node-RED 설치 및 설정
6. Mosquitto MQTT Broker 설치
7. Arduino 개발 환경 설정
8. 센서 노드 펌웨어 업로드 (21개 노드)
9. Node-RED Flow 배포
10. 시스템 자동 재시작 설정
11. 최종 검증 및 테스트
12. 문제 해결 가이드
13. 유지보수 가이드

**부록**:
- A: 참고 자료
- B: MQTT 토픽 목록
- C: 제어 로직

**읽어야 하는 이유**: 
- **가장 완전하고 최신화된 설치 가이드**
- step-by-step 실행 가능
- 모든 소프트웨어 설치 방법 포함
- 21개 노드 업로드 가이드 포함
- 문제 해결 가이드 포함

**중요**: 이 문서만으로 처음부터 끝까지 시스템 구축 가능

---

#### 3-2. (선택) 설치 매뉴얼 (구버전)
**위치**: `/INSTALLATION_MANUAL.md`  
**크기**: 33KB  
**소요 시간**: 1시간

**내용**:
- Windows 노트북 신규 설치
- 소프트웨어 설치 (Node-RED, Mosquitto)
- Arduino 개발 환경 설정
- Phase 1 안정화 작업

**읽어야 하는 이유**: 
- `WASABI_SMARTFARM_SETTING.md`의 이전 버전
- 일부 세부 사항이 다를 수 있음
- **권장: `WASABI_SMARTFARM_SETTING.md`를 우선 참조**

---

#### 3-3. Arduino 노드별 README 문서

**읽는 시점**: 각 노드 펌웨어 업로드 시

##### 3-3-1. 대기+토양 통합 노드
**위치**: `/arduino/air_soil_combined_node/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 13, 14, 15 개요
- 하드웨어 구성 (SHT30 + SEN0604)
- 연결 핀맵
- 필수 라이브러리
- config.h 설정 방법
- MQTT 토픽
- 업로드 방법
- 문제 해결

##### 3-3-2. 토양 전용 노드
**위치**: `/arduino/soil_sensor_node/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 01~12, 16~18 개요
- SEN0604 센서 구성
- RS485 연결
- config.h 설정
- 업로드 방법

##### 3-3-3. 물탱크 센서 노드
**위치**: `/arduino/water_tank_sensor_node/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 19 개요
- DS18B20 + pH/TDS/EC 센서
- 센서 보정 방법
- 업로드 방법

##### 3-3-4. 수위 센서 노드 (ESP8266)
**위치**: `/arduino/water_level_sensor_node/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 20 (Wemos D1 R1)
- HC-SR04 초음파 센서
- ESP8266 개발 환경
- 업로드 방법

##### 3-3-5. 수위 센서 노드 (Arduino R4)
**위치**: `/arduino/water_level_sensor_node_r4/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 20 (Arduino Uno R4 WiFi 대체)
- HC-SR04 초음파 센서
- ESP8266 버전과 비교
- 업로드 방법

##### 3-3-6. 액추에이터 노드
**위치**: `/arduino/actuator_node/README.md`  
**소요 시간**: 10분

**내용**:
- 노드 21 개요
- 릴레이 제어 (급수, 퇴수, 환풍기, LED)
- 제어 로직
- 업로드 방법

---

#### 3-4. Node-RED 관련 문서

**읽는 시점**: Node-RED Flow 배포 및 수정 시

##### 3-4-1. Node-RED Flow 수정 가이드
**위치**: `/nodered/NODE_RED_UPDATE_GUIDE.md`  
**크기**: 약 8KB  
**소요 시간**: 15분

**내용**:
- 센서 노드 재구성에 따른 Flow 수정
- 하트비트 모니터링 수정 (21개 노드)
- 센서 데이터 수신 노드 추가
- Dashboard UI 수정
- 테스트 방법

##### 3-4-2. Context Storage 설정 가이드
**위치**: `/nodered/CONTEXT_STORAGE_SETUP_GUIDE.md`  
**소요 시간**: 10분

**내용**:
- Context Storage 영구 저장 설정
- settings.js 수정 방법
- localfilesystem 모듈 사용

##### 3-4-3. 하트비트 모니터링 가이드
**위치**: `/nodered/HEARTBEAT_MONITORING_GUIDE.md`  
**소요 시간**: 15분

**내용**:
- 21개 노드 하트비트 모니터링
- Dashboard UI 디자인
- 타임아웃 설정 (2분)
- 상태 표시 (온라인/타임아웃/연결안됨)

##### 3-4-4. Task Scheduler 자동 재시작 가이드
**위치**: `/nodered/TASK_SCHEDULER_AUTO_RESTART_GUIDE.md`  
**소요 시간**: 20분

**내용**:
- Windows Task Scheduler 설정
- Node-RED 자동 시작
- Mosquitto 자동 시작
- 모니터링 스크립트 작성

---

## 단계 4: 안정성 및 유지보수

### 목적
24/7/365 무중단 운영을 위한 안정성 분석 및 개선 사항을 이해합니다.

### 읽기 순서

#### 4-1. 시스템 안정성 분석
**위치**: `/SYSTEM_RELIABILITY_ANALYSIS.md`  
**크기**: 22KB  
**소요 시간**: 30분

**내용**:
- 24/7/365 무중단 운영 요구사항
- 잠재적 장애 포인트 7개 분석
- 현재 구현된 안전 메커니즘
- 개선 권장사항
- Phase 1 구현 계획 (4단계)
- WiFi 재연결, Context Storage, Heartbeat, 자동 재시작

**읽어야 하는 이유**: 시스템 안정성 확보를 위한 설계 철학 이해

---

#### 4-2. 시스템 안정성 구현
**위치**: `/SYSTEM_RELIABILITY_IMPLEMENTATION.md`  
**크기**: 18KB  
**소요 시간**: 25분

**내용**:
- Phase 1 구현 상세 내역
- Phase 1-1: WiFi 재연결 로직 개선
- Phase 1-2: Context Storage 설정
- Phase 1-3: Heartbeat 모니터링 구현
- Phase 1-4: Task Scheduler 자동 재시작
- 각 단계별 구현 방법 및 검증

**읽어야 하는 이유**: 실제 안정성 기능 구현 과정 이해

---

#### 4-3. 시스템 안정성 최종 보고서
**위치**: `/SYSTEM_RELIABILITY_FINAL_REPORT.md`  
**크기**: 21KB  
**소요 시간**: 25분

**내용**:
- Phase 1 완료 결과
- 시스템 안정성 검증 결과
- 달성된 운영 지표
- 향후 Phase 2 계획 (Watchdog, Blocking Delay 제거)

**읽어야 하는 이유**: Phase 1 완료 후 시스템 상태 파악

---

#### 4-4. 안정성 검사 요약
**위치**: `/STABILITY_CHECK_SUMMARY.md`  
**크기**: 4.5KB  
**소요 시간**: 10분

**내용**:
- 치명적 이슈 (Critical Issues)
- 경고 이슈 (Warning Issues)
- 정상 항목
- 권장 해결 방안

**읽어야 하는 이유**: 현재 시스템의 잠재적 문제점 빠르게 파악

---

## 추가 문서 (필요 시 참조)

### Arduino 컴파일 및 개선 가이드

#### A-1. Arduino-Node-RED 통합 체크리스트
**위치**: `/ARDUINO_NODERED_INTEGRATION_CHECKLIST.md`  
**크기**: 22KB  
**소요 시간**: 20분

**내용**:
- Arduino와 Node-RED 통합 시 체크리스트
- MQTT 연결 확인
- 센서 데이터 수신 확인
- 제어 명령 전달 확인

#### A-2. 컴파일 오류 수정 가이드
**위치**: `/arduino/soil_sensor_node/COMPILATION_FIX_GUIDE.md`  
**소요 시간**: 10분

**내용**:
- Arduino 컴파일 오류 해결 방법
- 라이브러리 버전 충돌 해결
- ModbusMaster 라이브러리 마이그레이션

#### A-3. WiFi 재연결 개선 가이드
**위치**: `/arduino/WIFI_RECONNECT_IMPROVEMENT.md`  
**소요 시간**: 10분

**내용**:
- WiFi 재연결 로직 개선
- 무한 루프 제거
- 재시도 횟수 및 간격 설정

---

## 문서 위치 요약표

| 문서명 | 경로 | 크기 | 단계 |
|--------|------|------|------|
| **README.md** | `/README.md` | 9.5KB | 1-1 |
| **프로젝트 기획서** | `/docs/WASABI_SMARTFARM_PROJECT_PROPOSAL.md` | ~50KB | 1-2 |
| **센서 명세서** | `/SENSOR_SPECIFICATION.md` | 11KB | 2-1 |
| **센서 노드 재구성** | `/SENSOR_NODE_REORGANIZATION.md` | 7.2KB | 2-2 |
| **설정 매뉴얼 (최신)** | `/WASABI_SMARTFARM_SETTING.md` | 48KB | 3-1 ⭐ |
| **설치 매뉴얼 (구버전)** | `/INSTALLATION_MANUAL.md` | 33KB | 3-2 |
| **대기+토양 노드** | `/arduino/air_soil_combined_node/README.md` | - | 3-3-1 |
| **토양 노드** | `/arduino/soil_sensor_node/README.md` | - | 3-3-2 |
| **물탱크 노드** | `/arduino/water_tank_sensor_node/README.md` | - | 3-3-3 |
| **수위 노드 (ESP8266)** | `/arduino/water_level_sensor_node/README.md` | - | 3-3-4 |
| **수위 노드 (R4)** | `/arduino/water_level_sensor_node_r4/README.md` | - | 3-3-5 |
| **액추에이터 노드** | `/arduino/actuator_node/README.md` | - | 3-3-6 |
| **Node-RED 수정** | `/nodered/NODE_RED_UPDATE_GUIDE.md` | 8KB | 3-4-1 |
| **Context Storage** | `/nodered/CONTEXT_STORAGE_SETUP_GUIDE.md` | - | 3-4-2 |
| **하트비트 모니터링** | `/nodered/HEARTBEAT_MONITORING_GUIDE.md` | - | 3-4-3 |
| **자동 재시작** | `/nodered/TASK_SCHEDULER_AUTO_RESTART_GUIDE.md` | - | 3-4-4 |
| **안정성 분석** | `/SYSTEM_RELIABILITY_ANALYSIS.md` | 22KB | 4-1 |
| **안정성 구현** | `/SYSTEM_RELIABILITY_IMPLEMENTATION.md` | 18KB | 4-2 |
| **안정성 보고서** | `/SYSTEM_RELIABILITY_FINAL_REPORT.md` | 21KB | 4-3 |
| **안정성 검사 요약** | `/STABILITY_CHECK_SUMMARY.md` | 4.5KB | 4-4 |

---

## 사용 시나리오별 읽기 순서

### 시나리오 1: 프로젝트 처음 접하는 사람

```
1. README.md (10분)
   ↓
2. WASABI_SMARTFARM_PROJECT_PROPOSAL.md (40분)
   ↓
3. SENSOR_SPECIFICATION.md (15분)
   ↓
4. SENSOR_NODE_REORGANIZATION.md (10분)
   ↓
5. WASABI_SMARTFARM_SETTING.md (2시간 읽기, 8~10시간 실습)
```

**총 소요 시간**: 약 3시간 (읽기) + 8~10시간 (실습)

---

### 시나리오 2: 시스템 설치만 하고 싶은 사람

```
1. README.md (10분) - 전체 개요 파악
   ↓
2. SENSOR_NODE_REORGANIZATION.md (10분) - 최신 노드 구성 확인
   ↓
3. WASABI_SMARTFARM_SETTING.md (전체 읽고 실습)
   - 단계별로 따라하면서 시스템 구축
   - 문제 발생 시 "12. 문제 해결 가이드" 참조
```

**총 소요 시간**: 약 20분 (읽기) + 8~10시간 (실습)

---

### 시나리오 3: 특정 Arduino 노드만 수정하고 싶은 사람

```
1. SENSOR_NODE_REORGANIZATION.md (10분) - 노드 구성 확인
   ↓
2. 해당 노드의 README.md (10분)
   예: /arduino/air_soil_combined_node/README.md
   ↓
3. WASABI_SMARTFARM_SETTING.md의 "8. 센서 노드 펌웨어 업로드" 섹션 참조
```

**총 소요 시간**: 약 20분 (읽기) + 15분 (펌웨어 업로드)

---

### 시나리오 4: Node-RED Flow만 수정하고 싶은 사람

```
1. SENSOR_NODE_REORGANIZATION.md (10분) - 변경된 토픽 확인
   ↓
2. NODE_RED_UPDATE_GUIDE.md (15분) - 수정 가이드
   ↓
3. WASABI_SMARTFARM_SETTING.md의 "9. Node-RED Flow 배포" 참조
```

**총 소요 시간**: 약 25분 (읽기) + 2.5시간 (수정 및 테스트)

---

### 시나리오 5: 안정성 및 유지보수 담당자

```
1. SYSTEM_RELIABILITY_ANALYSIS.md (30분) - 안정성 이슈 파악
   ↓
2. SYSTEM_RELIABILITY_IMPLEMENTATION.md (25분) - 구현 방법
   ↓
3. SYSTEM_RELIABILITY_FINAL_REPORT.md (25분) - 현재 상태
   ↓
4. STABILITY_CHECK_SUMMARY.md (10분) - 체크리스트
   ↓
5. WASABI_SMARTFARM_SETTING.md의 "13. 유지보수 가이드" 참조
```

**총 소요 시간**: 약 1시간 30분 (읽기)

---

## 빠른 참조 (Quick Reference)

### 가장 중요한 3개 문서

1. **WASABI_SMARTFARM_SETTING.md** - 전체 설치 매뉴얼 (최신)
2. **SENSOR_NODE_REORGANIZATION.md** - 최신 노드 구성
3. **README.md** - 프로젝트 개요

### 문제 발생 시 참조 문서

| 문제 유형 | 참조 문서 | 섹션 |
|----------|----------|------|
| Arduino 업로드 오류 | WASABI_SMARTFARM_SETTING.md | 12.1 |
| WiFi 연결 오류 | WASABI_SMARTFARM_SETTING.md | 12.2 |
| MQTT 연결 오류 | WASABI_SMARTFARM_SETTING.md | 12.3 |
| Node-RED 오류 | WASABI_SMARTFARM_SETTING.md | 12.4 |
| 센서 읽기 오류 | WASABI_SMARTFARM_SETTING.md | 12.5 |
| 시스템 안정성 이슈 | STABILITY_CHECK_SUMMARY.md | 전체 |

---

## 문서 업데이트 이력

### v1.0.0 (2025-12-27)
- 최초 작성
- 4단계 학습 경로 정의
- 21개 문서 위치 및 읽기 순서 정리
- 5가지 사용 시나리오별 읽기 순서 제공

---

## 작성자

**서준원**

## GitHub Repository

**URL**: https://github.com/phdsjw/WasabiSmartFarm

---

**이 가이드를 따라 문서를 읽으면 Wasabi Smart Farm 프로젝트를 효율적으로 이해하고 구축할 수 있습니다.**

---

**END OF DOCUMENT**
