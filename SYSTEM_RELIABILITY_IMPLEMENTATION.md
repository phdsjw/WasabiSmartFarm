# 24/7/365 무중단 운영 구현 가이드

## 📋 문서 정보

**작성일**: 2025-12-17  
**버전**: v1.0.0  
**프로젝트**: WasabiSmartFarm  
**전제 문서**: SYSTEM_RELIABILITY_ANALYSIS.md  
**목적**: Phase 1 Critical Fixes 실제 구현 가이드

---

## 🎯 Phase 1 목표

**목표**: 24시간 365일 무중단 자동 운영 달성  
**예상 소요 시간**: 10시간 (1-2일)  
**예상 가동률 개선**: 90% → 99.5%

---

## 📋 구현 체크리스트

- [ ] **작업 1**: Context 변수 영구 저장 (localfilesystem) - 2시간
- [ ] **작업 2**: systemd 자동 재시작 설정 - 1시간
- [ ] **작업 3**: WiFi 재연결 로직 개선 - 3시간
- [ ] **작업 4**: 하트비트 모니터링 구현 - 4시간

---

## 🔧 작업 1: Context 변수 영구 저장 (2시간)

### 문제점
```javascript
// 현재: Node-RED 재시작 시 초기화
autoMode = false          // ← 자동 모드 OFF!
isIrrigating = false
lastIrrigationTime = 0
```

**영향**: Node-RED 크래시 → 자동 관수 중단

---

### 해결 방법: localfilesystem Context Store

#### 1.1. Node-RED settings.js 편집

```bash
# Node-RED 설정 파일 위치 확인
cd ~/.node-red
ls -la settings.js

# 백업
cp settings.js settings.js.backup.$(date +%Y%m%d)
```

**settings.js 편집**:
```javascript
// ~/.node-red/settings.js
module.exports = {
  // ... 기존 설정 ...
  
  // Context 저장소 설정 추가
  contextStorage: {
    default: {
      module: "memory"  // 기본: 메모리 (빠름)
    },
    file: {
      module: "localfilesystem",  // 파일 시스템에 저장
      config: {
        dir: "/home/nodered/.node-red/context",  // 저장 경로
        cache: true,  // 캐시 활성화 (성능 향상)
        flushInterval: 30  // 30초마다 디스크에 저장
      }
    }
  },
  
  // ... 기존 설정 ...
}
```

---

#### 1.2. Node-RED 플로우 수정

**자동 관수 로직 Function 노드 수정**:

```javascript
// 변경 전
const autoMode = context.get('autoMode') || false;
const isIrrigating = context.get('isIrrigating') || false;
const lastIrrigationTime = context.get('lastIrrigationTime') || 0;
const irrigationStartTime = context.get('irrigationStartTime') || 0;

// 변경 후 (파일에 저장)
const autoMode = context.get('autoMode', 'file') || false;
const isIrrigating = context.get('isIrrigating', 'file') || false;
const lastIrrigationTime = context.get('lastIrrigationTime', 'file') || 0;
const irrigationStartTime = context.get('irrigationStartTime', 'file') || 0;
```

**자동 모드 ON/OFF 버튼 Function 노드 수정**:

```javascript
// 변경 전
context.set('autoMode', autoMode);

// 변경 후 (파일에 저장)
context.set('autoMode', autoMode, 'file');

// 추가: 저장 확인 로그
node.warn(`Auto mode set to: ${autoMode} (saved to file)`);
```

**관수 시작/종료 시 Context 저장**:

```javascript
// 관수 시작 시
context.set('isIrrigating', true, 'file');
context.set('irrigationStartTime', now, 'file');

// 관수 종료 시
context.set('isIrrigating', false, 'file');
context.set('lastIrrigationTime', now, 'file');
context.set('irrigationStartTime', 0, 'file');
```

---

#### 1.3. Node-RED 재시작 및 테스트

```bash
# Node-RED 재시작
sudo systemctl restart nodered

# 로그 확인
sudo journalctl -u nodered -f

# Context 파일 생성 확인
ls -la ~/.node-red/context/
# 예상 출력: global_file.json, [flow_id]_file.json
```

**테스트 시나리오**:
```
1. Dashboard에서 자동 모드 ON
2. Node-RED 재시작: sudo systemctl restart nodered
3. Dashboard 새로고침
4. 자동 모드가 여전히 ON인지 확인 ✅
```

---

#### 1.4. 예상 파일 구조

