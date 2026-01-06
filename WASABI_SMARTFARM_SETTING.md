# Wasabi Smart Farm 전체 설정 매뉴얼

**버전**: v2.0.0  
**작성일**: 2025-12-27  
**대상**: Windows 노트북 + Arduino 노드 전체 설치  
**예상 소요 시간**: 약 8-10시간  
**작성자**: 서준원

---

## 목차

1. [시스템 개요](#1-시스템-개요)
2. [시스템 요구사항](#2-시스템-요구사항)
3. [사전 준비사항](#3-사전-준비사항)
4. [Windows 서버 환경 구축](#4-windows-서버-환경-구축)
5. [Node-RED 설치 및 설정](#5-node-red-설치-및-설정)
6. [Mosquitto MQTT Broker 설치](#6-mosquitto-mqtt-broker-설치)
7. [Arduino 개발 환경 설정](#7-arduino-개발-환경-설정)
8. [센서 노드 펌웨어 업로드](#8-센서-노드-펌웨어-업로드)
9. [Node-RED Flow 배포](#9-node-red-flow-배포)
10. [시스템 자동 재시작 설정](#10-시스템-자동-재시작-설정)
11. [최종 검증 및 테스트](#11-최종-검증-및-테스트)
12. [문제 해결 가이드](#12-문제-해결-가이드)
13. [유지보수 가이드](#13-유지보수-가이드)

---

## 1. 시스템 개요

### 1.1 프로젝트 소개

**Wasabi Smart Farm Control System**은 와사비 재배를 위한 IoT 기반 스마트팜 자동화 시스템입니다.

### 1.2 시스템 아키텍처

```
┌─────────────────────────────────────────────────────────┐
│                   Cloud / Remote Access                  │
│            (Google Sheets, InfluxDB Cloud)              │
└───────────────────────┬─────────────────────────────────┘
                        │ Internet
┌───────────────────────┴─────────────────────────────────┐
│                   Server Layer (Windows)                 │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────────┐   │
│  │ Node-RED    │  │ Mosquitto   │  │   InfluxDB   │   │
│  │  (1880)     │  │   (1883)    │  │    (8086)    │   │
│  └─────────────┘  └─────────────┘  └──────────────┘   │
└───────────────────────┬─────────────────────────────────┘
                        │ WiFi (MQTT)
┌───────────────────────┴─────────────────────────────────┐
│                Edge Layer (Arduino Nodes)                │
│                                                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌─────────┐│
│  │ 노드13~15│  │ 노드01~12│  │  노드19  │  │ 노드20  ││
│  │  대기+   │  │          │  │          │  │         ││
│  │  토양    │  │  토양    │  │  물탱크  │  │  수위   ││
│  │ SHT30+   │  │ SEN0604  │  │ DS18B20  │  │ HC-SR04 ││
│  │ SEN0604  │  │          │  │ pH/TDS/EC│  │         ││
│  └──────────┘  └──────────┘  └──────────┘  └─────────┘│
│                                                           │
│  ┌──────────┐  ┌──────────┐                             │
│  │ 노드16~18│  │  노드21  │                             │
│  │          │  │          │                             │
│  │  토양    │  │액추에이터│                             │
│  │ SEN0604  │  │ 릴레이   │                             │
│  │          │  │          │                             │
│  └──────────┘  └──────────┘                             │
└───────────────────────────────────────────────────────────┘
```

### 1.3 노드 구성 (총 21개)

| 노드 번호 | 노드 유형 | MCU | 센서/역할 | Zone/Tank ID |
|----------|----------|-----|----------|-------------|
| **01~12** | 토양 전용 | Arduino Uno R4 WiFi | SEN0604 | Tank 01~12 |
| **13** | 대기+토양 통합 | Arduino Uno R4 WiFi | SHT30 + SEN0604 | Zone 13, Tank 13 |
| **14** | 대기+토양 통합 | Arduino Uno R4 WiFi | SHT30 + SEN0604 | Zone 14, Tank 14 |
| **15** | 대기+토양 통합 | Arduino Uno R4 WiFi | SHT30 + SEN0604 | Zone 15, Tank 15 |
| **16~18** | 토양 전용 | Arduino Uno R4 WiFi | SEN0604 | Tank 16~18 |
| **19** | 물탱크 | Arduino Uno R4 WiFi | DS18B20 + pH/TDS/EC | - |
| **20** | 수위 센서 | Arduino Uno R4 WiFi 또는 Wemos D1 R1 | HC-SR04 | 물탱크 |
| **21** | 액추에이터 | Arduino Uno R4 WiFi | 릴레이 제어 | - |

**총 21개 노드**: 20개 Arduino Uno R4 WiFi + 0~1개 Wemos D1 R1 (ESP8266)

### 1.4 사용 센서

| 센서 | 종류 | 수량 | 측정 항목 | 통신 방식 |
|------|------|------|----------|----------|
| **SHT30** | 대기 온습도 | 3개 | 온도, 습도 | I2C |
| **SEN0604** | 토양 4-in-1 | 18개 | 토양온도, 습도, EC, pH | RS485 Modbus RTU |
| **DS18B20** | 수온 | 1개 | 수온 | 1-Wire |
| **pH 센서** | 수질 pH | 1개 | pH | 아날로그 |
| **TDS 센서** | 수질 TDS | 1개 | TDS | 아날로그 |
| **EC 센서** | 수질 EC | 1개 | EC | 아날로그 |
| **HC-SR04** | 초음파 수위 | 1개 | 수위(거리→%) | 디지털 |

---

## 2. 시스템 요구사항

### 2.1 하드웨어 최소 사양

#### Windows 서버 노트북
- **CPU**: Intel Core i3 이상 (또는 동급)
- **RAM**: 4GB 이상 (권장: 8GB)
- **저장공간**: 10GB 이상 여유 공간
- **네트워크**: WiFi 또는 유선 랜 (인터넷 연결 필수)
- **OS**: Windows 10 (64bit) 이상 또는 Windows 11

#### Arduino 노드
- **Arduino Uno R4 WiFi** × 20개 (최대)
- **Wemos D1 R1 (ESP8266)** × 0~1개 (수위 센서용 옵션)
- **RS485 확장보드** (DFR0259 또는 호환) × 18개

### 2.2 소프트웨어 요구사항

#### Windows 서버
- **Node.js**: v16 이상 (권장: v18 LTS)
- **Node-RED**: v3.0 이상
- **Mosquitto MQTT Broker**: v2.0 이상
- **Arduino IDE**: v2.x (최신 버전)

### 2.3 네트워크 요구사항

- **고정 IP 또는 DHCP 예약**: 서버 노트북에 권장
- **포트 개방**:
  - 1880: Node-RED
  - 1883: MQTT
- **WiFi 대역**: 2.4GHz 필수 (Arduino Uno R4 WiFi는 5GHz 미지원)

---

## 3. 사전 준비사항

### 3.1 필수 정보 확인 및 기록

설치 전에 다음 정보를 미리 확인하고 메모해두세요:

```
[ ] WiFi SSID: _______________________
[ ] WiFi 비밀번호: _______________________
[ ] 서버 노트북 IP 주소: _______________________
    (확인 방법: cmd → ipconfig → IPv4 주소)
[ ] 설치 경로: C:\SPB_Data\wasabismartfarm
[ ] Arduino 보드 수량: _______개 (최대 21개)
```

### 3.2 Windows 관리자 권한 확인

1. `시작` 버튼 우클릭 → `Windows PowerShell (관리자)` 선택
2. 관리자 권한으로 실행되는지 확인 (창 제목에 "관리자" 표시)
3. 다음 명령어로 사용자 그룹 확인:
   ```powershell
   whoami /groups
   ```
4. `BUILTIN\Administrators` 그룹에 속해 있어야 함

### 3.3 폴더 구조 생성

**PowerShell 관리자 모드**에서 다음 명령어 실행:

```powershell
# 기본 폴더 구조 생성
New-Item -Path "C:\SPB_Data\wasabismartfarm" -ItemType Directory -Force
New-Item -Path "C:\SPB_Data\wasabismartfarm\scripts" -ItemType Directory -Force
New-Item -Path "C:\SPB_Data\wasabismartfarm\logs" -ItemType Directory -Force
New-Item -Path "C:\SPB_Data\wasabismartfarm\backups" -ItemType Directory -Force

# 폴더 생성 확인
Get-ChildItem "C:\SPB_Data\wasabismartfarm"
```

**예상 폴더 구조**:
```
C:\SPB_Data\wasabismartfarm\
├── flows_wasabi.json (추후 생성)
├── settings.js (추후 생성)
├── scripts\ (모니터링 스크립트)
├── logs\ (로그 파일)
└── backups\ (백업 파일)
```

---

## 4. Windows 서버 환경 구축

### 4.1 Node.js 설치

#### 4.1.1 다운로드 및 설치

1. **Node.js 공식 사이트 접속**:
   ```
   https://nodejs.org/
   ```

2. **LTS 버전 다운로드**:
   - 권장: v18.x LTS 또는 v20.x LTS
   - Windows Installer (.msi) 64-bit 다운로드

3. **설치 실행**:
   - 다운로드한 `.msi` 파일 더블클릭
   - "Next" 연속 클릭
   - "Automatically install the necessary tools..." 체크
   - "Install" 클릭

4. **설치 확인**:
   ```powershell
   node --version
   npm --version
   ```
   
   **예상 출력**:
   ```
   v18.19.0
   10.2.3
   ```

#### 4.1.2 (추가항목) npm 글로벌 경로 설정 (선택사항)

**이유**: 글로벌 모듈 권한 문제 방지

```powershell
# npm 글로벌 경로 확인
npm config get prefix

# 사용자 폴더로 변경 (권장)
npm config set prefix "$env:USERPROFILE\AppData\Roaming\npm"
```

---

## 5. Node-RED 설치 및 설정

### 5.1 Node-RED 설치

#### 5.1.1 npm을 통한 설치

**PowerShell 관리자 모드**에서 실행:

```powershell
npm install -g --unsafe-perm node-red
```

**설치 시간**: 약 5~10분

#### 5.1.2 설치 확인

```powershell
node-red --version
```

**예상 출력**:
```
3.1.0
```

### 5.2 Node-RED 초기 실행 및 설정 파일 생성

#### 5.2.1 최초 실행

**PowerShell**에서 실행 (관리자 권한 불필요):

```powershell
node-red
```

**초기 실행 출력 예시**:
```
[info] Node-RED version: v3.1.0
[info] Node.js version: v18.19.0
[info] Windows_NT 10.0.22631 x64 LE
[info] Loading palette nodes
[info] Settings file: C:\Users\사용자명\.node-red\settings.js
[info] Context store: 'default' [module=memory]
[info] User directory: C:\Users\사용자명\.node-red
[warn] Projects disabled: editorTheme.projects.enabled=false
[info] Flows file: C:\Users\사용자명\.node-red\flows.json
[info] Server now running at http://127.0.0.1:1880/
```

**주의**: 
- 최초 실행 시 기본 설정 파일 `settings.js`가 자동 생성됨
- 설치 위치: `C:\Users\사용자명\.node-red\settings.js`

#### 5.2.2 Node-RED 정지

**Ctrl + C** 2회 입력하여 Node-RED 종료

### 5.3 settings.js 수정 (Context Storage 영구 저장 설정)

#### 5.3.1 settings.js 파일 열기

**파일 경로**:
```
C:\Users\사용자명\.node-red\settings.js
```

**메모장으로 열기**:
```powershell
notepad "C:\Users\$env:USERNAME\.node-red\settings.js"
```

#### 5.3.2 Context Storage 설정 추가

**settings.js 파일에서 `contextStorage` 섹션을 찾아 다음과 같이 수정**:

```javascript
// Context Storage 설정 (약 500번째 줄 근처)
contextStorage: {
    default: {
        module: "localfilesystem"
    },
    memory: {
        module: "memory"
    }
},
```

**변경 전 (주석 처리된 상태)**:
```javascript
// contextStorage: {
//    default: {
//        module:"memory"
//    },
// },
```

**변경 후**:
```javascript
contextStorage: {
    default: {
        module: "localfilesystem"
    },
    memory: {
        module: "memory"
    }
},
```

#### 5.3.3 (추가항목) 작업 디렉토리 변경 (선택사항)

**이유**: 프로젝트 파일을 특정 경로에 저장하려면

**settings.js 파일에서 `userDir` 설정 변경** (약 50번째 줄 근처):

```javascript
// 기본값:
// userDir: 'C:\\Users\\사용자명\\.node-red',

// 변경 예시:
userDir: 'C:\\SPB_Data\\wasabismartfarm',
```

**주의**: 백슬래시(`\`)를 두 번 입력해야 함 (`\\`)

#### 5.3.4 settings.js 저장 및 종료

`Ctrl + S` → 파일 저장  
`Alt + F4` → 메모장 종료

### 5.4 Node-RED 필수 노드 설치

#### 5.4.1 Node-RED 재실행

```powershell
node-red
```

#### 5.4.2 웹 브라우저로 Node-RED 접속

**주소**: `http://localhost:1880`

#### 5.4.3 추가 노드 설치

**Node-RED 웹 UI**에서:

1. 우측 상단 `햄버거 메뉴 (≡)` 클릭
2. `Manage palette` 선택
3. `Install` 탭 선택
4. 다음 노드를 검색하여 설치:

| 노드 이름 | 설명 |
|-----------|------|
| `node-red-dashboard` | Dashboard UI 생성 |
| `node-red-node-ui-table` | 테이블 UI 위젯 |
| `node-red-contrib-influxdb` | InfluxDB 연동 (선택) |

**설치 방법**:
- 검색창에 노드 이름 입력
- 검색 결과에서 해당 노드 찾기
- `install` 버튼 클릭
- 설치 완료 후 `Done` 클릭

**설치 시간**: 각 노드당 약 1~2분

#### 5.4.4 Node-RED 재시작

설치 완료 후 Node-RED 재시작:
- `Ctrl + C` 2회 → Node-RED 종료
- `node-red` 명령어로 재실행

---

## 6. Mosquitto MQTT Broker 설치

### 6.1 Mosquitto 다운로드

#### 6.1.1 공식 사이트 접속

```
https://mosquitto.org/download/
```

#### 6.1.2 Windows Installer 다운로드

- **Windows 64-bit**: `mosquitto-X.X.X-install-windows-x64.exe`
- 권장 버전: 2.0.18 이상

### 6.2 Mosquitto 설치

#### 6.2.1 인증서 설치 (선행 필수)

**설치 중 OpenSSL 관련 오류가 발생하면**:

1. **Visual C++ Redistributable 설치**:
   ```
   https://aka.ms/vs/17/release/vc_redist.x64.exe
   ```
   - 다운로드 후 설치 실행

2. **OpenSSL 설치** (필요 시):
   ```
   https://slproweb.com/products/Win32OpenSSL.html
   ```
   - "Win64 OpenSSL" 최신 버전 다운로드
   - 설치 실행

#### 6.2.2 Mosquitto 설치 실행

1. 다운로드한 `.exe` 파일 더블클릭
2. "Next" 연속 클릭
3. "Service" 옵션 체크 (권장)
4. "Install" 클릭

**기본 설치 경로**:
```
C:\Program Files\mosquitto\
```

### 6.3 Mosquitto 설정

#### 6.3.1 설정 파일 수정

**설정 파일 경로**:
```
C:\Program Files\mosquitto\mosquitto.conf
```

**메모장으로 열기** (관리자 권한):
```powershell
Start-Process notepad "C:\Program Files\mosquitto\mosquitto.conf" -Verb RunAs
```

**설정 파일 내용 수정**:

```conf
# 기본 설정
listener 1883 0.0.0.0
allow_anonymous true

# 로그 설정
log_dest file C:\Program Files\mosquitto\mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

# 지속성 설정
persistence true
persistence_location C:\Program Files\mosquitto\data\
```

**파일 저장**: `Ctrl + S`

#### 6.3.2 데이터 폴더 생성

**PowerShell 관리자 모드**에서:

```powershell
New-Item -Path "C:\Program Files\mosquitto\data" -ItemType Directory -Force
```

### 6.4 Mosquitto 서비스 시작

#### 6.4.1 서비스 상태 확인

**PowerShell 관리자 모드**에서:

```powershell
Get-Service mosquitto
```

**예상 출력**:
```
Status   Name               DisplayName
------   ----               -----------
Stopped  mosquitto          Mosquitto Broker
```

#### 6.4.2 서비스 시작

```powershell
Start-Service mosquitto
```

#### 6.4.3 서비스 자동 시작 설정

```powershell
Set-Service -Name mosquitto -StartupType Automatic
```

#### 6.4.4 서비스 상태 재확인

```powershell
Get-Service mosquitto
```

**예상 출력**:
```
Status   Name               DisplayName
------   ----               -----------
Running  mosquitto          Mosquitto Broker
```

### 6.5 Mosquitto 연결 테스트

#### 6.5.1 Mosquitto 명령어 도구 사용

**PowerShell**에서:

```powershell
# 환경변수에 Mosquitto 경로 추가
$env:Path += ";C:\Program Files\mosquitto"

# MQTT 메시지 구독 (터미널 1)
mosquitto_sub -h localhost -t test/topic
```

**새 PowerShell 창**을 열고:

```powershell
# 환경변수에 Mosquitto 경로 추가
$env:Path += ";C:\Program Files\mosquitto"

# MQTT 메시지 발행 (터미널 2)
mosquitto_pub -h localhost -t test/topic -m "Hello MQTT"
```

**예상 결과**: 터미널 1에 "Hello MQTT" 메시지 표시

---

## 7. Arduino 개발 환경 설정

### 7.1 Arduino IDE 설치

#### 7.1.1 다운로드

**공식 사이트**:
```
https://www.arduino.cc/en/software
```

**다운로드**:
- **Arduino IDE 2.x** (최신 버전)
- Windows Installer (.exe)

#### 7.1.2 설치 실행

1. 다운로드한 `.exe` 파일 더블클릭
2. "I Agree" 클릭
3. "Next" 연속 클릭
4. "Install" 클릭

**설치 시간**: 약 5분

### 7.2 Arduino Uno R4 WiFi 보드 지원 추가

#### 7.2.1 Arduino IDE 실행

#### 7.2.2 보드 매니저 열기

`도구` → `보드` → `보드 매니저...`

#### 7.2.3 보드 패키지 설치

**검색창**에 다음 입력:
```
Arduino UNO R4
```

**검색 결과**:
- **Arduino UNO R4 Boards** by Arduino

**설치**:
- `INSTALL` 버튼 클릭
- 버전: 최신 버전 (1.1.0 이상)

**설치 시간**: 약 3~5분

#### 7.2.4 (추가항목) ESP8266 보드 지원 추가 (수위 센서용, 선택사항)

**이유**: 수위 센서 노드(노드 20)를 Wemos D1 R1 (ESP8266)으로 구성할 경우

**추가 보드 매니저 URL 설정**:

1. `파일` → `환경설정`
2. `추가적인 보드 매니저 URLs` 입력란에 다음 추가:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. `확인` 클릭

**보드 매니저에서 ESP8266 설치**:

1. `도구` → `보드` → `보드 매니저...`
2. 검색창에 `esp8266` 입력
3. **esp8266** by ESP8266 Community 선택
4. `INSTALL` 버튼 클릭
5. 버전: 최신 버전 (3.1.0 이상)

### 7.3 필수 라이브러리 설치

#### 7.3.1 라이브러리 매니저 열기

`도구` → `라이브러리 관리...` 또는 `Ctrl + Shift + I`

#### 7.3.2 라이브러리 설치 목록

다음 라이브러리를 **검색 → 설치**:

| 라이브러리 이름 | 버전 | 설명 |
|----------------|------|------|
| `PubSubClient` | 최신 | MQTT 클라이언트 |
| `ArduinoJson` | 최신 | JSON 직렬화/역직렬화 |
| `Adafruit SHT31 Library` | 최신 | SHT30 센서 (대기+토양 통합 노드용) |
| `ArduinoModbus` | 최신 | Modbus RTU 통신 (토양 센서용) |
| `ArduinoRS485` | 최신 | RS485 통신 (토양 센서용) |
| `OneWire` | 최신 | 1-Wire 통신 (물탱크 센서용) |
| `DallasTemperature` | 최신 | DS18B20 온도 센서 (물탱크 센서용) |

**설치 방법**:
1. 검색창에 라이브러리 이름 입력
2. 검색 결과에서 해당 라이브러리 선택
3. `INSTALL` 버튼 클릭
4. 의존성 라이브러리 설치 팝업이 나타나면 `INSTALL ALL` 클릭

**총 설치 시간**: 약 10분

### 7.4 Arduino 보드 연결 테스트

#### 7.4.1 Arduino Uno R4 WiFi 연결

1. USB 케이블로 Arduino 보드를 컴퓨터에 연결
2. 보드 전원 LED가 켜지는지 확인

#### 7.4.2 보드 및 포트 선택

**Arduino IDE**에서:

1. `도구` → `보드` → `Arduino UNO R4 Boards` → `Arduino UNO R4 WiFi` 선택
2. `도구` → `포트` → `COMx (Arduino UNO R4 WiFi)` 선택
   - `COMx`는 자동으로 감지됨 (예: COM3, COM4 등)

#### 7.4.3 Blink 예제 업로드 테스트

1. `파일` → `예제` → `01.Basics` → `Blink` 선택
2. `스케치` → `확인/컴파일` (체크 아이콘) 클릭
3. `스케치` → `업로드` (화살표 아이콘) 클릭

**업로드 성공 시**:
- 하단에 "업로드 완료" 메시지 표시
- Arduino 보드의 내장 LED가 1초 간격으로 깜빡임

**문제 발생 시**: [12. 문제 해결 가이드](#12-문제-해결-가이드) 참조

---

## 8. 센서 노드 펌웨어 업로드

### 8.1 프로젝트 파일 다운로드

#### 8.1.1 GitHub 리포지토리

**GitHub URL**:
```
https://github.com/phdsjw/WasabiSmartFarm
```

#### 8.1.2 다운로드 방법

**방법 1: ZIP 다운로드 (간편)**

1. GitHub 페이지 접속
2. 녹색 `Code` 버튼 클릭
3. `Download ZIP` 선택
4. 다운로드한 `WasabiSmartFarm-main.zip` 압축 해제
5. 적절한 위치에 압축 해제 (예: `C:\Projects\WasabiSmartFarm`)

**방법 2: Git Clone (개발자용)**

```powershell
# Git 설치 필요 (https://git-scm.com/)
cd C:\Projects
git clone https://github.com/phdsjw/WasabiSmartFarm.git
cd WasabiSmartFarm
```

### 8.2 노드별 펌웨어 업로드 순서

**업로드 순서** (권장):

1. 대기+토양 통합 노드 (노드 13, 14, 15) - 3개
2. 토양 전용 노드 (노드 01~12, 16~18) - 15개
3. 물탱크 센서 노드 (노드 19) - 1개
4. 수위 센서 노드 (노드 20) - 1개
5. 액추에이터 노드 (노드 21) - 1개

### 8.3 대기+토양 통합 노드 (노드 13, 14, 15)

#### 8.3.1 폴더 위치

```
WasabiSmartFarm\arduino\air_soil_combined_node\
```

#### 8.3.2 Arduino IDE에서 열기

1. Arduino IDE 실행
2. `파일` → `열기`
3. `air_soil_combined_node.ino` 파일 선택

#### 8.3.3 config.h 수정

**config.h 파일 열기** (Arduino IDE 상단 탭):

```cpp
// ============================================================
// 1. 노드 ID 설정 (중요!)
// ============================================================

// 노드 13용 (첫 번째 보드)
#define ZONE_ID "13"
#define TANK_ID "13"

// 노드 14용 (두 번째 보드) - 주석 해제하고 위 내용 주석 처리
// #define ZONE_ID "14"
// #define TANK_ID "14"

// 노드 15용 (세 번째 보드) - 주석 해제하고 위 내용 주석 처리
// #define ZONE_ID "15"
// #define TANK_ID "15"

// ============================================================
// 2. WiFi 설정
// ============================================================
#define WIFI_SSID        "YOUR_WIFI_SSID"      // WiFi SSID
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"  // WiFi 비밀번호
#define WIFI_TIMEOUT     10000                 // WiFi 연결 타임아웃 (ms)
#define WIFI_MAX_RETRY   5                     // WiFi 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000              // WiFi 재시도 간격 (ms)

// ============================================================
// 3. MQTT 설정
// ============================================================
#define MQTT_SERVER      "192.168.0.100"  // MQTT Broker IP (서버 노트북 IP)
#define MQTT_PORT        1883             // MQTT Broker 포트
#define MQTT_USER        ""               // MQTT 사용자명 (비워두면 인증 안함)
#define MQTT_PASSWORD    ""               // MQTT 비밀번호
```

**수정 항목**:
1. `WIFI_SSID`: 실제 WiFi SSID로 변경
2. `WIFI_PASSWORD`: 실제 WiFi 비밀번호로 변경
3. `MQTT_SERVER`: 서버 노트북의 실제 IP 주소로 변경
4. `ZONE_ID`, `TANK_ID`: 각 노드에 맞게 설정
   - 노드 13: `ZONE_ID "13"`, `TANK_ID "13"`
   - 노드 14: `ZONE_ID "14"`, `TANK_ID "14"`
   - 노드 15: `ZONE_ID "15"`, `TANK_ID "15"`

#### 8.3.4 컴파일 및 업로드

1. **보드 선택**: `도구` → `보드` → `Arduino UNO R4 WiFi`
2. **포트 선택**: `도구` → `포트` → `COMx (Arduino UNO R4 WiFi)`
3. **컴파일**: `스케치` → `확인/컴파일` (Ctrl + R)
   - 하단에 "컴파일 완료" 메시지 확인
4. **업로드**: `스케치` → `업로드` (Ctrl + U)
   - 하단에 "업로드 완료" 메시지 확인

#### 8.3.5 시리얼 모니터로 동작 확인

1. `도구` → `시리얼 모니터` (Ctrl + Shift + M)
2. 우측 하단 보드레이트: `115200` 선택

**예상 출력**:
```
=== Wasabi SmartFarm - Air+Soil Combined Sensor Node ===
Version: v1.0.0
Zone ID: 13
Tank ID: 13
=====================================================

[INIT] Initializing...
[SHT30] Initializing sensor...
[SHT30] Sensor initialized successfully!
[SEN0604] Initializing sensor...
[SEN0604] Sensor initialized successfully!
[MQTT] Connecting to WiFi...
[WiFi] Connecting to YOUR_WIFI_SSID...
[WiFi] Connected! IP: 192.168.0.101
[WiFi] RSSI: -45 dBm
[MQTT] Connecting to MQTT Broker...
[MQTT] Connected to MQTT Broker!
[SETUP] Initialization complete!

[SHT30] Temperature: 22.5°C, Humidity: 65.2%
[MQTT] Published air data to sensor/air/zone13/data
[SEN0604] Soil Temp: 20.1°C, Moisture: 85.5%, EC: 1250.0, pH: 6.50
[MQTT] Published soil data to sensor/soil/tank13/data
```

#### 8.3.6 반복 (노드 14, 15)

- `config.h`에서 `ZONE_ID`와 `TANK_ID`를 변경
- 다시 컴파일 및 업로드
- 노드 13, 14, 15 총 3개 보드 완료

### 8.4 토양 전용 노드 (노드 01~12, 16~18)

#### 8.4.1 폴더 위치

```
WasabiSmartFarm\arduino\soil_sensor_node\
```

#### 8.4.2 Arduino IDE에서 열기

`soil_sensor_node.ino` 파일 선택

#### 8.4.3 config.h 수정

```cpp
// ============================================================
// 1. 노드 ID 설정 (중요!)
// ============================================================

// Tank 01용 (첫 번째 토양 노드)
#define TANK_ID "01"

// 다른 노드용 - TANK_ID를 해당 번호로 변경
// Tank 02: #define TANK_ID "02"
// Tank 03: #define TANK_ID "03"
// ...
// Tank 12: #define TANK_ID "12"
// Tank 16: #define TANK_ID "16"
// Tank 17: #define TANK_ID "17"
// Tank 18: #define TANK_ID "18"

// ============================================================
// 2. WiFi 설정
// ============================================================
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// ============================================================
// 3. MQTT 설정
// ============================================================
#define MQTT_SERVER      "192.168.0.100"
#define MQTT_PORT        1883
```

#### 8.4.4 업로드 순서

**15개 보드에 순차적으로 업로드**:

1. `TANK_ID "01"` → 업로드 → 시리얼 모니터 확인
2. `TANK_ID "02"` → 업로드 → 시리얼 모니터 확인
3. ...
4. `TANK_ID "12"` → 업로드 → 시리얼 모니터 확인
5. `TANK_ID "16"` → 업로드 → 시리얼 모니터 확인
6. `TANK_ID "17"` → 업로드 → 시리얼 모니터 확인
7. `TANK_ID "18"` → 업로드 → 시리얼 모니터 확인

**작업 시간**: 15개 × 약 5분 = 약 75분 (1시간 15분)

### 8.5 물탱크 센서 노드 (노드 19)

#### 8.5.1 폴더 위치

```
WasabiSmartFarm\arduino\water_tank_sensor_node\
```

#### 8.5.2 config.h 수정

```cpp
// WiFi 설정
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// MQTT 설정
#define MQTT_SERVER      "192.168.0.100"
#define MQTT_PORT        1883

// 센서 보정값 (실제 센서에 맞게 조정 필요)
#define PH_CALIBRATION_VOLTAGE   2.5f
#define TDS_KVALUE              0.5f
#define EC_CALIBRATION_FACTOR   1.0f
```

#### 8.5.3 업로드

1. 보드 선택: `Arduino UNO R4 WiFi`
2. 포트 선택
3. 컴파일 및 업로드

### 8.6 수위 센서 노드 (노드 20)

**두 가지 옵션**:
- **옵션 A**: Arduino Uno R4 WiFi 사용 (권장)
- **옵션 B**: Wemos D1 R1 (ESP8266) 사용 (비용 절감)

#### 8.6.1 옵션 A: Arduino Uno R4 WiFi 사용

**폴더 위치**:
```
WasabiSmartFarm\arduino\water_level_sensor_node_r4\
```

**config.h 수정**:
```cpp
// WiFi 설정
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// MQTT 설정
#define MQTT_SERVER      "192.168.0.100"
#define MQTT_PORT        1883

// 물탱크 설정
#define TANK_HEIGHT_CM      100.0f  // 물탱크 실제 높이 (cm)
#define SENSOR_OFFSET_CM    5.0f    // 센서 설치 오프셋 (cm)
```

**핀 연결**:
| HC-SR04 핀 | Arduino R4 핀 |
|-----------|--------------|
| VCC | 5V |
| GND | GND |
| Trig | D7 |
| Echo | D8 |

#### 8.6.2 옵션 B: Wemos D1 R1 (ESP8266) 사용

**폴더 위치**:
```
WasabiSmartFarm\arduino\water_level_sensor_node\
```

**보드 선택**:
- `도구` → `보드` → `ESP8266 Boards` → `LOLIN(WEMOS) D1 R1`
- `도구` → `Upload Speed` → `115200`
- `도구` → `CPU Frequency` → `80 MHz`

**config.h 수정**:
```cpp
// WiFi 설정
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// MQTT 설정
#define MQTT_SERVER      "192.168.0.100"
#define MQTT_PORT        1883

// 물탱크 설정
#define TANK_HEIGHT_CM      100.0f
#define SENSOR_OFFSET_CM    5.0f
```

**핀 연결**:
| HC-SR04 핀 | Wemos D1 R1 핀 |
|-----------|---------------|
| VCC | 5V |
| GND | GND |
| Trig | D1 |
| Echo | D2 |

### 8.7 액추에이터 노드 (노드 21)

#### 8.7.1 폴더 위치

```
WasabiSmartFarm\arduino\actuator_node\
```

#### 8.7.2 config.h 수정

```cpp
// WiFi 설정
#define WIFI_SSID        "YOUR_WIFI_SSID"
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"

// MQTT 설정
#define MQTT_SERVER      "192.168.0.100"
#define MQTT_PORT        1883

// 릴레이 핀 설정 (실제 하드웨어에 맞게 조정)
#define RELAY_PUMP_IN    2   // 급수 펌프 릴레이
#define RELAY_PUMP_OUT   3   // 퇴수 펌프 릴레이
#define RELAY_FAN        4   // 환풍기 릴레이
#define RELAY_LED        5   // LED 조명 릴레이
```

#### 8.7.3 업로드

1. 보드 선택: `Arduino UNO R4 WiFi`
2. 포트 선택
3. 컴파일 및 업로드

### 8.8 전체 노드 업로드 완료 체크리스트

```
[ ] 노드 13 - 대기+토양 통합 (Zone 13, Tank 13)
[ ] 노드 14 - 대기+토양 통합 (Zone 14, Tank 14)
[ ] 노드 15 - 대기+토양 통합 (Zone 15, Tank 15)
[ ] 노드 01 - 토양 전용 (Tank 01)
[ ] 노드 02 - 토양 전용 (Tank 02)
[ ] 노드 03 - 토양 전용 (Tank 03)
[ ] 노드 04 - 토양 전용 (Tank 04)
[ ] 노드 05 - 토양 전용 (Tank 05)
[ ] 노드 06 - 토양 전용 (Tank 06)
[ ] 노드 07 - 토양 전용 (Tank 07)
[ ] 노드 08 - 토양 전용 (Tank 08)
[ ] 노드 09 - 토양 전용 (Tank 09)
[ ] 노드 10 - 토양 전용 (Tank 10)
[ ] 노드 11 - 토양 전용 (Tank 11)
[ ] 노드 12 - 토양 전용 (Tank 12)
[ ] 노드 16 - 토양 전용 (Tank 16)
[ ] 노드 17 - 토양 전용 (Tank 17)
[ ] 노드 18 - 토양 전용 (Tank 18)
[ ] 노드 19 - 물탱크 센서
[ ] 노드 20 - 수위 센서
[ ] 노드 21 - 액추에이터
```

**총 21개 노드 업로드 완료**

---

## 9. Node-RED Flow 배포

### 9.1 Flow 파일 복사

#### 9.1.1 GitHub에서 Flow 파일 찾기

**GitHub 경로**:
```
WasabiSmartFarm\nodered\flows_wasabi_03.json
```

#### 9.1.2 Flow 파일 복사

**방법 1: 파일 탐색기 사용**

1. 다운로드한 프로젝트 폴더의 `nodered\flows_wasabi_03.json` 파일을 찾습니다.
2. 파일을 복사합니다.
3. Node-RED 작업 디렉토리로 이동:
   ```
   C:\Users\사용자명\.node-red\
   ```
   또는
   ```
   C:\SPB_Data\wasabismartfarm\
   ```
4. `flows.json` 파일명으로 붙여넣기 (기존 파일이 있으면 백업 후 덮어쓰기)

**방법 2: PowerShell 명령어 사용**

```powershell
# 기존 파일 백업
Copy-Item "C:\Users\$env:USERNAME\.node-red\flows.json" `
          "C:\Users\$env:USERNAME\.node-red\flows.json.backup" `
          -ErrorAction SilentlyContinue

# 새 파일 복사
Copy-Item "C:\Projects\WasabiSmartFarm\nodered\flows_wasabi_03.json" `
          "C:\Users\$env:USERNAME\.node-red\flows.json"
```

### 9.2 Node-RED 재시작

#### 9.2.1 Node-RED 종료

**실행 중인 Node-RED 터미널**에서:
- `Ctrl + C` 2회 입력

#### 9.2.2 Node-RED 재실행

```powershell
node-red
```

#### 9.2.3 웹 UI 접속

**브라우저**에서:
```
http://localhost:1880
```

### 9.3 Flow 확인 및 배포

#### 9.3.1 Flow 탭 확인

**Node-RED 웹 UI**에서 다음 탭들이 보이는지 확인:

1. **제어 및 알림** - 하트비트 모니터링 및 제어 버튼
2. **센서 데이터 수신** - MQTT 구독 및 데이터 처리
3. **Dashboard** - 사용자 인터페이스 정의

#### 9.3.2 MQTT 연결 설정 확인

1. 아무 MQTT 노드 더블클릭
2. `Server` 옆의 연필 아이콘 클릭
3. `Connection` 탭:
   - **Server**: `localhost` 또는 `127.0.0.1`
   - **Port**: `1883`
4. `Update` 클릭
5. `Done` 클릭

#### 9.3.3 배포 (Deploy)

**우측 상단** `Deploy` 버튼 클릭

**예상 결과**:
- "Successfully deployed" 메시지 표시
- 모든 노드 연결선이 초록색으로 변경 (MQTT 연결 성공)

### 9.4 Dashboard 접속

#### 9.4.1 Dashboard URL

```
http://localhost:1880/ui
```

#### 9.4.2 Dashboard 탭 확인

1. **대시보드 홈** - 시스템 개요
2. **센서 모니터링** - 실시간 센서 데이터
3. **제어 및 알림** - 하트비트 모니터링, 제어 버튼
4. **급수 제어** - 급수 펌프 자동/수동 제어
5. **데이터 기록** - 센서 데이터 이력

---

## 10. 시스템 자동 재시작 설정

### 10.1 Node-RED 자동 재시작 설정 (Windows Task Scheduler)

#### 10.1.1 Task Scheduler 열기

1. `시작` 버튼 클릭
2. "작업 스케줄러" 검색
3. `작업 스케줄러` 앱 실행

#### 10.1.2 새 작업 만들기

1. 우측 `작업 만들기...` 클릭

**일반 탭**:
- **이름**: `Node-RED Auto Start`
- **설명**: `Node-RED 자동 실행 및 재시작`
- **보안 옵션**: `사용자의 로그온 여부에 관계없이 실행` 선택
- **가장 높은 수준의 권한으로 실행** 체크

**트리거 탭**:
1. `새로 만들기...` 클릭
2. **작업 시작**: `컴퓨터를 시작할 때`
3. `확인` 클릭

**동작 탭**:
1. `새로 만들기...` 클릭
2. **동작**: `프로그램 시작`
3. **프로그램/스크립트**: `node`
4. **인수 추가**: `C:\Users\사용자명\AppData\Roaming\npm\node_modules\node-red\red.js`
   - 실제 경로 확인:
     ```powershell
     where.exe node
     Get-Command node-red | Select-Object -ExpandProperty Definition
     ```
5. `확인` 클릭

**조건 탭**:
- 모든 체크박스 해제 (전원 연결 조건 제거)

**설정 탭**:
- **작업이 실패한 경우 다시 시작 간격**: `1분`
- **다시 시작 시도 간격**: `3회`

`확인` 클릭

#### 10.1.3 (추가항목) Node-RED 모니터링 스크립트 작성 (고급)

**이유**: Node-RED가 중단되면 자동으로 재시작

**스크립트 파일 생성**:

**PowerShell**에서:
```powershell
$scriptPath = "C:\SPB_Data\wasabismartfarm\scripts\monitor_nodered.ps1"

$scriptContent = @'
# Node-RED 모니터링 및 자동 재시작 스크립트
$logPath = "C:\SPB_Data\wasabismartfarm\logs\nodered_monitor.log"

while ($true) {
    $process = Get-Process node -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -like "*node-red*" }
    
    if (-not $process) {
        $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        Add-Content -Path $logPath -Value "[$timestamp] Node-RED not running. Starting..."
        
        Start-Process -FilePath "node" -ArgumentList "C:\Users\$env:USERNAME\AppData\Roaming\npm\node_modules\node-red\red.js" -WindowStyle Hidden
        
        Start-Sleep -Seconds 10
    }
    
    Start-Sleep -Seconds 60
}
'@

New-Item -Path $scriptPath -ItemType File -Force -Value $scriptContent
```

**Task Scheduler에 모니터링 작업 추가**:

1. `작업 만들기...`
2. **이름**: `Node-RED Monitor`
3. **트리거**: `컴퓨터를 시작할 때`
4. **동작**: `프로그램 시작`
   - **프로그램/스크립트**: `powershell.exe`
   - **인수 추가**: `-ExecutionPolicy Bypass -File "C:\SPB_Data\wasabismartfarm\scripts\monitor_nodered.ps1"`
5. `확인` 클릭

### 10.2 Mosquitto 자동 재시작 확인

Mosquitto는 이미 Windows 서비스로 설치되어 자동 재시작이 설정되어 있습니다.

**서비스 상태 확인**:

```powershell
Get-Service mosquitto | Select-Object Name, Status, StartType
```

**예상 출력**:
```
Name      Status  StartType
----      ------  ---------
mosquitto Running Automatic
```

**자동 시작이 안되어 있으면**:
```powershell
Set-Service -Name mosquitto -StartupType Automatic
```

---

## 11. 최종 검증 및 테스트

### 11.1 시스템 전체 가동 확인

#### 11.1.1 서버 서비스 확인

**PowerShell**에서:

```powershell
# Mosquitto 상태 확인
Get-Service mosquitto

# Node-RED 프로세스 확인
Get-Process node | Where-Object { $_.CommandLine -like "*node-red*" }
```

**예상 결과**:
- Mosquitto: `Running`
- Node-RED: `node` 프로세스 1개 이상

#### 11.1.2 네트워크 포트 확인

```powershell
# 포트 사용 확인
netstat -an | Select-String "1880|1883"
```

**예상 출력**:
```
TCP    0.0.0.0:1880           0.0.0.0:0              LISTENING
TCP    0.0.0.0:1883           0.0.0.0:0              LISTENING
```

### 11.2 Arduino 노드 연결 확인

#### 11.2.1 MQTT 메시지 구독

**새 PowerShell 창**에서:

```powershell
# Mosquitto 경로 추가
$env:Path += ";C:\Program Files\mosquitto"

# 모든 센서 하트비트 구독
mosquitto_sub -h localhost -t "sensor/+/+/heartbeat" -t "actuator/heartbeat"
```

**예상 결과**: 60초마다 각 노드의 하트비트 메시지 수신

```json
{"zone_id":"13","tank_id":"13","node_type":"combined","uptime":123456,"wifi_rssi":-45,"timestamp":123456}
{"zone_id":"14","tank_id":"14","node_type":"combined","uptime":123456,"wifi_rssi":-48,"timestamp":123456}
...
```

#### 11.2.2 센서 데이터 구독

```powershell
# 대기 센서 데이터 구독
mosquitto_sub -h localhost -t "sensor/air/+/data"

# 토양 센서 데이터 구독
mosquitto_sub -h localhost -t "sensor/soil/+/data"

# 수위 센서 데이터 구독
mosquitto_sub -h localhost -t "sensor/water_level/data"
```

**예상 결과**: 10초마다 센서 데이터 수신

### 11.3 Dashboard UI 테스트

#### 11.3.1 Dashboard 접속

```
http://localhost:1880/ui
```

#### 11.3.2 하트비트 모니터링 확인

1. `제어 및 알림` 탭 클릭
2. 21개 노드 카드 확인:
   - 연결된 노드: 녹색 램프 (온라인)
   - 연결 안된 노드: 회색 램프 (연결 안됨)
   - 2분 이상 하트비트 없음: 빨간색 램프 (타임아웃)

#### 11.3.3 센서 데이터 확인

1. `센서 모니터링` 탭 클릭
2. 대기 센서 (Zone 13, 14, 15): 온도, 습도 표시
3. 토양 센서 (Tank 01~18): 토양 온도, 습도, EC, pH 표시
4. 물탱크 센서: 수온, pH, TDS, EC 표시
5. 수위 센서: 거리(cm), 수위(%) 표시

#### 11.3.4 제어 기능 테스트 (선택)

1. `급수 제어` 탭 클릭
2. **수동 제어 모드** 선택
3. **급수 펌프 ON** 버튼 클릭
4. 액추에이터 노드가 MQTT 메시지를 받아 릴레이 제어
5. **급수 펌프 OFF** 버튼 클릭

### 11.4 7일 연속 운영 테스트 (장기 안정성)

#### 11.4.1 테스트 목적

- 24/7/365 연속 운영 안정성 확인
- 자동 재시작 기능 검증
- WiFi/MQTT 재연결 로직 검증

#### 11.4.2 모니터링 항목

```
[ ] Day 1: 시스템 정상 가동
[ ] Day 2: WiFi 재연결 테스트 (공유기 재부팅)
[ ] Day 3: MQTT Broker 재시작 테스트
[ ] Day 4: Node-RED 재시작 테스트
[ ] Day 5: Arduino 노드 전원 재부팅 테스트
[ ] Day 6: 장시간 데이터 수집 확인
[ ] Day 7: 전체 시스템 안정성 평가
```

#### 11.4.3 로그 확인

**Mosquitto 로그**:
```powershell
Get-Content "C:\Program Files\mosquitto\mosquitto.log" -Tail 50
```

**Node-RED 로그** (콘솔 출력):
- Node-RED 실행 터미널 확인

---

## 12. 문제 해결 가이드

### 12.1 Arduino 업로드 오류

#### 12.1.1 "Port not found" 오류

**원인**: USB 드라이버 미설치 또는 포트 인식 안됨

**해결 방법**:
1. Arduino 보드 USB 케이블 재연결
2. 장치 관리자 확인:
   - `시작` → "장치 관리자" 검색
   - `포트 (COM & LPT)` 확장
   - `Arduino UNO R4 WiFi (COMx)` 확인
3. 드라이버 재설치:
   - 장치 우클릭 → `드라이버 업데이트`
   - `자동으로 드라이버 소프트웨어 검색`

#### 12.1.2 "Compilation error" 오류

**원인**: 라이브러리 미설치 또는 버전 불일치

**해결 방법**:
1. 오류 메시지에서 누락된 라이브러리 확인
2. `도구` → `라이브러리 관리`에서 해당 라이브러리 설치
3. Arduino IDE 재시작

#### 12.1.3 "Upload error" 오류

**원인**: 보드 선택 오류 또는 시리얼 포트 사용 중

**해결 방법**:
1. 올바른 보드 선택: `Arduino UNO R4 WiFi`
2. 시리얼 모니터 닫기
3. Arduino IDE 재시작
4. 보드 리셋 버튼 누른 상태에서 업로드 시도

### 12.2 WiFi 연결 오류

#### 12.2.1 "WiFi connection failed" 메시지

**원인**: SSID/비밀번호 오류 또는 2.4GHz WiFi 아님

**해결 방법**:
1. `config.h`에서 SSID/비밀번호 재확인
2. WiFi 대역 확인: 반드시 2.4GHz 사용
3. 공유기 재부팅
4. Arduino 보드 전원 재부팅

#### 12.2.2 "WiFi signal weak" 메시지

**원인**: 공유기 거리가 멀거나 장애물 많음

**해결 방법**:
1. Arduino 보드를 공유기에 가까이 배치
2. 공유기 안테나 방향 조정
3. WiFi 신호 증폭기 사용

### 12.3 MQTT 연결 오류

#### 12.3.1 "MQTT connection failed" 메시지

**원인**: MQTT Broker IP 오류 또는 Broker 미실행

**해결 방법**:
1. Mosquitto 서비스 상태 확인:
   ```powershell
   Get-Service mosquitto
   ```
2. 서버 IP 주소 재확인:
   ```powershell
   ipconfig
   ```
3. `config.h`에서 `MQTT_SERVER` IP 수정
4. 방화벽 포트 1883 개방 확인

#### 12.3.2 "MQTT authentication failed" 메시지

**원인**: MQTT 사용자명/비밀번호 불일치

**해결 방법**:
1. `mosquitto.conf`에서 `allow_anonymous true` 설정 확인
2. 사용자명/비밀번호 사용 안함으로 설정:
   ```cpp
   #define MQTT_USER        ""
   #define MQTT_PASSWORD    ""
   ```

### 12.4 Node-RED 오류

#### 12.4.1 "Node-RED is not starting" 오류

**원인**: 포트 1880 이미 사용 중

**해결 방법**:
1. 기존 Node-RED 프로세스 종료:
   ```powershell
   Get-Process node | Where-Object { $_.CommandLine -like "*node-red*" } | Stop-Process
   ```
2. Node-RED 재실행

#### 12.4.2 "MQTT connection error" in Node-RED

**원인**: MQTT Broker 연결 설정 오류

**해결 방법**:
1. Node-RED 웹 UI에서 MQTT 노드 더블클릭
2. Server 설정: `localhost` 또는 `127.0.0.1`
3. Port: `1883`
4. `Deploy` 클릭

### 12.5 센서 읽기 오류

#### 12.5.1 SHT30 센서 읽기 실패

**원인**: I2C 연결 불량 또는 센서 불량

**해결 방법**:
1. I2C 연결 재확인: SDA (A4), SCL (A5), VCC (5V), GND
2. I2C 주소 스캐너 실행:
   ```cpp
   // Wire.h 사용하여 I2C 주소 스캔
   ```
3. 센서 교체

#### 12.5.2 SEN0604 센서 읽기 실패

**원인**: RS485 연결 불량 또는 Modbus 설정 오류

**해결 방법**:
1. RS485 연결 재확인: A, B, TX, RX, DE/RE
2. 보드레이트 확인: `4800 bps`
3. Slave ID 확인: `1` (기본값)
4. 터미네이션 저항 확인

---

## 13. 유지보수 가이드

### 13.1 일일 점검 항목

```
[ ] Node-RED Dashboard에서 21개 노드 하트비트 확인
[ ] 센서 데이터 정상 수신 확인
[ ] 제어 기능 정상 작동 확인
```

### 13.2 주간 점검 항목

```
[ ] Mosquitto 로그 확인
[ ] Node-RED 로그 확인
[ ] 센서 보정 필요 여부 확인
[ ] WiFi 신호 강도 확인
```

### 13.3 월간 점검 항목

```
[ ] Arduino 펌웨어 업데이트 확인
[ ] Node-RED 버전 업데이트 확인
[ ] 시스템 백업 수행
[ ] 센서 청소 및 유지보수
```

### 13.4 백업 및 복구

#### 13.4.1 Node-RED Flow 백업

```powershell
# 백업
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
Copy-Item "C:\Users\$env:USERNAME\.node-red\flows.json" `
          "C:\SPB_Data\wasabismartfarm\backups\flows_$timestamp.json"
```

#### 13.4.2 설정 파일 백업

```powershell
# settings.js 백업
Copy-Item "C:\Users\$env:USERNAME\.node-red\settings.js" `
          "C:\SPB_Data\wasabismartfarm\backups\settings_$timestamp.js"

# mosquitto.conf 백업
Copy-Item "C:\Program Files\mosquitto\mosquitto.conf" `
          "C:\SPB_Data\wasabismartfarm\backups\mosquitto_$timestamp.conf"
```

#### 13.4.3 복구

```powershell
# Flow 복구
Copy-Item "C:\SPB_Data\wasabismartfarm\backups\flows_20250127_120000.json" `
          "C:\Users\$env:USERNAME\.node-red\flows.json"

# Node-RED 재시작
```

### 13.5 로그 관리

#### 13.5.1 로그 파일 위치

- **Mosquitto**: `C:\Program Files\mosquitto\mosquitto.log`
- **Node-RED**: 콘솔 출력 (리다이렉트 필요)
- **Arduino**: 시리얼 모니터 (로깅 기능 없음)

#### 13.5.2 로그 파일 크기 관리

```powershell
# 로그 파일 크기 확인
Get-Item "C:\Program Files\mosquitto\mosquitto.log" | Select-Object Length

# 로그 파일 비우기 (주의: 기존 로그 삭제됨)
Clear-Content "C:\Program Files\mosquitto\mosquitto.log"
```

---

## 부록 A: 참고 자료

### A.1 공식 문서

- **Arduino Uno R4 WiFi**: https://docs.arduino.cc/hardware/uno-r4-wifi
- **Node-RED**: https://nodered.org/docs/
- **Mosquitto**: https://mosquitto.org/documentation/
- **SHT30 Datasheet**: https://www.sensirion.com/sht30
- **SEN0604 Wiki**: https://wiki.dfrobot.com/SEN0604

### A.2 프로젝트 GitHub

- **GitHub Repository**: https://github.com/phdsjw/WasabiSmartFarm
- **Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues

### A.3 관련 문서

- `SENSOR_SPECIFICATION.md` - 센서 명세서
- `SENSOR_NODE_REORGANIZATION.md` - 센서 노드 재구성 변경사항
- `nodered/NODE_RED_UPDATE_GUIDE.md` - Node-RED 수정 가이드
- `arduino/air_soil_combined_node/README.md` - 대기+토양 통합 노드 가이드
- `arduino/water_level_sensor_node/README.md` - 수위 센서 노드 가이드 (ESP8266)
- `arduino/water_level_sensor_node_r4/README.md` - 수위 센서 노드 가이드 (Arduino R4)

---

## 부록 B: MQTT 토픽 목록

### B.1 센서 데이터 토픽

| 토픽 | 설명 | 주기 |
|------|------|------|
| `sensor/air/zone13/data` | Zone 13 대기 센서 데이터 | 10초 |
| `sensor/air/zone14/data` | Zone 14 대기 센서 데이터 | 10초 |
| `sensor/air/zone15/data` | Zone 15 대기 센서 데이터 | 10초 |
| `sensor/soil/tank01/data` ~ `sensor/soil/tank18/data` | 토양 센서 데이터 (18개) | 10초 |
| `sensor/water_tank/data` | 물탱크 센서 데이터 | 10초 |
| `sensor/water_level/data` | 수위 센서 데이터 | 3초 |

### B.2 하트비트 토픽

| 토픽 | 설명 | 주기 |
|------|------|------|
| `sensor/combined/zone13/heartbeat` | Zone 13 통합 노드 하트비트 | 60초 |
| `sensor/combined/zone14/heartbeat` | Zone 14 통합 노드 하트비트 | 60초 |
| `sensor/combined/zone15/heartbeat` | Zone 15 통합 노드 하트비트 | 60초 |
| `sensor/soil/tank01/heartbeat` ~ `sensor/soil/tank18/heartbeat` | 토양 노드 하트비트 (18개) | 60초 |
| `sensor/water_tank/heartbeat` | 물탱크 노드 하트비트 | 60초 |
| `sensor/water_level/heartbeat` | 수위 노드 하트비트 | 60초 |
| `actuator/heartbeat` | 액추에이터 노드 하트비트 | 60초 |

### B.3 제어 토픽

| 토픽 | 설명 | 방향 |
|------|------|------|
| `actuator/control/pump_in` | 급수 펌프 제어 | Node-RED → Arduino |
| `actuator/control/pump_out` | 퇴수 펌프 제어 | Node-RED → Arduino |
| `actuator/control/fan` | 환풍기 제어 | Node-RED → Arduino |
| `actuator/control/led` | LED 조명 제어 | Node-RED → Arduino |
| `actuator/state` | 액추에이터 상태 | Arduino → Node-RED |

---

## 부록 C: 제어 로직

### C.1 급수 제어 로직

**조건** (OR 조건 - 하나라도 만족하면 작동):

```
토양 습도 <= 95%  OR
토양 EC >= 5.0 μS/cm  OR
토양 온도 >= 22°C
```

**동작**:
- 4분/시간 작동 (매 시간 0분~4분)
- 18개 솔레노이드 밸브 순차 개방

### C.2 냉각수 모터 제어 로직

**조건**:
```
대기 온도 >= 22°C
```

**동작**:
- 냉각수 펌프 ON

### C.3 온수 모터 제어 로직

**조건**:
```
대기 온도 <= 18°C
```

**동작**:
- 온수 펌프 ON

### C.4 LED 보광등 제어 로직

**조건**:
```
2시간마다 10분 OFF
```

**동작**:
- 2시간 ON → 10분 OFF 반복

### C.5 퇴수 펌프 제어 로직

**조건**:
```
수위 >= 80%
```

**동작**:
- 퇴수 펌프 자동 작동
- 수위 < 80%가 되면 자동 정지

---

## 작성자

**서준원**

## 버전 이력

- **v2.0.0** (2025-12-27):
  - 센서 노드 재구성 반영
  - 대기+토양 통합 노드 (노드 13, 14, 15) 추가
  - 수위 센서 노드 (노드 20) 추가
  - Arduino Uno R4 WiFi 및 Wemos D1 R1 지원
  - 전체 설정 매뉴얼 통합

- **v1.0.0** (2025-12-23):
  - 초기 버전
  - Windows 서버 환경 구축
  - Node-RED 및 Mosquitto 설치
  - Arduino 개발 환경 설정

---

**완료 체크리스트**:

```
[ ] Windows 서버 환경 구축 완료
[ ] Node-RED 설치 및 설정 완료
[ ] Mosquitto MQTT Broker 설치 완료
[ ] Arduino 개발 환경 설정 완료
[ ] 센서 노드 펌웨어 업로드 완료 (21개)
[ ] Node-RED Flow 배포 완료
[ ] 시스템 자동 재시작 설정 완료
[ ] 최종 검증 및 테스트 완료
[ ] 7일 연속 운영 테스트 완료
```

**이 문서로 Wasabi Smart Farm 전체 시스템을 처음부터 끝까지 설정할 수 있습니다.**

---

**END OF DOCUMENT**
