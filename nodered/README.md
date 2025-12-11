# 🌐 와사비 스마트팜 - Node-RED 플로우

**작성자**: 서준원  
**버전**: v1.0.0  
**날짜**: 2024-12-11

---

## 📋 개요

와사비 스마트팜의 중앙 제어 시스템입니다.  
22개 센서 노드에서 데이터를 수집하고, 평균값을 계산하여 자동 관수 제어를 수행합니다.

---

## 🎯 주요 기능

### 데이터 수집
- ✅ **토양 센서 18개** - 온도, 습도, EC, pH
- ✅ **대기 센서 3개** - 온도, 습도
- ✅ **수조 센서 1개** - 수온, pH, TDS, EC
- ✅ **액추에이터 상태** - 펌프 ON/OFF 상태

### 데이터 처리
- ✅ **평균값 계산** - 토양/대기 센서 평균
- ✅ **데이터 유효성 검증** - 최근 1분 이내 데이터만 사용
- ✅ **최소 센서 개수 체크** - 토양 12개 이상 필요

### 자동 제어
- ✅ **자동 관수 제어**
  - 조건: 토양 습도 ≤ 95% OR 토양 EC ≥ 5.0 OR 토양 온도 ≥ 22°C
  - 작동 시간: 4분
  - 최소 간격: 1시간
- ✅ **타이머 자동 종료** - 4분 후 자동 OFF

### Dashboard UI
- ✅ **개요 탭** - 시스템 상태, 평균값
- ✅ **토양 센서 탭** - 18개 센서 차트
- ✅ **대기 센서 탭** - 3개 센서 차트
- ✅ **수조 센서 탭** - 수질 데이터
- ✅ **제어 탭** - 수동/자동 제어

### 데이터 저장
- ✅ **InfluxDB 저장** - 시계열 데이터베이스
- ✅ **관수 로그** - 작동 이력 저장

### 수동 제어
- ✅ **관수 시작/정지** - 버튼 제어
- ✅ **배수 시작/정지** - 버튼 제어
- ✅ **긴급 정지** - 모든 액추에이터 정지
- ✅ **자동 모드 스위치** - ON/OFF

---

## 🔧 설치 및 설정

### 1. Node-RED 설치

#### Ubuntu/Debian
```bash
# Node.js 설치 (v16 이상)
curl -fsSL https://deb.nodesource.com/setup_16.x | sudo -E bash -
sudo apt-get install -y nodejs

# Node-RED 설치
sudo npm install -g --unsafe-perm node-red

# 서비스 등록
sudo systemctl enable nodered.service
sudo systemctl start nodered.service
```

#### Windows
```powershell
# Node.js 다운로드 및 설치
# https://nodejs.org/

# Node-RED 설치
npm install -g --unsafe-perm node-red

# 실행
node-red
```

### 2. 필수 노드 설치

Node-RED 설치 후 다음 노드를 설치하세요:

```bash
cd ~/.node-red

# Dashboard
npm install node-red-dashboard

# InfluxDB
npm install node-red-contrib-influxdb

# 또는 Node-RED UI에서 설치:
# 메뉴 → Manage palette → Install 탭
```

**필수 노드**:
- `node-red-dashboard` - Dashboard UI
- `node-red-contrib-influxdb` - InfluxDB 연동

**선택 노드**:
- `node-red-contrib-telegrambot` - Telegram 알림
- `node-red-node-email` - 이메일 알림
- `node-red-contrib-googlesheets` - Google Sheets 연동

### 3. Mosquitto MQTT Broker 설치

#### Ubuntu/Debian
```bash
# Mosquitto 설치
sudo apt-get update
sudo apt-get install -y mosquitto mosquitto-clients

# 서비스 시작
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# 상태 확인
sudo systemctl status mosquitto
```

#### 설정 파일 (`/etc/mosquitto/mosquitto.conf`)
```conf
# 기본 설정
listener 1883
allow_anonymous true

# 로그 설정
log_dest file /var/log/mosquitto/mosquitto.log
log_type all

# 영구 저장
persistence true
persistence_location /var/lib/mosquitto/
```

### 4. InfluxDB 설치 (선택사항)

#### Ubuntu/Debian
```bash
# InfluxDB 저장소 추가
wget -q https://repos.influxdata.com/influxdata-archive_compat.key
echo '393e8779c89ac8d958f81f942f9ad7fb82a25e133faddaf92e15b16e6ac9ce4c influxdata-archive_compat.key' | sha256sum -c && cat influxdata-archive_compat.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg > /dev/null
echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list

# 설치
sudo apt-get update
sudo apt-get install influxdb

# 서비스 시작
sudo systemctl enable influxdb
sudo systemctl start influxdb
```

#### 데이터베이스 생성
```bash
# InfluxDB CLI 실행
influx

# 데이터베이스 생성
CREATE DATABASE wasabi_smartfarm

# 확인
SHOW DATABASES

# 종료
exit
```

### 5. 플로우 임포트

1. Node-RED 웹 UI 접속: `http://localhost:1880`
2. 메뉴(☰) → Import 클릭
3. `flows.json` 파일 내용 복사 붙여넣기
4. Import 버튼 클릭