```
~/.node-red/context/
├── global_file.json          # 전역 Context
└── [flow_id]_file.json       # 플로우별 Context
```

**[flow_id]_file.json 예시**:
```json
{
  "autoMode": true,
  "isIrrigating": false,
  "lastIrrigationTime": 1734441600000,
  "irrigationStartTime": 0
}
```

---

### ✅ 작업 1 완료 기준

- [ ] settings.js에 contextStorage 설정 추가
- [ ] 모든 Function 노드에서 `context.get/set(..., 'file')` 사용
- [ ] Node-RED 재시작 후 Context 변수 유지 확인
- [ ] ~/.node-red/context/ 폴더에 JSON 파일 생성 확인

---

## 🔄 작업 2: systemd 자동 재시작 설정 (1시간)

### 2.1. Node-RED systemd 서비스 설정

```bash
# 서비스 파일 편집
sudo systemctl edit nodered --full

# 또는 직접 편집
sudo nano /etc/systemd/system/nodered.service
```

**nodered.service 내용**:
```ini
[Unit]
Description=Node-RED
Documentation=https://nodered.org/
After=syslog.target network.target

[Service]
Type=simple
User=nodered
Group=nodered
WorkingDirectory=/home/nodered
Environment="NODE_OPTIONS=--max-old-space-size=2048"  # 메모리 제한
ExecStart=/usr/bin/node-red

# 자동 재시작 설정
Restart=always                    # 항상 재시작
RestartSec=10                     # 10초 후 재시작
StartLimitInterval=600            # 10분 동안
StartLimitBurst=5                 # 최대 5회 재시작 시도

# 리소스 제한
MemoryLimit=2G                    # 메모리 2GB 제한
CPUQuota=50%                      # CPU 50% 제한

# 로그 설정
StandardOutput=journal
StandardError=journal
SyslogIdentifier=node-red

[Install]
WantedBy=multi-user.target
```

---

### 2.2. Mosquitto MQTT Broker systemd 설정

```bash
# Mosquitto 서비스 파일 확인
sudo systemctl status mosquitto

# 서비스 파일 편집
sudo nano /etc/systemd/system/mosquitto.service
```

**mosquitto.service 내용**:
```ini
[Unit]
Description=Mosquitto MQTT Broker
Documentation=https://mosquitto.org/
After=network.target

[Service]
Type=notify
NotifyAccess=main
User=mosquitto
Group=mosquitto
ExecStart=/usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
ExecReload=/bin/kill -HUP $MAINPID

# 자동 재시작 설정
Restart=always
RestartSec=5
StartLimitInterval=600
StartLimitBurst=5

# 로그 설정
StandardOutput=journal
StandardError=journal
SyslogIdentifier=mosquitto

[Install]
WantedBy=multi-user.target
```

---

### 2.3. systemd 데몬 리로드 및 활성화

```bash
# 데몬 리로드
sudo systemctl daemon-reload

# 서비스 활성화 (부팅 시 자동 시작)
sudo systemctl enable nodered
sudo systemctl enable mosquitto

# 서비스 재시작
sudo systemctl restart nodered
sudo systemctl restart mosquitto

# 상태 확인
sudo systemctl status nodered
sudo systemctl status mosquitto
```

---

### 2.4. 테스트

**Node-RED 크래시 테스트**:
```bash
# 1. Node-RED 프로세스 ID 확인
ps aux | grep node-red

# 2. 강제 종료
sudo kill -9 [PID]

# 3. 10초 대기 후 자동 재시작 확인
sleep 10
sudo systemctl status nodered
# 예상: active (running)

# 4. 로그 확인
sudo journalctl -u nodered --since "1 minute ago"
```

**MQTT Broker 크래시 테스트**:
```bash
# 1. Mosquitto 강제 종료
sudo killall -9 mosquitto

# 2. 5초 대기 후 자동 재시작 확인
sleep 5
sudo systemctl status mosquitto

# 3. 로그 확인
sudo journalctl -u mosquitto --since "1 minute ago"
```

---

### ✅ 작업 2 완료 기준

- [ ] Node-RED systemd 서비스에 `Restart=always` 설정
- [ ] Mosquitto systemd 서비스에 `Restart=always` 설정
- [ ] 강제 종료 후 자동 재시작 확인
- [ ] 부팅 시 자동 시작 확인

---

## 📡 작업 3: WiFi 재연결 로직 개선 (3시간)

### 3.1. Arduino config.h 수정

