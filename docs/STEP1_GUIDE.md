# 📘 Step 1: 기초 통신 인프라 구축 및 센서 데이터 수집

## 🎯 목표

Arduino Uno R4 WiFi와 Node-RED 간의 MQTT 통신을 구축하고, 센서 데이터를 실시간으로 수집하여 Dashboard에 표시합니다.

---

## 📋 체크리스트

### Arduino 작업
- [ ] WiFi 연결 설정
- [ ] MQTT Broker 연결
- [ ] SHT30 (대기 온습도) 센서 연결 및 테스트
- [ ] DS18B20 (수온) 센서 연결 및 테스트
- [ ] Analog 센서 (pH, TDS, EC) 연결 및 테스트
- [ ] Modbus RTU 토양 센서 (SEN0604) 연결 및 테스트
- [ ] 수위 센서 연결 및 테스트
- [ ] 센서 데이터 MQTT로 전송 확인

### Node-RED 작업
- [ ] Mosquitto MQTT Broker 설치 및 실행
- [ ] Node-RED 설치 및 실행
- [ ] Node-RED Dashboard 설치
- [ ] Flow 임포트 (`flows_step1.json`)
- [ ] Dashboard에서 센서 데이터 표시 확인

---

## 🔧 설치 및 설정

### 1. Arduino 환경 설정