### 6. MQTT Broker 설정

플로우 임포트 후 MQTT 브로커 설정 확인:

1. `mqtt-broker` 노드 더블클릭
2. Server 탭:
   - **Server**: `localhost` (또는 브로커 IP)
   - **Port**: `1883`
   - **Client ID**: `nodered_wasabi_farm`
3. Apply 클릭

### 7. InfluxDB 연결 설정 (선택)

InfluxDB 노드 추가 시:

1. InfluxDB 노드 추가
2. 설정:
   - **Version**: `1.x`
   - **URL**: `http://localhost:8086`
   - **Database**: `wasabi_smartfarm`
3. `save_to_influx_*` 노드와 연결

---

## 📡 MQTT 토픽 구조

### 구독 토픽 (센서 데이터 수신)

```
sensor/soil/+/data              # 토양 센서 18개 (와일드카드)
sensor/air/+/data               # 대기 센서 3개 (와일드카드)
sensor/water_tank/data          # 수조 센서 1개
actuator/state                  # 액추에이터 상태
actuator/heartbeat              # 액추에이터 하트비트
```

### 발행 토픽 (명령 전송)

```
actuator/irrigation_pump/on     # 관수 펌프 켜기
actuator/irrigation_pump/off    # 관수 펌프 끄기
actuator/drainage_pump/on       # 배수 펌프 켜기
actuator/drainage_pump/off      # 배수 펌프 끄기
actuator/fan/on                 # 팬 켜기
actuator/fan/off                # 팬 끄기
actuator/led/on                 # LED 켜기
actuator/led/off                # LED 끄기
actuator/emergency_stop         # 긴급 정지
actuator/reset                  # 긴급 정지 해제
```

---

## 🖥️ Dashboard 접속

### URL
- **로컬**: http://localhost:1880/ui
- **네트워크**: http://[서버IP]:1880/ui

### Dashboard 탭

#### 1. 개요 탭
- **시스템 상태**: 액추에이터 ON/OFF 상태
- **평균값**: 토양/대기 센서 평균값 실시간 표시

#### 2. 토양 센서 탭
- **토양 센서 차트**: 18개 센서 데이터 차트
- 온도, 습도, EC, pH 그래프

#### 3. 대기 센서 탭
- **대기 센서 차트**: 3개 센서 데이터 차트
- 온도, 습도 그래프

#### 4. 수조 센서 탭
- **수조 수질**: 수온, pH, TDS, EC 게이지

#### 5. 제어 탭
- **수동 제어**: 관수/배수 시작/정지 버튼
- **긴급 정지**: 🚨 긴급 정지 버튼
- **자동 제어 설정**: 자동 모드 ON/OFF 스위치

---

## ⚙️ 자동 관수 제어 로직

### 조건 체크 (1분 주기)

```javascript
IF (평균 토양 습도 <= 95%) OR
   (평균 토양 EC >= 5.0 μS/cm) OR
   (평균 토양 온도 >= 22°C)
THEN
    관수 시작 (4분)
    다음 관수까지 최소 1시간 대기
```

### 평균값 계산 규칙

#### 토양 센서 (18개)
- 최근 1분 이내 데이터만 사용
- 최소 12개 센서 필요
- 유효 센서 수가 12개 미만이면 평균 계산 안 함

#### 대기 센서 (3개)
- 최근 1분 이내 데이터만 사용
- 모든 센서 데이터 사용

### 관수 작동

1. **조건 충족 확인**
2. **최소 간격 체크** (1시간)
3. **MQTT 명령 전송**: `actuator/irrigation_pump/on`
4. **4분 타이머 시작**
5. **4분 후 자동 정지**: `actuator/irrigation_pump/off`
6. **로그 저장**: InfluxDB에 관수 이력 저장

---

## 🔧 플로우 커스터마이징

### 관수 조건 변경

`check_irrigation_condition` 함수 노드에서 임계값 수정:

```javascript
const moistureThreshold = global.get('irrigationMoistureThreshold') || 95;
const ecThreshold = global.get('irrigationECThreshold') || 5.0;
const tempThreshold = global.get('irrigationTempThreshold') || 22;
```

### 관수 시간 변경

`irrigation_timer` delay 노드에서 시간 수정:
- 현재: 4분 (240초)
- 수정: delay 노드 더블클릭 → Timeout 변경

### 최소 간격 변경

`check_irrigation_condition` 함수 노드에서:

```javascript
const oneHour = 3600000;  // 1시간 (밀리초)
// 변경 예: 30분 = 1800000
```

---

## 📊 데이터 저장 (InfluxDB)

### Measurement 구조

#### soil_sensor
```
Tags:
  - tank_id (01~18)

Fields:
  - soil_temp (float)
  - soil_moisture (float)
  - soil_ec (float)
  - soil_ph (float)
  - rssi (int)
```

#### air_sensor
```
Tags:
  - zone_id (01~03)

Fields:
  - air_temp (float)
  - air_humidity (float)
  - rssi (int)
```