```bash
cd /home/user/webapp/arduino/actuator_node
nano config.h
```

**config.h 변경**:
```cpp
// 변경 전
#define WIFI_TIMEOUT 30000  // 30초

// 변경 후
#define WIFI_TIMEOUT 10000           // 10초로 단축
#define WIFI_MAX_RETRY 5             // 최대 5회 재시도
#define WIFI_RETRY_INTERVAL 10000    // 재시도 간격: 10초
```

---

### 3.2. mqtt_handler.cpp 수정

```bash
nano mqtt_handler.cpp
```

**connectWiFi() 함수 개선**:
```cpp
// 개선된 WiFi 재연결 로직
bool MQTTHandler::connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;  // 이미 연결됨
  }
  
  DEBUG_PRINT(F("[WiFi] Connecting to: "));
  DEBUG_PRINTLN(WIFI_SSID);
  
  int retryCount = 0;
  
  while (retryCount < WIFI_MAX_RETRY) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > WIFI_TIMEOUT) {
        // 타임아웃 발생
        DEBUG_PRINT(F("[WiFi] Connection timeout. Retry "));
        DEBUG_PRINT(retryCount + 1);
        DEBUG_PRINT(F("/"));
        DEBUG_PRINTLN(WIFI_MAX_RETRY);
        retryCount++;
        break;
      }
      delay(500);
      DEBUG_PRINT(F("."));
    }
    
    // 연결 성공
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PRINTLN(F("\n[WiFi] Connected!"));
      DEBUG_PRINT(F("[WiFi] IP Address: "));
      DEBUG_PRINTLN(WiFi.localIP());
      DEBUG_PRINT(F("[WiFi] RSSI: "));
      DEBUG_PRINT(WiFi.RSSI());
      DEBUG_PRINTLN(F(" dBm"));
      return true;
    }
    
    // 재시도 전 대기
    if (retryCount < WIFI_MAX_RETRY) {
      DEBUG_PRINTLN(F("[WiFi] Waiting before retry..."));
      delay(WIFI_RETRY_INTERVAL);
    }
  }
  
  // 최대 재시도 횟수 도달
  DEBUG_PRINTLN(F("\n[WiFi] ERROR: Max retry reached"));
  DEBUG_PRINTLN(F("[WiFi] WARNING: Continuing without WiFi connection"));
  DEBUG_PRINTLN(F("[WiFi] System will retry on next loop() cycle"));
  
  return false;  // 실패했지만 시스템은 계속 동작
}
```

---

### 3.3. 모든 Arduino 노드에 적용

**적용 대상**:
- `arduino/actuator_node/config.h`, `mqtt_handler.cpp`
- `arduino/air_sensor_node/config.h`, `mqtt_handler.cpp`
- `arduino/soil_sensor_node/config.h`, `mqtt_handler.cpp`
- `arduino/water_tank_sensor_node/config.h`, `mqtt_handler.cpp`

**일괄 적용 스크립트**:
```bash
cd /home/user/webapp

# config.h 수정
for node in actuator_node air_sensor_node soil_sensor_node water_tank_sensor_node; do
  echo "Updating arduino/${node}/config.h"
  # (MultiEdit로 수정 필요)
done
```

---

### 3.4. Arduino 업로드 및 테스트

```bash
# Arduino IDE에서 각 노드 업로드
# 1. actuator_node.ino
# 2. air_sensor_node.ino
# 3. soil_sensor_node.ino
# 4. water_tank_sensor_node.ino
```

**테스트 시나리오**:
```
1. Arduino 정상 동작 확인
2. WiFi 라우터 전원 OFF (30초)
3. 시리얼 모니터 확인:
   [WiFi] Connection lost. Reconnecting...
   [WiFi] Connection timeout. Retry 1/5
   [WiFi] Waiting before retry...
4. WiFi 라우터 전원 ON
5. 재연결 확인:
   [WiFi] Connected!
   [MQTT] Connected to broker!
6. 자동 관수 로직 정상 동작 확인
```

---

### ✅ 작업 3 완료 기준

- [ ] 모든 Arduino 노드에 WiFi 재연결 로직 적용
- [ ] WiFi 타임아웃 10초로 단축
- [ ] 최대 5회 재시도 설정
- [ ] WiFi 단절 테스트 통과

---

## 💓 작업 4: 하트비트 모니터링 구현 (4시간)

### 4.1. Node-RED 하트비트 모니터링 플로우 추가

**flows_improved_ui.json에 추가**:

