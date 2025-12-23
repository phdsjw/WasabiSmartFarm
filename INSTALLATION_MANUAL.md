# 🌱 Wasabi Smart Farm 설치 매뉴얼

**버전**: v1.0.0  
**작성일**: 2025-12-23  
**대상**: Windows 노트북 신규 설치  
**예상 소요 시간**: 약 3~4시간

---

## 📋 목차

1. [시스템 요구사항](#1-시스템-요구사항)
2. [사전 준비사항](#2-사전-준비사항)
3. [소프트웨어 설치](#3-소프트웨어-설치)
4. [Node-RED 설정](#4-node-red-설정)
5. [Mosquitto MQTT Broker 설치](#5-mosquitto-mqtt-broker-설치)
6. [방화벽 및 포트 설정](#6-방화벽-및-포트-설정)
7. [Arduino 개발 환경 설정](#7-arduino-개발-환경-설정)
8. [프로젝트 파일 다운로드 및 배포](#8-프로젝트-파일-다운로드-및-배포)
9. [Phase 1 안정화 작업](#9-phase-1-안정화-작업)
10. [최종 검증 및 테스트](#10-최종-검증-및-테스트)
11. [문제 해결 (Troubleshooting)](#11-문제-해결-troubleshooting)
12. [유지보수 가이드](#12-유지보수-가이드)

---

## 1. 시스템 요구사항

### 하드웨어 최소 사양
- **CPU**: Intel Core i3 이상 (또는 동급)
- **RAM**: 4GB 이상 (권장 8GB)
- **저장공간**: 10GB 이상 여유 공간
- **네트워크**: WiFi 또는 유선 랜 (인터넷 연결 필수)

### 소프트웨어 요구사항
- **운영체제**: Windows 10 (64bit) 이상 또는 Windows 11
- **관리자 권한**: 설치 과정에서 필요
- **안티바이러스**: 일시적으로 비활성화 필요 (방화벽 설정 시)

### 네트워크 요구사항
- **고정 IP** 또는 **DHCP 예약**: 서버 노트북에 권장
- **포트 개방**: 1880 (Node-RED), 1883 (MQTT)
- **Arduino WiFi 연결**: 2.4GHz WiFi 필수 (5GHz 불가)

---

## 2. 사전 준비사항

### 2.1 필수 정보 확인

설치 전에 다음 정보를 미리 확인하고 메모해두세요:

```
[ ] WiFi SSID: _______________________
[ ] WiFi 비밀번호: _______________________
[ ] 노트북 IP 주소: _______________________
[ ] 설치 경로: C:\SPB_Data\wasabismartfarm
```

### 2.2 Windows 계정 권한 확인

1. `시작` 버튼 우클릭 → `Windows PowerShell (관리자)` 선택
2. 관리자 권한으로 실행되는지 확인 (창 제목에 "관리자" 표시)
3. 다음 명령어로 사용자 그룹 확인:
   ```powershell
   whoami /groups
   ```
4. `BUILTIN\Administrators` 그룹에 속해 있어야 함

### 2.3 방화벽 설정 확인

1. `제어판` → `Windows Defender 방화벽`
2. 현재 프로필 확인 (일반적으로 "개인" 또는 "공용")
3. 잠시 후 포트 개방 시 사용할 정보 확인

### 2.4 폴더 구조 생성

1. `파일 탐색기` 열기 (`Win + E`)
2. `C:\` 드라이브로 이동
3. 다음 폴더 생성:

```
C:\
└── SPB_Data\
    └── wasabismartfarm\
        ├── flows_wasabi.json (추후 생성)
        ├── settings.js (추후 생성)
        ├── scripts\ (추후 생성)
        └── logs\ (추후 생성)
```

**PowerShell 명령어로 한번에 생성:**
```powershell
New-Item -Path "C:\SPB_Data\wasabismartfarm\scripts" -ItemType Directory -Force
New-Item -Path "C:\SPB_Data\wasabismartfarm\logs" -ItemType Directory -Force
```

---

## 3. 소프트웨어 설치

### 3.1 Node.js 설치

Node-RED를 실행하려면 Node.js가 필요합니다.

#### 다운로드
1. 웹 브라우저에서 https://nodejs.org/ko 접속
2. **LTS (Long Term Support) 버전** 다운로드 (권장: v20.x.x)
   - 예: `node-v20.11.0-x64.msi`
3. 다운로드 완료 후 설치 파일 실행

#### 설치 과정
1. 설치 마법사 시작 → `Next` 클릭
2. 라이선스 동의 → `I accept...` 체크 → `Next`
3. 설치 경로 확인 → 기본값 유지 (`C:\Program Files\nodejs\`) → `Next`
4. 설치 구성 요소 선택:
   - ✅ Node.js runtime
   - ✅ npm package manager
   - ✅ Add to PATH
   - 모두 선택된 상태로 `Next`
5. `Install` 클릭 → 설치 진행 (약 2~3분)
6. `Finish` 클릭

#### 설치 확인
1. `시작` → `PowerShell` 실행
2. 다음 명령어 입력:
   ```powershell
   node --version
   ```
   출력 예시: `v20.11.0`
3. npm 버전 확인:
   ```powershell
   npm --version
   ```
   출력 예시: `10.2.4`

> ⚠️ **주의**: 버전이 출력되지 않으면 PowerShell을 재시작하거나 PC를 재부팅하세요.

---

### 3.2 Node-RED 설치

#### 글로벌 설치
1. `PowerShell (관리자)` 실행
2. 다음 명령어 입력:
   ```powershell
   npm install -g --unsafe-perm node-red
   ```
3. 설치 진행 (약 5~10분 소요)
   - 경고 메시지는 무시 가능 (빨간색 `WARN` 메시지)
   - 오류 메시지가 있으면 중단하고 [문제 해결](#11-문제-해결-troubleshooting) 참조

#### 설치 확인
```powershell
node-red --version
```
출력 예시: `3.1.0`

#### 첫 실행 (테스트)
```powershell
node-red
```

출력 확인:
```
Welcome to Node-RED
===================
...
[info] Server now running at http://127.0.0.1:1880/
[info] Starting flows
[info] Started flows
```

**웹 브라우저 확인:**
1. Chrome 또는 Edge 브라우저 열기
2. 주소창에 `http://localhost:1880` 입력
3. Node-RED 편집기 화면이 나타나면 성공

**종료:**
- PowerShell에서 `Ctrl + C` 누르기
- `y` 입력 후 Enter (종료 확인)

---

### 3.3 Node-RED 필수 라이브러리 설치

Node-RED를 종료한 상태에서 다음 라이브러리를 설치합니다.

#### 설치 명령어
```powershell
cd C:\Users\%USERNAME%\.node-red
npm install node-red-dashboard
npm install node-red-contrib-influxdb
npm install node-red-node-google
```

> ℹ️ **설명**:
> - `node-red-dashboard`: 웹 UI 대시보드
> - `node-red-contrib-influxdb`: 시계열 데이터베이스 연동
> - `node-red-node-google`: Google Sheets 연동

#### 설치 확인
```powershell
dir node_modules | findstr "node-red-dashboard"
dir node_modules | findstr "influxdb"
dir node_modules | findstr "google"
```

각 라이브러리 이름이 출력되면 성공.

---

## 4. Node-RED 설정

### 4.1 설정 파일 생성 및 수정

#### 기본 설정 파일 확인
1. Node-RED를 한 번 실행하면 자동으로 설정 파일이 생성됩니다:
   ```
   C:\Users\[사용자이름]\.node-red\settings.js
   ```

2. 파일 탐색기에서 해당 경로로 이동:
   ```
   %USERPROFILE%\.node-red
   ```

3. `settings.js` 파일을 찾아서 메모장으로 열기:
   - 파일 우클릭 → `연결 프로그램` → `메모장` 선택

#### Context 영구 저장 설정 추가

**Phase 1-1: Context Storage 설정**

`settings.js` 파일에서 `contextStorage` 부분을 찾아 수정합니다:

1. 파일에서 `contextStorage` 검색 (Ctrl + F)
2. 기존 내용이 주석 처리되어 있으면 아래 내용으로 교체:

```javascript
    contextStorage: {
        default: {
            module: "memory"
        },
        file: {
            module: "localfilesystem",
            config: {
                dir: "C:\\SPB_Data\\wasabismartfarm\\context",
                flushInterval: 30
            }
        }
    },
```

3. `C:\SPB_Data\wasabismartfarm\context` 폴더 생성:
   ```powershell
   New-Item -Path "C:\SPB_Data\wasabismartfarm\context" -ItemType Directory -Force
   ```

4. `settings.js` 파일 저장 후 닫기

> ℹ️ **설명**:
> - `default`: 메모리 기반 (재시작 시 초기화)
> - `file`: 파일 시스템 영구 저장 (재시작해도 유지)
> - `flushInterval: 30`: 30초마다 디스크에 기록

---

### 4.2 사용자 정의 경로 설정

Node-RED가 프로젝트 파일을 사용하도록 설정합니다.

#### settings.js 추가 수정

`settings.js` 파일을 다시 열어서 다음 항목을 확인/수정:

1. **사용자 디렉토리 설정** (파일 상단 부근):
   ```javascript
   userDir: 'C:\\SPB_Data\\wasabismartfarm\\',
   ```

2. **플로우 파일 이름 설정**:
   ```javascript
   flowFile: 'flows_wasabi.json',
   ```

3. **자동 저장 설정**:
   ```javascript
   flowFilePretty: true,
   ```

4. **에디터 테마 설정** (선택 사항):
   ```javascript
   editorTheme: {
       projects: {
           enabled: false
       }
   },
   ```

#### 최종 settings.js 저장 위치

**원본 위치**:
```
C:\Users\[사용자이름]\.node-red\settings.js
```

**백업 생성**:
```powershell
Copy-Item "C:\Users\$env:USERNAME\.node-red\settings.js" `
          "C:\SPB_Data\wasabismartfarm\settings.js.backup"
```

---

### 4.3 Node-RED 실행 배치 파일 생성

매번 명령어를 입력하지 않고 더블클릭으로 실행할 수 있도록 배치 파일을 만듭니다.

#### 배치 파일 생성

1. 메모장 열기
2. 다음 내용 입력:

```batch
@echo off
title Wasabi Smart Farm - Node-RED Server
echo ========================================
echo  Wasabi Smart Farm Node-RED Server
echo ========================================
echo Starting Node-RED...
echo.
cd /d C:\SPB_Data\wasabismartfarm
node-red --userDir "C:\SPB_Data\wasabismartfarm"
pause
```

3. 다른 이름으로 저장:
   - 파일명: `start_nodered.bat`
   - 저장 위치: `C:\SPB_Data\wasabismartfarm\`
   - 파일 형식: `모든 파일 (*.*)`
   - 인코딩: `ANSI`

#### 실행 및 확인

1. `C:\SPB_Data\wasabismartfarm\` 폴더에서 `start_nodered.bat` 더블클릭
2. CMD 창이 열리며 Node-RED가 시작됨
3. 브라우저에서 `http://localhost:1880` 접속 확인
4. **종료하지 말고** 다음 단계 진행

---

## 5. Mosquitto MQTT Broker 설치

### 5.1 Mosquitto 다운로드 및 설치

#### 다운로드
1. 웹 브라우저에서 https://mosquitto.org/download/ 접속
2. `Windows` 섹션에서 **64-bit installer** 다운로드
   - 예: `mosquitto-2.0.18-install-windows-x64.exe`
3. 다운로드 완료 후 설치 파일 실행

#### 설치 과정
1. 보안 경고 → `예` 클릭
2. 설치 마법사 시작 → `Next`
3. 라이선스 동의 → `I Agree`
4. 설치 구성 요소 선택:
   - ✅ Broker (mosquitto)
   - ✅ Service
   - ✅ Client tools (mosquitto_pub, mosquitto_sub)
   - 모두 선택 → `Next`
5. 설치 경로 확인:
   - 기본값: `C:\Program Files\mosquitto\`
   - `Install` 클릭
6. 설치 완료 → `Finish`

#### 설치 확인
1. `서비스` 확인:
   - `Win + R` → `services.msc` 입력 → Enter
   - `Mosquitto Broker` 서비스 찾기
   - 상태: `실행 중` 확인

2. 명령어로 확인:
   ```powershell
   cd "C:\Program Files\mosquitto"
   .\mosquitto.exe --help
   ```

---

### 5.2 Mosquitto 설정 파일 수정

#### 설정 파일 편집
1. 경로 이동:
   ```
   C:\Program Files\mosquitto\
   ```

2. `mosquitto.conf` 파일 찾기
   - 없으면 새로 생성

3. 메모장(관리자 권한)으로 열기:
   - `시작` → `메모장` 우클릭 → `관리자 권한으로 실행`
   - `파일` → `열기` → `mosquitto.conf` 선택

4. 다음 내용 추가/수정:

```conf
# Mosquitto MQTT Broker Configuration
# Wasabi Smart Farm Project

# 리스너 설정
listener 1883 0.0.0.0

# 익명 접속 허용 (개발 단계)
allow_anonymous true

# 로그 설정
log_dest file C:/Program Files/mosquitto/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information
log_timestamp true

# 지속성 설정
persistence true
persistence_location C:/Program Files/mosquitto/data/

# 자동 저장 주기 (초)
autosave_interval 300

# 최대 클라이언트 수
max_connections 100
```

5. 저장 후 닫기

#### 로그 및 데이터 폴더 생성
```powershell
New-Item -Path "C:\Program Files\mosquitto\data" -ItemType Directory -Force
```

#### Mosquitto 서비스 재시작
1. `서비스` 열기 (`services.msc`)
2. `Mosquitto Broker` 우클릭 → `다시 시작`
3. 상태가 `실행 중`으로 변경되는지 확인

---

### 5.3 MQTT 연결 테스트

#### 테스트 준비
1. PowerShell 2개 창 열기
   - 창 1: Publisher (발행자)
   - 창 2: Subscriber (구독자)

#### Subscriber 실행 (창 2)
```powershell
cd "C:\Program Files\mosquitto"
.\mosquitto_sub.exe -h localhost -t "test/topic" -v
```
> 대기 상태로 유지

#### Publisher 실행 (창 1)
```powershell
cd "C:\Program Files\mosquitto"
.\mosquitto_pub.exe -h localhost -t "test/topic" -m "Hello Wasabi Smart Farm"
```

#### 결과 확인
- **창 2 (Subscriber)**에 다음 메시지 출력:
  ```
  test/topic Hello Wasabi Smart Farm
  ```

✅ 정상 작동 확인!

---

## 6. 방화벽 및 포트 설정

### 6.1 Windows 방화벽 규칙 추가

#### Node-RED 포트 (1880) 개방

1. `Windows Defender 방화벽` 열기:
   - `제어판` → `시스템 및 보안` → `Windows Defender 방화벽`

2. 왼쪽 메뉴에서 `고급 설정` 클릭

3. **인바운드 규칙** 생성:
   - 왼쪽: `인바운드 규칙` 클릭
   - 오른쪽: `새 규칙...` 클릭

4. 규칙 구성:
   - **규칙 종류**: `포트` 선택 → `다음`
   - **프로토콜 및 포트**: 
     - `TCP` 선택
     - `특정 로컬 포트`: `1880` 입력
     - `다음`
   - **작업**: `연결 허용` → `다음`
   - **프로필**: 모두 선택 (도메인, 개인, 공용) → `다음`
   - **이름**: `Node-RED (Port 1880)` 입력
   - **설명**: `Wasabi Smart Farm Node-RED 웹 서버`
   - `마침`

#### MQTT 포트 (1883) 개방

위와 동일한 과정으로 반복:
- 포트: `1883`
- 이름: `Mosquitto MQTT (Port 1883)`
- 설명: `Wasabi Smart Farm MQTT Broker`

---

### 6.2 아웃바운드 규칙 추가 (선택 사항)

외부로 나가는 연결도 명시적으로 허용하려면:

1. `아웃바운드 규칙` 클릭
2. 위와 동일한 방법으로 포트 1880, 1883 규칙 추가

---

### 6.3 방화벽 테스트

#### 로컬 테스트
```powershell
Test-NetConnection -ComputerName localhost -Port 1880
Test-NetConnection -ComputerName localhost -Port 1883
```

출력 확인:
```
TcpTestSucceeded : True
```

#### 외부 접속 테스트 (같은 WiFi의 다른 기기)

1. 노트북 IP 주소 확인:
   ```powershell
   ipconfig
   ```
   출력 예시: `192.168.0.100`

2. 스마트폰이나 다른 PC에서 접속:
   ```
   http://192.168.0.100:1880
   ```

3. Node-RED 편집기가 보이면 성공!

---

## 7. Arduino 개발 환경 설정

### 7.1 Arduino IDE 2.x 설치

#### 다운로드
1. https://www.arduino.cc/en/software 접속
2. `Arduino IDE 2.3.2` (최신 버전) 다운로드
   - Windows 10/11: `Windows Win 10 and newer, 64 bits` 선택
   - 예: `arduino-ide_2.3.2_Windows_64bit.exe`

#### 설치 과정
1. 설치 파일 실행
2. 라이선스 동의 → `I Agree`
3. 설치 옵션:
   - ✅ Install USB Driver
   - ✅ Associate .ino files
   - `Next` → `Install`
4. 설치 완료 (약 5분)

---

### 7.2 Arduino Uno R4 WiFi 보드 추가

#### 보드 매니저 설정
1. Arduino IDE 실행
2. 메뉴: `Tools` → `Board` → `Boards Manager...`
3. 검색창에 `Arduino UNO R4` 입력
4. `Arduino UNO R4 Boards by Arduino` 찾기
5. `Install` 클릭 (버전: 최신)
6. 설치 완료 후 `Close`

#### 보드 선택
1. 메뉴: `Tools` → `Board` → `Arduino UNO R4 Boards` → `Arduino UNO R4 WiFi`
2. Arduino를 USB로 연결
3. 메뉴: `Tools` → `Port` → `COM3` (또는 자동 인식된 포트) 선택

---

### 7.3 필수 라이브러리 설치

#### 라이브러리 매니저 열기
- 메뉴: `Tools` → `Manage Libraries...`
- 또는 단축키: `Ctrl + Shift + I`

#### 각 라이브러리 설치 (검색 → Install)

| 라이브러리 이름 | 버전 | 용도 |
|----------------|------|------|
| **WiFiS3** | 최신 | Arduino UNO R4 WiFi 통신 |
| **PubSubClient** | 2.8.0 이상 | MQTT 클라이언트 |
| **ArduinoJson** | 6.21.0 이상 | JSON 데이터 처리 |
| **ArduinoModbus** | 1.0.9 이상 | Modbus RTU 통신 |
| **ArduinoRS485** | 1.0.5 이상 | RS485 통신 |
| **OneWire** | 2.3.7 이상 | 1-Wire 프로토콜 (DS18B20) |
| **DallasTemperature** | 3.9.0 이상 | DS18B20 온도 센서 |
| **Adafruit SHT31 Library** | 2.2.0 이상 | SHT30/SHT31 센서 |

**설치 방법** (각 라이브러리마다 반복):
1. 검색창에 라이브러리 이름 입력
2. 정확한 라이브러리 찾기 (작성자 확인)
3. `Install` 클릭
4. 의존성 라이브러리 설치 팝업 → `Install all`

#### 설치 확인
- 메뉴: `Sketch` → `Include Library`
- 설치된 라이브러리 목록에서 위 항목들이 보이는지 확인

---

### 7.4 Arduino 컴파일 테스트

#### 빈 스케치로 테스트
1. 새 스케치: `File` → `New Sketch`
2. 다음 코드 입력:

```cpp
void setup() {
  Serial.begin(115200);
  Serial.println("Wasabi Smart Farm - Test OK");
}

void loop() {
  delay(1000);
}
```

3. 컴파일: `Sketch` → `Verify/Compile` (또는 `Ctrl + R`)
4. 하단에 `Done compiling` 메시지 확인
5. ✅ 성공!

---

## 8. 프로젝트 파일 다운로드 및 배포

### 8.1 GitHub에서 프로젝트 클론

#### Git 설치 (없는 경우)
1. https://git-scm.com/download/win 접속
2. `64-bit Git for Windows Setup` 다운로드
3. 설치 (기본 옵션 유지)

#### 프로젝트 클론
1. PowerShell 열기
2. 작업 디렉토리로 이동:
   ```powershell
   cd C:\SPB_Data
   ```

3. Git 클론 실행:
   ```powershell
   git clone https://github.com/phdsjw/WasabiSmartFarm.git
   ```

4. 클론 완료 확인:
   ```powershell
   dir WasabiSmartFarm
   ```

---

### 8.2 Node-RED 플로우 파일 배포

#### 최신 플로우 파일 복사
```powershell
Copy-Item "C:\SPB_Data\WasabiSmartFarm\nodered\flows_wasabi_03.json" `
          "C:\SPB_Data\wasabismartfarm\flows_wasabi.json"
```

#### 파일 확인
```powershell
Test-Path "C:\SPB_Data\wasabismartfarm\flows_wasabi.json"
```
출력: `True`

---

### 8.3 PowerShell 모니터링 스크립트 배포

#### 스크립트 복사
```powershell
Copy-Item "C:\SPB_Data\WasabiSmartFarm\nodered\scripts\monitor_nodered.ps1" `
          "C:\SPB_Data\wasabismartfarm\scripts\"
Copy-Item "C:\SPB_Data\WasabiSmartFarm\nodered\scripts\monitor_mosquitto.ps1" `
          "C:\SPB_Data\wasabismartfarm\scripts\"
```

#### 실행 정책 설정
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```
- 경고 메시지 → `Y` 입력

#### 스크립트 테스트
```powershell
cd C:\SPB_Data\wasabismartfarm\scripts
.\monitor_nodered.ps1
```

출력 확인:
```
[YYYY-MM-DD HH:MM:SS] Node-RED 프로세스 정상 실행 중 (PID: xxxx)
```

---

### 8.4 Arduino 펌웨어 준비

#### 펌웨어 위치 확인
```
C:\SPB_Data\WasabiSmartFarm\arduino\
├── wasabi_controller\       ← 메인 컨트롤러
├── actuator_node\            ← 액추에이터 노드
├── soil_sensor_node\         ← 토양 센서 노드
├── air_sensor_node\          ← 대기 센서 노드
└── water_tank_sensor_node\   ← 물탱크 센서 노드
```

#### WiFi 및 MQTT 설정 파일 수정

각 Arduino 프로젝트의 `config.h` 파일을 수정해야 합니다.

**예시: wasabi_controller/config.h**

1. Arduino IDE에서 열기:
   ```
   File → Open → C:\SPB_Data\WasabiSmartFarm\arduino\wasabi_controller\wasabi_controller.ino
   ```

2. `config.h` 탭 선택

3. WiFi 설정 수정:
   ```cpp
   // WiFi 설정
   const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ← 여기 수정
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ← 여기 수정
   ```

4. MQTT 서버 설정 수정:
   ```cpp
   // MQTT 설정
   const char* MQTT_SERVER = "192.168.0.100";  // ← 노트북 IP 주소
   const int MQTT_PORT = 1883;
   const char* MQTT_CLIENT_ID = "wasabi_controller_01";
   ```

5. 저장: `Ctrl + S`

> ⚠️ **중요**: 모든 Arduino 노드 (actuator_node, soil_sensor_node 등)에서 동일하게 수정!

---

## 9. Phase 1 안정화 작업

Phase 1의 4가지 작업을 순서대로 진행합니다.

### 9.1 Phase 1-1: Context 영구 저장 ✅

이미 [4.1 절](#41-설정-파일-생성-및-수정)에서 완료했습니다.

**확인 사항:**
- ✅ `settings.js`에 `contextStorage` 설정 추가
- ✅ `C:\SPB_Data\wasabismartfarm\context\` 폴더 생성

---

### 9.2 Phase 1-2: WiFi 재연결 로직 개선 ✅

Arduino 펌웨어에 이미 포함되어 있습니다.

**확인 사항:**
- ✅ `config.h`에 `WIFI_TIMEOUT`, `WIFI_MAX_RETRY` 설정
- ✅ `mqtt_handler.cpp`에 재시도 로직 구현

**별도 작업 없음** - 펌웨어를 그대로 업로드하면 됩니다.

---

### 9.3 Phase 1-3: Heartbeat 모니터링 ✅

Node-RED 플로우에 이미 포함되어 있습니다 (`flows_wasabi_03.json`).

**확인 사항:**
- ✅ 21개 노드 모니터링 (액추에이터 1, 물탱크 1, 시스템 1, 대기 1, 토양 18)
- ✅ Dashboard UI에 상태 표시
- ✅ 2분 타임아웃 설정

**별도 작업 없음** - Node-RED 시작 시 자동 적용됩니다.

---

### 9.4 Phase 1-4: Task Scheduler 자동 재시작

#### 작업 스케줄러 열기
1. `Win + R` → `taskschd.msc` 입력 → Enter
2. `작업 스케줄러 라이브러리` 선택

#### Node-RED 자동 재시작 작업 생성

**1. 기본 작업 만들기**
- 오른쪽: `기본 작업 만들기...` 클릭

**2. 이름 및 설명**
- 이름: `Monitor Node-RED`
- 설명: `Wasabi Smart Farm Node-RED 프로세스 모니터링 및 자동 재시작`
- `다음`

**3. 트리거 설정**
- `컴퓨터를 시작할 때` 선택
- `다음`

**4. 작업 설정**
- `프로그램 시작` 선택
- `다음`

**5. 프로그램 설정**
- **프로그램/스크립트**: `powershell.exe`
- **인수 추가**:
  ```
  -ExecutionPolicy Bypass -WindowStyle Hidden -File "C:\SPB_Data\wasabismartfarm\scripts\monitor_nodered.ps1"
  ```
- **시작 위치**: `C:\SPB_Data\wasabismartfarm\scripts`
- `다음`

**6. 고급 설정**
- `마침` 클릭
- 작업 속성 창에서 추가 설정:

**7. 일반 탭**
- ✅ `가장 높은 수준의 권한으로 실행`
- ✅ `사용자의 로그온 여부에 관계없이 실행`

**8. 트리거 탭**
- 트리거 더블클릭 → `편집`
- ✅ `사용`
- `고급 설정`:
  - ✅ `다음 시간 간격으로 작업 반복`: `1분`
  - `기간`: `무한정`
- `확인`

**9. 조건 탭**
- ❌ `컴퓨터의 AC 전원이 켜져 있을 때만 작업 시작` (체크 해제)
- ✅ `작업을 실행하기 위해 절전 모드 종료`

**10. 설정 탭**
- ✅ `요청 시 작업 실행 허용`
- ✅ `작업 실패 시 다시 시작 간격`: `1분`
- `확인`

#### Mosquitto 자동 재시작 작업 생성

위와 동일한 방법으로 반복:

- **이름**: `Monitor Mosquitto`
- **스크립트**: `monitor_mosquitto.ps1`
- 나머지 설정 동일

#### 작업 테스트

1. 작업 스케줄러에서 `Monitor Node-RED` 우클릭
2. `실행` 클릭
3. 로그 확인:
   ```powershell
   Get-Content "C:\SPB_Data\wasabismartfarm\logs\monitor_nodered.log" -Tail 10
   ```

출력 예시:
```
[2025-12-23 10:30:00] Node-RED 프로세스 정상 실행 중 (PID: 12345)
```

---

## 10. 최종 검증 및 테스트

### 10.1 Node-RED 시작 및 Dashboard 확인

#### Node-RED 시작
1. `C:\SPB_Data\wasabismartfarm\start_nodered.bat` 더블클릭
2. CMD 창에서 로그 확인:
   ```
   [info] Server now running at http://127.0.0.1:1880/
   [info] Started flows
   ```

#### Dashboard 접속
1. 브라우저에서 `http://localhost:1880/ui` 접속
2. 대시보드 탭 확인:
   - **실시간 모니터링**
   - **토양 센서 트렌드**
   - **토양 센서 히트맵**
   - **환경 센서 트렌드**
   - **제어 및 알림** ← Heartbeat 모니터링

#### Heartbeat 상태 확인
1. `제어 및 알림` 탭 선택
2. `시스템 상태 모니터링` 섹션 확인
3. 21개 노드 카드 표시:
   - 액추에이터 노드
   - 물탱크 센서
   - 시스템 컨트롤러
   - 대기센서 Zone1
   - 토양센서 01 ~ 18

4. 초기 상태:
   - 모든 노드: ⚪ **회색** (연결 안됨)
   - 이유: Arduino가 아직 연결되지 않음

---

### 10.2 Arduino 펌웨어 업로드

#### wasabi_controller 업로드

1. Arduino IDE에서 열기:
   ```
   C:\SPB_Data\WasabiSmartFarm\arduino\wasabi_controller\wasabi_controller.ino
   ```

2. `config.h` 확인:
   - WiFi SSID/Password 올바른지 확인
   - MQTT Server IP 올바른지 확인

3. Arduino Uno R4 WiFi를 USB로 연결

4. 포트 선택: `Tools` → `Port` → `COM3` (자동 인식)

5. 업로드: `Sketch` → `Upload` (또는 `Ctrl + U`)

6. 업로드 완료 대기 (약 30초)

7. 시리얼 모니터 확인:
   - `Tools` → `Serial Monitor` (또는 `Ctrl + Shift + M`)
   - 보드레이트: `115200`
   - 출력 확인:
     ```
     ==========================================
     Wasabi Smart Farm Controller
     Version: 1.0.0 - Step 1
     ==========================================
     ...
     WiFi 연결 중...
     WiFi 연결 성공!
     IP: 192.168.0.xxx
     MQTT 연결 중...
     MQTT 연결 성공!
     ```

#### 다른 노드 업로드 (필요 시)

동일한 방법으로:
- `actuator_node.ino`
- `soil_sensor_node.ino` (18개)
- `air_sensor_node.ino`
- `water_tank_sensor_node.ino`

각 노드마다 `config.h`의 `MQTT_CLIENT_ID`를 고유하게 설정!

---

### 10.3 MQTT 통신 확인

#### mosquitto_sub로 메시지 확인

1. PowerShell 열기
2. 다음 명령어 실행:
   ```powershell
   cd "C:\Program Files\mosquitto"
   .\mosquitto_sub.exe -h localhost -t "sensor/#" -v
   ```

3. Arduino에서 전송하는 센서 데이터 확인:
   ```
   sensor/wasabi_controller/env {"airTemp":22.5,"airHumidity":65.0,...}
   sensor/wasabi_controller/heartbeat {"status":"alive","uptime":12345}
   ```

4. `Ctrl + C`로 종료

---

### 10.4 Dashboard Heartbeat 상태 확인

1. 브라우저에서 Dashboard 새로고침
2. `제어 및 알림` → `시스템 상태 모니터링`
3. 연결된 노드 확인:
   - 시스템 컨트롤러: 🟢 **녹색** (온라인)
   - 다른 노드: ⚪ **회색** (아직 연결 안됨)

4. 1분 대기 후 상태 업데이트 확인

5. 2분 이상 Heartbeat가 없으면:
   - 🔴 **빨간색** (타임아웃)
   - Toast 알림 표시

✅ 정상 작동!

---

### 10.5 자동 재시작 테스트

#### Node-RED 프로세스 강제 종료
1. 작업 관리자 열기 (`Ctrl + Shift + Esc`)
2. `세부 정보` 탭
3. `node.exe` 찾기
4. 우클릭 → `작업 끝내기`

#### 자동 재시작 확인
1. 1분 대기
2. 작업 관리자에서 `node.exe` 다시 나타나는지 확인
3. 브라우저에서 `http://localhost:1880/ui` 재접속
4. Dashboard 정상 작동 확인

✅ 자동 재시작 성공!

---

## 11. 문제 해결 (Troubleshooting)

### 11.1 Node.js 설치 오류

**문제**: `node --version` 명령어가 작동하지 않음

**해결책**:
1. PowerShell 재시작
2. 환경 변수 확인:
   ```powershell
   $env:Path
   ```
   출력에 `C:\Program Files\nodejs\` 포함되어 있는지 확인

3. 없으면 수동 추가:
   - `시스템 속성` → `환경 변수` → `Path` 편집 → 추가

---

### 11.2 Node-RED 실행 오류

**문제**: `node-red` 명령어 실행 시 오류

**해결책 1**: npm 캐시 정리
```powershell
npm cache clean --force
npm install -g --unsafe-perm node-red
```

**해결책 2**: 관리자 권한으로 재설치
```powershell
# PowerShell (관리자)
npm uninstall -g node-red
npm install -g --unsafe-perm node-red
```

---

### 11.3 Mosquitto 서비스 시작 실패

**문제**: `services.msc`에서 Mosquitto가 시작되지 않음

**해결책**:
1. 로그 파일 확인:
   ```
   C:\Program Files\mosquitto\mosquitto.log
   ```

2. 설정 파일 문법 오류 확인:
   ```powershell
   cd "C:\Program Files\mosquitto"
   .\mosquitto.exe -c mosquitto.conf -v
   ```

3. 포트 충돌 확인:
   ```powershell
   netstat -ano | findstr :1883
   ```
   다른 프로세스가 1883 포트를 사용 중이면 종료

---

### 11.4 Arduino 컴파일 오류

**문제**: 라이브러리 관련 오류

**해결책**:
1. 라이브러리 재설치
2. Arduino IDE 재시작
3. 보드 매니저 업데이트:
   - `Tools` → `Board` → `Boards Manager`
   - `Arduino UNO R4 Boards` 업데이트

---

### 11.5 WiFi 연결 실패

**문제**: Arduino가 WiFi에 연결되지 않음

**해결책**:
1. WiFi 2.4GHz 대역 확인 (5GHz 불가)
2. `config.h`에서 SSID/Password 오타 확인
3. 시리얼 모니터로 오류 메시지 확인
4. WiFi 라우터 재시작

---

### 11.6 MQTT 연결 실패

**문제**: Arduino에서 MQTT 연결 실패

**해결책**:
1. Mosquitto 서비스 실행 중인지 확인
2. 방화벽 1883 포트 개방 확인
3. `config.h`에서 MQTT_SERVER IP 주소 확인
4. mosquitto.conf에서 `allow_anonymous true` 설정 확인

---

### 11.7 Dashboard에 Heartbeat 표시 안됨

**문제**: 녹색 불이 켜지지 않음

**해결책**:
1. Node-RED 플로우 확인:
   - `http://localhost:1880` 접속
   - `MQTT in` 노드가 연결되어 있는지 확인

2. MQTT 토픽 구독 확인:
   ```powershell
   cd "C:\Program Files\mosquitto"
   .\mosquitto_sub.exe -h localhost -t "+/heartbeat" -v
   ```

3. Arduino 시리얼 모니터에서 Heartbeat 발송 확인

---

## 12. 유지보수 가이드

### 12.1 정기 점검 (주 1회)

#### 체크리스트
```
[ ] Node-RED 서비스 정상 실행 중
[ ] Mosquitto 서비스 정상 실행 중
[ ] Dashboard 접속 가능
[ ] 21개 노드 Heartbeat 정상 (녹색)
[ ] 로그 파일 크기 확인 (100MB 이하)
[ ] 디스크 여유 공간 확인 (10GB 이상)
```

#### 로그 확인
```powershell
# Node-RED 재시작 로그
Get-Content "C:\SPB_Data\wasabismartfarm\logs\monitor_nodered.log" -Tail 20

# Mosquitto 재시작 로그
Get-Content "C:\SPB_Data\wasabismartfarm\logs\monitor_mosquitto.log" -Tail 20

# Mosquitto MQTT 로그
Get-Content "C:\Program Files\mosquitto\mosquitto.log" -Tail 50
```

---

### 12.2 백업 (월 1회)

#### 백업 대상
1. Node-RED 플로우 파일:
   ```powershell
   Copy-Item "C:\SPB_Data\wasabismartfarm\flows_wasabi.json" `
             "C:\SPB_Data\wasabismartfarm\flows_wasabi_backup_$(Get-Date -Format 'yyyyMMdd').json"
   ```

2. 설정 파일:
   ```powershell
   Copy-Item "C:\Users\$env:USERNAME\.node-red\settings.js" `
             "C:\SPB_Data\wasabismartfarm\settings_backup_$(Get-Date -Format 'yyyyMMdd').js"
   ```

3. Context 데이터:
   ```powershell
   Copy-Item -Recurse "C:\SPB_Data\wasabismartfarm\context" `
             "C:\SPB_Data\wasabismartfarm\context_backup_$(Get-Date -Format 'yyyyMMdd')"
   ```

---

### 12.3 업데이트

#### Node-RED 업데이트
```powershell
npm update -g node-red
```

#### Node-RED 라이브러리 업데이트
```powershell
cd C:\Users\%USERNAME%\.node-red
npm update
```

#### Arduino 라이브러리 업데이트
- Arduino IDE → `Tools` → `Manage Libraries...`
- 업데이트 가능한 라이브러리 확인 후 `Update` 클릭

---

### 12.4 로그 관리

#### 로그 파일 정리 (월 1회)

```powershell
# 30일 이상 된 로그 파일 삭제
Get-ChildItem "C:\SPB_Data\wasabismartfarm\logs\*.log" | 
    Where-Object { $_.LastWriteTime -lt (Get-Date).AddDays(-30) } | 
    Remove-Item -Force
```

#### Mosquitto 로그 로테이션

`mosquitto.conf`에 추가:
```conf
log_dest file C:/Program Files/mosquitto/mosquitto.log
max_log_size 10485760
```
(10MB 초과 시 자동 로테이션)

---

### 12.5 성능 모니터링

#### 시스템 리소스 확인
```powershell
# CPU 사용률
Get-Counter '\Processor(_Total)\% Processor Time'

# 메모리 사용률
Get-Counter '\Memory\Available MBytes'

# Node-RED 프로세스 메모리
Get-Process node | Select-Object Name, CPU, @{Name="Memory(MB)";Expression={[math]::Round($_.WorkingSet64 / 1MB, 2)}}
```

#### 권장 사양
- Node-RED 메모리: 500MB 이하
- CPU 사용률: 평균 10% 이하
- 디스크 여유: 10GB 이상

---

## 📊 체크리스트

설치 완료 후 다음 항목을 확인하세요:

### 소프트웨어 설치
```
[ ] Node.js 설치 완료 (v20.x.x)
[ ] Node-RED 설치 완료 (v3.x.x)
[ ] Node-RED 라이브러리 설치 (dashboard, influxdb, google)
[ ] Mosquitto MQTT Broker 설치 완료
[ ] Arduino IDE 2.x 설치 완료
[ ] Arduino 라이브러리 8개 설치 완료
[ ] Git 설치 완료
```

### 설정 파일
```
[ ] settings.js Context Storage 설정
[ ] mosquitto.conf 설정
[ ] flows_wasabi.json 배포
[ ] PowerShell 스크립트 배포
```

### 방화벽 및 네트워크
```
[ ] Node-RED 포트 1880 개방
[ ] MQTT 포트 1883 개방
[ ] 외부 접속 테스트 완료
```

### Phase 1 안정화
```
[ ] Phase 1-1: Context 영구 저장 ✅
[ ] Phase 1-2: WiFi 재연결 로직 ✅
[ ] Phase 1-3: Heartbeat 모니터링 ✅
[ ] Phase 1-4: Task Scheduler 자동 재시작 ✅
```

### Arduino 펌웨어
```
[ ] config.h WiFi 설정
[ ] config.h MQTT 설정
[ ] wasabi_controller 업로드 완료
[ ] actuator_node 업로드 완료 (필요 시)
[ ] 시리얼 모니터 정상 출력 확인
```

### 최종 검증
```
[ ] Node-RED Dashboard 접속 가능
[ ] Heartbeat 모니터링 정상 작동
[ ] MQTT 메시지 송수신 확인
[ ] 자동 재시작 테스트 통과
[ ] 7일 연속 가동 테스트 (선택)
```

---

## 🎉 설치 완료!

축하합니다! Wasabi Smart Farm 시스템 설치가 완료되었습니다.

### 다음 단계

1. **7일 안정화 테스트**
   - 24/7 연속 가동
   - Heartbeat 모니터링 확인
   - 자동 재시작 동작 확인

2. **센서 캘리브레이션**
   - pH 센서 교정
   - TDS/EC 센서 교정
   - 온도 센서 검증

3. **제어 로직 테스트**
   - 자동 관수 테스트
   - 퇴수 테스트
   - 비상 정지 테스트

4. **Phase 2 고도화** (선택)
   - Arduino Watchdog 타이머
   - 센서 데이터 검증
   - 알림 시스템 강화

---

## 📚 참고 문서

프로젝트 저장소의 다음 문서를 참조하세요:

- `README.md`: 프로젝트 개요
- `nodered/HEARTBEAT_MONITORING_GUIDE.md`: Heartbeat 모니터링 상세 가이드
- `nodered/HEARTBEAT_UI_DESIGN.md`: Dashboard UI 디자인 가이드
- `nodered/TASK_SCHEDULER_AUTO_RESTART_GUIDE.md`: 자동 재시작 상세 가이드
- `nodered/CONTEXT_STORAGE_SETUP_GUIDE.md`: Context Storage 설정 가이드
- `arduino/WIFI_RECONNECT_IMPROVEMENT.md`: WiFi 재연결 로직 가이드

---

## 📧 지원

문제가 발생하면 다음 방법으로 지원받으세요:

- **GitHub Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues
- **프로젝트 관리자**: 서준원

---

**Made with 💚 for Wasabi Smart Farm**

**Document Version**: v1.0.0  
**Last Updated**: 2025-12-23