#### water_tank_sensor
```
Tags:
  - location (main_tank)

Fields:
  - water_temp (float)
  - water_ph (float)
  - water_tds (int)
  - water_ec (float)
  - rssi (int)
```

#### irrigation_log
```
Fields:
  - reason (string)
  - avg_soil_moisture (float)
  - avg_soil_ec (float)
  - avg_soil_temp (float)
  - duration_seconds (int)
```

### 데이터 조회 (InfluxQL)

```sql
-- 최근 1시간 토양 습도
SELECT mean("soil_moisture") 
FROM "soil_sensor" 
WHERE time > now() - 1h 
GROUP BY time(10m), "tank_id"

-- 관수 이력
SELECT * FROM "irrigation_log" 
WHERE time > now() - 24h

-- 평균 대기 온도
SELECT mean("air_temp") 
FROM "air_sensor" 
WHERE time > now() - 1d 
GROUP BY time(1h)
```

---

## 🧪 테스트 방법

### 1. MQTT 연결 테스트

```bash
# Mosquitto 구독 (모든 토픽)
mosquitto_sub -h localhost -t "#" -v

# 토양 센서 데이터만 구독
mosquitto_sub -h localhost -t "sensor/soil/+/data" -v

# 액추에이터 상태만 구독
mosquitto_sub -h localhost -t "actuator/#" -v
```

### 2. 테스트 데이터 발행

```bash
# 토양 센서 테스트 데이터
mosquitto_pub -h localhost -t "sensor/soil/tank01/data" -m '{"tank_id":"01","soil_temp":20.5,"soil_moisture":92.3,"soil_ec":3.2,"soil_ph":6.5,"timestamp":1702284000000,"rssi":-65}'

# 대기 센서 테스트 데이터
mosquitto_pub -h localhost -t "sensor/air/zone01/data" -m '{"zone_id":"01","air_temp":22.5,"air_humidity":65.3,"timestamp":1702284000000,"rssi":-65}'

# 수조 센서 테스트 데이터
mosquitto_pub -h localhost -t "sensor/water_tank/data" -m '{"water_temp":18.5,"water_ph":6.8,"water_tds":450,"water_ec":1.2,"timestamp":1702284000000,"rssi":-65}'
```

### 3. 수동 제어 테스트

Dashboard UI에서:
1. **제어** 탭 이동
2. **관수 시작** 버튼 클릭
3. 시리얼 모니터에서 릴레이 ON 확인
4. **관수 정지** 버튼 클릭
5. 릴레이 OFF 확인

### 4. 자동 제어 테스트

1. **자동 관수** 스위치 ON
2. 토양 센서 데이터 발행 (습도 90%)
   ```bash
   for i in {01..18}; do
     mosquitto_pub -h localhost -t "sensor/soil/tank$i/data" -m "{\"tank_id\":\"$i\",\"soil_temp\":20.5,\"soil_moisture\":90.0,\"soil_ec\":3.2,\"soil_ph\":6.5,\"timestamp\":$(date +%s)000,\"rssi\":-65}"
   done
   ```
3. 1분 후 자동 관수 시작 확인
4. 4분 후 자동 정지 확인

---

## 🛠️ 문제 해결

### Node-RED가 시작되지 않음

```bash
# 로그 확인
journalctl -u nodered -f

# 또는
~/.node-red/node-red.log
```

### Dashboard가 표시되지 않음

1. `node-red-dashboard` 설치 확인:
   ```bash
   cd ~/.node-red
   npm list node-red-dashboard
   ```
2. Node-RED 재시작:
   ```bash
   sudo systemctl restart nodered
   ```

### MQTT 연결 실패

1. Mosquitto 서비스 확인:
   ```bash
   sudo systemctl status mosquitto
   ```
2. 방화벽 확인:
   ```bash
   sudo ufw allow 1883/tcp
   ```
3. MQTT 브로커 주소 확인 (`mqtt-broker` 노드)

### InfluxDB 연결 실패

1. InfluxDB 서비스 확인:
   ```bash
   sudo systemctl status influxdb
   ```
2. 데이터베이스 존재 확인:
   ```bash
   influx -execute "SHOW DATABASES"
   ```

### 평균값이 계산되지 않음

1. 디버그 노드 추가:
   - `calculate_soil_avg` 노드 출력에 `debug` 노드 연결
   - Debug 창에서 출력 확인
2. 전역 컨텍스트 확인:
   - Function 노드에 추가:
     ```javascript
     node.warn(JSON.stringify(global.get('soilData')));
     ```

---

## 📁 파일 구조

```
nodered/
├── flows.json              # 메인 플로우
└── README.md               # 이 파일
```

---

## 🔄 업그레이드 가이드

### 플로우 백업

```bash
# flows.json 백업
cp ~/.node-red/flows.json ~/.node-red/flows_backup_$(date +%Y%m%d).json
```

### 플로우 업데이트

1. 백업 생성
2. Node-RED UI에서 Export
3. 새 플로우 Import
4. Deploy 클릭

---

## 📞 지원

- **GitHub**: https://github.com/phdsjw/WasabiSmartFarm
- **작성자**: 서준원

---

## 📜 라이선스

이 프로젝트는 MIT 라이선스를 따릅니다.