```json
[
  {
    "id": "heartbeat_monitor_group",
    "type": "ui_group",
    "name": "시스템 상태",
    "tab": "ui_tab_control",
    "order": 2,
    "disp": true,
    "width": "12",
    "collapse": false
  },
  {
    "id": "mqtt_heartbeat_all",
    "type": "mqtt in",
    "z": "main_tab",
    "name": "모든 하트비트 수신",
    "topic": "+/heartbeat",
    "qos": "0",
    "datatype": "json",
    "broker": "mqtt_broker",
    "nl": false,
    "rap": true,
    "rh": 0,
    "inputs": 0,
    "x": 140,
    "y": 760,
    "wires": [["heartbeat_processor"]]
  },
  {
    "id": "heartbeat_processor",
    "type": "function",
    "z": "main_tab",
    "name": "하트비트 처리",
    "func": "// 토픽에서 노드 ID 추출\n// 예: actuator/heartbeat → actuator\nconst parts = msg.topic.split('/');\nconst nodeId = parts[0];\n\n// 현재 시간 저장\nconst now = Date.now();\ncontext.set(`hb_${nodeId}`, now, 'file');\n\n// 로그\nnode.log(`Heartbeat received from ${nodeId}`);\n\n// 상태 업데이트 메시지 생성\nmsg.payload = {\n  node: nodeId,\n  status: 'online',\n  timestamp: now,\n  data: msg.payload\n};\n\nreturn msg;",
    "outputs": 1,
    "x": 360,
    "y": 760,
    "wires": [["heartbeat_status_display"]]
  },
  {
    "id": "heartbeat_checker",
    "type": "inject",
    "z": "main_tab",
    "name": "60초마다 체크",
    "props": [],
    "repeat": "60",
    "crontab": "",
    "once": true,
    "onceDelay": "10",
    "topic": "",
    "x": 140,
    "y": 800,
    "wires": [["heartbeat_timeout_check"]]
  },
  {
    "id": "heartbeat_timeout_check",
    "type": "function",
    "z": "main_tab",
    "name": "타임아웃 체크",
    "func": "const now = Date.now();\nconst timeout = 120000;  // 2분\nconst nodes = ['actuator', 'sensor/soil/zone1', 'sensor/air/zone1', 'sensor/water_tank'];\nconst alerts = [];\n\nnodes.forEach(nodeId => {\n  const lastHB = context.get(`hb_${nodeId}`, 'file') || 0;\n  const elapsed = now - lastHB;\n  \n  if (lastHB === 0) {\n    // 아직 하트비트를 받지 못함\n    alerts.push({\n      node: nodeId,\n      status: 'never_connected',\n      message: 'No heartbeat received yet'\n    });\n  } else if (elapsed > timeout) {\n    // 타임아웃\n    alerts.push({\n      node: nodeId,\n      status: 'timeout',\n      elapsed: Math.floor(elapsed / 1000) + 's',\n      message: `Heartbeat timeout (${Math.floor(elapsed/1000)}s)`\n    });\n  }\n});\n\nif (alerts.length > 0) {\n  node.error('⚠️ Heartbeat timeout detected');\n  msg.payload = {\n    alert_type: 'heartbeat_timeout',\n    nodes: alerts,\n    timestamp: now\n  };\n  return msg;\n}\n\nreturn null;",
    "outputs": 1,
    "x": 360,
    "y": 800,
    "wires": [["heartbeat_alert_display"]]
  },
  {
    "id": "heartbeat_status_display",
    "type": "ui_text",
    "z": "main_tab",
    "group": "heartbeat_monitor_group",
    "order": 1,
    "width": "12",
    "height": "2",
    "name": "하트비트 상태",
    "label": "노드 상태",
    "format": "{{msg.payload}}",
    "layout": "row-spread",
    "x": 580,
    "y": 760,
    "wires": []
  },
  {
    "id": "heartbeat_alert_display",
    "type": "ui_toast",
    "z": "main_tab",
    "position": "top right",
    "displayTime": "10",
    "highlight": "red",
    "sendall": true,
    "outputs": 0,
    "ok": "OK",
    "cancel": "",
    "raw": false,
    "className": "",
    "topic": "⚠️ 하트비트 타임아웃",
    "name": "경고 알림",
    "x": 580,
    "y": 800,
    "wires": []
  }
]
```

---

### 4.2. Dashboard 상태 표시 추가