#### 1.1 Arduino IDE 설치
1. [Arduino IDE 2.x](https://www.arduino.cc/en/software) 다운로드 및 설치
2. Arduino Uno R4 WiFi 보드 패키지 설치
   - `Tools` → `Board Manager` → "Arduino UNO R4 WiFi" 검색 및 설치

#### 1.2 필수 라이브러리 설치
Arduino IDE에서 `Tools` → `Manage Libraries`로 이동하여 다음 라이브러리를 설치하세요:

| 라이브러리 | 버전 | 용도 |
|----------|------|------|
| `WiFiS3` | 최신 | WiFi 연결 (R4 전용) |
| `PubSubClient` | 2.8+ | MQTT 통신 |
| `ArduinoModbus` | 1.0+ | Modbus RTU 프로토콜 |
| `ArduinoRS485` | 1.0+ | RS485 통신 |
| `OneWire` | 2.3+ | 1-Wire 프로토콜 (DS18B20) |
| `DallasTemperature` | 3.9+ | DS18B20 온도 센서 |
| `Adafruit_SHT31` | 2.2+ | SHT30 온습도 센서 |
| `ArduinoJson` | 6.21+ | JSON 직렬화 |

#### 1.3 config.h 수정
`arduino/wasabi_controller/config.h` 파일을 열고 다음 항목을 수정하세요:

```cpp
// WiFi 설정
#define WIFI_SSID        "YOUR_WIFI_SSID"      // 실제 WiFi SSID
#define WIFI_PASSWORD    "YOUR_WIFI_PASSWORD"  // 실제 WiFi 비밀번호

// MQTT 설정
#define MQTT_SERVER      "192.168.0.100"       // Node-RED 서버 IP
#define MQTT_PORT        1883
```

### 2. Node-RED 환경 설정

#### 2.1 Mosquitto MQTT Broker 설치

**Raspberry Pi / Linux:**
```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

**Windows:**
1. [Mosquitto 다운로드](https://mosquitto.org/download/)
2. 설치 후 서비스 시작: `net start mosquitto`

#### 2.2 Node-RED 설치

**Raspberry Pi / Linux:**
```bash
sudo apt install -y nodejs npm
sudo npm install -g --unsafe-perm node-red
```

**Windows:**
```powershell
npm install -g --unsafe-perm node-red
```

#### 2.3 Node-RED Dashboard 설치
```bash
cd ~/.node-red
npm install node-red-dashboard
```

#### 2.4 Node-RED 실행
```bash
node-red
```

브라우저에서 `http://localhost:1880` 접속

---

## 📥 Flow 임포트

1. Node-RED 웹 인터페이스에서 우측 상단 메뉴 (≡) 클릭
2. `Import` 선택
3. `nodered/flows_step1.json` 파일 내용을 복사하여 붙여넣기
4. `Import` 버튼 클릭
5. 우측 상단 `Deploy` 버튼 클릭

---

## 🚀 실행 및 테스트

### Step 1: Arduino 업로드

1. Arduino IDE에서 `arduino/wasabi_controller/wasabi_controller.ino` 열기
2. `Tools` → `Board` → "Arduino UNO R4 WiFi" 선택
3. `Tools` → `Port` → Arduino가 연결된 포트 선택
4. 업로드 버튼 (→) 클릭
5. 업로드 완료 후 `Tools` → `Serial Monitor` 열기 (115200 baud)

### Step 2: 연결 확인

**Serial Monitor에 다음과 같이 표시되어야 합니다:**

```
=== Wasabi SmartFarm Controller ===
Step 1: Sensor Data Collection
Version: 1.0.0
===================================

[SETUP] Connecting to WiFi...
Connecting to WiFi: YOUR_WIFI_SSID
..........
[OK] WiFi connected!
IP Address: 192.168.0.10

[SETUP] Connecting to MQTT Broker...
Connecting to MQTT Broker... [OK] MQTT connected!

[SETUP] Initializing sensors...
[OK] SHT30 initialized
[OK] Found 1 DS18B20 sensor(s)
[OK] Modbus RTU initialized
[OK] All sensors initialized

[SETUP] Setup complete!

[SENSOR] Reading environment sensors...
  Air Temp: 21.5°C, Humidity: 65.3%
  Water Temp: 18.7°C, pH: 6.8, TDS: 450 ppm, EC: 1.2 mS/cm
  Tank 1 - Temp: 20.1°C, Moisture: 92.5%, EC: 3.2 μS/cm, pH: 6.5
  ...
[MQTT] Environment data published
```

### Step 3: Node-RED Dashboard 확인

1. 브라우저에서 `http://localhost:1880/ui` 접속
2. "모니터링" 탭 선택
3. 실시간 센서 데이터가 표시되는지 확인:
   - **환경 센서**: 대기 온도, 대기 습도, 온도 차트
   - **물탱크 센서**: 수온, pH, TDS, EC
   - **탱크 수위**: Tank 01~18 수위 게이지

---

## 🧪 테스트 모드 (Arduino 없이 테스트)

Arduino가 아직 준비되지 않았거나 테스트만 하고 싶은 경우:

1. Node-RED Flow에서 "테스트 데이터 주입" Inject 노드 찾기
2. 노드 좌측의 버튼 클릭
3. Dashboard에 테스트 데이터가 표시됨

---

## 🐛 문제 해결

### Arduino WiFi 연결 실패
**증상**: Serial Monitor에 "WiFi connection failed!" 표시

**해결 방법**:
1. `config.h`에서 SSID와 비밀번호 확인
2. WiFi 신호 강도 확인 (라우터와의 거리)
3. 2.4GHz WiFi 사용 확인 (5GHz는 지원 안 함)

### MQTT 연결 실패
**증상**: Serial Monitor에 "MQTT connection failed, rc=-2" 표시

**해결 방법**:
1. Mosquitto가 실행 중인지 확인: `sudo systemctl status mosquitto`
2. `config.h`에서 MQTT_SERVER IP 주소 확인
3. 방화벽에서 1883 포트 열기: `sudo ufw allow 1883/tcp`

### 센서 데이터 수신 안 됨
**증상**: Node-RED Debug에 메시지가 표시되지 않음

**해결 방법**:
1. Node-RED의 MQTT Broker 설정 확인 (localhost 또는 IP)
2. Arduino Serial Monitor에서 MQTT publish 로그 확인
3. Mosquitto에서 토픽 구독 테스트:
   ```bash
   mosquitto_sub -v -t '#'
   ```

### SHT30 센서 오류
**증상**: Serial Monitor에 "SHT30 sensor not found!" 표시

**해결 방법**:
1. I2C 연결 확인 (SDA: A4, SCL: A5)
2. I2C 주소 확인 (기본: 0x44, 설정에 따라 0x45)
3. I2C 스캐너로 주소 확인:
   ```cpp
   // I2C Scanner 예제 실행
   ```

---

## 📊 예상 결과

### Serial Monitor 출력 (10초마다)
```
[SENSOR] Reading environment sensors...
  Air Temp: 21.5°C, Humidity: 65.3%
  Water Temp: 18.7°C, pH: 6.8, TDS: 450 ppm, EC: 1.2 mS/cm
  Tank 1 - Temp: 20.1°C, Moisture: 92.5%, EC: 3.2 μS/cm, pH: 6.5
  Tank 2 - Temp: 20.3°C, Moisture: 91.3%, EC: 3.5 μS/cm, pH: 6.6
  ...
[MQTT] Environment data published

[SENSOR] Reading water level sensors...
  Tank 1 water level: 75%
  Tank 2 water level: 68%
  ...
```

### Dashboard 표시
![Dashboard 예시](../hardware/screenshots/step1_dashboard.png)

- ✅ 대기 온도 게이지: 21.5°C (녹색)
- ✅ 대기 습도 게이지: 65.3% (녹색)
- ✅ 온도 차트: 실시간 추이 표시
- ✅ 수온 게이지: 18.7°C
- ✅ pH 게이지: 6.8
- ✅ TDS 게이지: 450 ppm
- ✅ EC 게이지: 1.2 mS/cm
- ✅ 탱크 수위: Tank 01~18 각각 표시

---

## ✅ Step 1 완료 기준

다음 항목이 모두 달성되면 Step 1이 완료된 것입니다:

- [x] Arduino가 WiFi에 연결됨
- [x] Arduino가 MQTT Broker에 연결됨
- [x] 센서 데이터가 10초마다 MQTT로 전송됨
- [x] Node-RED가 센서 데이터를 수신함
- [x] Dashboard에 모든 센서 데이터가 실시간 표시됨
- [x] 수위 데이터가 3초마다 업데이트됨
- [x] 하트비트가 1분마다 전송되고 연결 상태가 표시됨

---

## 🎯 다음 단계

Step 1이 성공적으로 완료되면, 다음 단계로 진행할 수 있습니다:

**Step 2: 관수장비 자동 제어 로직 구현**
- 토양 습도, EC, 온도 조건 기반 자동 관수
- 시간 기반 제어 (1시간당 4분)
- 관수 이력 로깅

---

## 📞 지원

문제가 발생하거나 질문이 있으면:
1. GitHub Issues에 문의
2. Serial Monitor 출력 캡처 첨부
3. Node-RED Debug 로그 첨부

---

**작성자**: 서준원  
**버전**: 1.0.0  
**최종 수정**: 2024-12-11