**ui_template 노드로 상세 상태 표시**:
```html
<div style="padding: 10px;">
  <h3>시스템 노드 상태</h3>
  <table style="width: 100%; border-collapse: collapse;">
    <tr style="background: #2196F3; color: white;">
      <th style="padding: 8px; text-align: left;">노드</th>
      <th style="padding: 8px; text-align: left;">상태</th>
      <th style="padding: 8px; text-align: left;">마지막 하트비트</th>
    </tr>
    <tr ng-repeat="node in msg.payload.nodes">
      <td style="padding: 8px; border: 1px solid #ddd;">{{node.name}}</td>
      <td style="padding: 8px; border: 1px solid #ddd;">
        <span ng-if="node.status === 'online'" style="color: green;">●  온라인</span>
        <span ng-if="node.status === 'timeout'" style="color: red;">●  타임아웃</span>
      </td>
      <td style="padding: 8px; border: 1px solid #ddd;">{{node.elapsed}}</td>
    </tr>
  </table>
</div>
```

---

### 4.3. 이메일 알림 추가 (선택사항)

**node-red-node-email 설치**:
```bash
cd ~/.node-red
npm install node-red-node-email
sudo systemctl restart nodered
```

**이메일 알림 노드 추가**:
```json
{
  "id": "heartbeat_email_alert",
  "type": "e-mail",
  "z": "main_tab",
  "server": "smtp.gmail.com",
  "port": "465",
  "secure": true,
  "name": "이메일 알림",
  "dname": "WasabiSmartFarm Alert",
  "x": 800,
  "y": 800,
  "wires": []
}
```

---

### ✅ 작업 4 완료 기준

- [ ] 하트비트 수신 노드 추가 (mqtt in: +/heartbeat)
- [ ] 하트비트 처리 로직 구현 (타임스탬프 저장)
- [ ] 60초마다 타임아웃 체크
- [ ] Dashboard에 상태 표시
- [ ] 타임아웃 시 경고 알림

---

## 🧪 통합 테스트

### 테스트 1: Context 변수 영구 저장
```
1. Dashboard에서 자동 모드 ON
2. sudo systemctl restart nodered
3. 10초 대기
4. Dashboard 새로고침
5. ✅ 자동 모드가 여전히 ON
```

### 테스트 2: systemd 자동 재시작
```
1. sudo kill -9 $(pgrep node-red)
2. 10초 대기
3. sudo systemctl status nodered
4. ✅ active (running)
```

### 테스트 3: WiFi 재연결
```
1. WiFi 라우터 전원 OFF
2. 30초 대기
3. WiFi 라우터 전원 ON
4. Arduino 시리얼 모니터 확인
5. ✅ [WiFi] Connected!
```

### 테스트 4: 하트비트 모니터링
```
1. Dashboard "시스템 상태" 그룹 확인
2. 모든 노드 "온라인" 표시 확인
3. actuator_node Arduino 전원 OFF
4. 2분 대기
5. ✅ "타임아웃" 경고 알림 표시
```

---

## 📋 Git Commit

```bash
cd /home/user/webapp

# 변경사항 확인
git status

# 추가
git add -A

# 커밋
git commit -m "feat: 24/7 무중단 운영 Phase 1 구현 (v1.1.0)

- Context 변수 localfilesystem 영구 저장
- systemd 자동 재시작 설정
- WiFi 재연결 로직 개선 (5회 재시도)
- 하트비트 모니터링 구현

예상 가동률: 90% → 99.5%"

# 푸시
git push origin main
```

---

## ✅ 최종 검증

### 가동률 측정 (7일 테스트)

```bash
# 가동 시간 확인
uptime

# Node-RED 재시작 횟수
sudo journalctl -u nodered --since "7 days ago" | grep "Started" | wc -l

# MQTT Broker 재시작 횟수
sudo journalctl -u mosquitto --since "7 days ago" | grep "Started" | wc -l

# 가동률 계산
# 가동률 = (168시간 - 다운타임) / 168시간 × 100%
```

**목표**:
- 가동률 ≥ 99.5% (7시간 이내 다운타임)
- 자동 복구 성공률 ≥ 95%

---

## 🎉 완료!

Phase 1 구현 완료 후 시스템은 **24/7/365 무중단 운영**이 가능합니다!

**다음 단계**: Phase 2 (선택사항)
- Arduino Watchdog 타이머
- 센서 데이터 유효성 검사
- 알림 시스템 구축

---

**작성일**: 2025-12-17  
**프로젝트**: WasabiSmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm
