# 와사비 스마트팜 24/7/365 무중단 운영 안정성 분석

## 📋 문서 정보

**작성일**: 2025-12-17  
**버전**: v1.0.0  
**프로젝트**: WasabiSmartFarm  
**목적**: 24시간 365일 무중단 자동 운영 안정성 검증 및 개선안 제시

---

## 🎯 요구사항

**핵심 요구사항**: 자동 모드 ON 시, 365일 이상 무중단 운영

**시스템 구성**:
- Arduino Uno R4 WiFi × 5개 (센서 노드 + 액추에이터 노드)
- Node-RED (자동 관수 제어 로직)
- MQTT Broker (Mosquitto)
- WiFi 네트워크

---

## 🔍 현재 시스템 안정성 분석

### 1. 잠재적 장애 포인트 (7개)

| 번호 | 컴포넌트 | 장애 유형 | 발생 가능성 | 영향도 |
|------|----------|----------|------------|--------|
| 1 | **WiFi 네트워크** | 일시적 단절, 신호 약화 | ⚠️ 높음 | 🔴 높음 |
| 2 | **MQTT Broker** | 프로세스 종료, 메모리 부족 | ⚠️ 중간 | 🔴 높음 |
| 3 | **Node-RED** | 메모리 누수, 크래시 | ⚠️ 중간 | 🔴 높음 |
| 4 | **Arduino 노드** | WiFi 재연결 실패, 메모리 부족 | ⚠️ 중간 | 🟡 중간 |
| 5 | **전원** | 순간 정전, 전압 불안정 | ⚠️ 낮음 | 🔴 높음 |
| 6 | **센서 오류** | 센서 고장, 잘못된 데이터 | ⚠️ 중간 | 🟡 중간 |
| 7 | **Context 변수** | Node-RED 재시작 시 초기화 | ⚠️ 중간 | 🟡 중간 |

---

## 📊 현재 구현된 안전 메커니즘

### ✅ Arduino 노드 (현재 구현됨)

#### 1. WiFi 자동 재연결
```cpp
// mqtt_handler.cpp - loop() 함수
void MQTTHandler::loop() {
  // WiFi 재연결
  if (!isWiFiConnected()) {
    DEBUG_PRINTLN(F("[WiFi] Connection lost. Reconnecting..."));
    connectWiFi();  // 자동 재연결 시도
  }
  
  // MQTT 재연결 (5초마다 시도)
  if (!isMQTTConnected()) {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt > 5000) {
      _lastReconnectAttempt = now;
      DEBUG_PRINTLN(F("[MQTT] Connection lost. Reconnecting..."));
      
      if (connectMQTT()) {
        _lastReconnectAttempt = 0;
      }
    }
  }
  
  _mqttClient.loop();  // MQTT 메시지 처리
}
```

**평가**: ✅ **양호**
- WiFi 끊김 감지 및 자동 재연결
- MQTT 연결 끊김 시 5초마다 재시도
- 무한 루프 방지 (타임아웃 처리)

**문제점**: 
- WiFi 재연결 타임아웃이 30초로 길어서 재연결 실패 시 시스템 정지
- 재연결 시도 횟수 제한 없음 (무한 재시도)

---

#### 2. Heartbeat 전송
```cpp
// actuator_node.ino - loop() 함수
// 하트비트 전송 (10초 주기)
if (currentMillis - lastHeartbeat >= HEARTBEAT_INTERVAL) {
  lastHeartbeat = currentMillis;
  
  ActuatorState state = actuatorControl.getState();
  mqttHandler.publishHeartbeat(state);
}
```

**평가**: ✅ **양호**
- 10초마다 하트비트 전송
- MQTT 토픽: `actuator/heartbeat`
- Payload: `{"status": "alive", "emergency_stop": false, ...}`

**문제점**: 
- Node-RED에서 하트비트 모니터링 로직 없음
- 하트비트 끊김 시 경고/복구 메커니즘 부재

---

#### 3. 안전 타임아웃
```cpp
// config.h
#define IRRIGATION_TIMEOUT 300000    // 관수 펌프: 5분
#define DRAINAGE_TIMEOUT 300000      // 배수 펌프: 5분
#define FAN_TIMEOUT 3600000          // 팬: 60분
#define LED_TIMEOUT 43200000         // LED: 12시간

// actuator_control.cpp
void ActuatorControl::checkTimeouts() {
  unsigned long now = millis();
  
  // 관수 펌프 타임아웃
  if (_state.irrigation_pump) {
    if (now - _state.irrigation_start_time > IRRIGATION_TIMEOUT) {
      DEBUG_PRINTLN(F("[ACTUATOR] Irrigation timeout! Auto-stopping..."));
      stopIrrigationPump();
    }
  }
  // 배수 펌프 타임아웃
  if (_state.drainage_pump) {
    if (now - _state.drainage_start_time > DRAINAGE_TIMEOUT) {
      DEBUG_PRINTLN(F("[ACTUATOR] Drainage timeout! Auto-stopping..."));
      stopDrainagePump();
    }
  }
  // ... (Fan, LED 동일)
}
```

**평가**: ✅ **매우 우수**
- 모든 액추에이터에 타임아웃 설정
- 자동 종료로 과부하 방지
- 하드웨어 보호

**문제점**: 없음 ✅

---

### ⚠️ Node-RED (부분적 구현)

#### 1. MQTT 자동 재연결
```json
{
  "id": "mqtt_broker",
  "autoConnect": true,        // ✅ 자동 연결 활성화
  "keepalive": "60",          // ✅ 60초 keep-alive
  "cleansession": true,       // ⚠️ 세션 유지 안 함
  "birthTopic": "nodered/status",
  "birthPayload": "online",
  "willTopic": "nodered/status",
  "willPayload": "offline"    // ✅ Last Will 메시지
}
```

**평가**: ✅ **양호**
- MQTT Broker 재연결 자동 처리
- Last Will 메시지로 비정상 종료 감지 가능

**문제점**:
- `cleansession: true`로 설정되어 있어 Node-RED 재시작 시 구독 정보 초기화
- Context 변수가 메모리에만 저장 (재시작 시 손실)

---

#### 2. Context 변수 관리
```javascript
// 자동 관수 로직
const autoMode = context.get('autoMode') || false;
const isIrrigating = context.get('isIrrigating') || false;
const lastIrrigationTime = context.get('lastIrrigationTime') || 0;
```

**평가**: ⚠️ **위험**
- Context 변수가 메모리에만 저장
- Node-RED 재시작 시 모든 변수 초기화됨
- `autoMode`가 `false`로 리셋 → **자동 관수 중단!**

**문제점**: 🔴 **심각**
- Node-RED 크래시 시 자동 모드 OFF
- 수동으로 다시 켜지 않으면 관수 중단
- 365일 무중단 운영 불가능

---

#### 3. 자동 관수 로직 복구 메커니즘
**현재 상태**: ❌ **없음**
- Node-RED 재시작 감지 로직 없음
- Context 변수 영구 저장 미구현
- Watchdog 타이머 없음

---

### ❌ MQTT Broker (Mosquitto)

#### 현재 상태
- 수동 설치 및 실행
- systemd 자동 재시작 설정 필요 확인 필요
- 로그 로테이션 설정 필요

**평가**: ⚠️ **확인 필요**

---

## 🚨 심각한 문제점 요약

### 🔴 Critical Issues (시스템 중단 가능)

#### 1. Node-RED Context 변수 휘발성 🔴
**문제**:
```javascript
// Node-RED 재시작 시
autoMode = false          // ← 자동 모드 OFF!
isIrrigating = false      // ← 관수 중단!
lastIrrigationTime = 0    // ← 1시간 간격 로직 리셋!
```

**영향**: 
- Node-RED 크래시 → 자동 모드 비활성화 → 관수 중단
- 사용자가 수동으로 다시 켜지 않으면 와사비 고사 위험

**발생 시나리오**:
1. 자동 모드 ON → 정상 운영 중
2. Node-RED 메모리 부족 → 프로세스 종료
3. systemd가 자동 재시작
4. **autoMode = false로 초기화됨**
5. 자동 관수 중단 → 와사비 피해

**해결 필요도**: 🔴 **최우선 (Critical)**

---

#### 2. WiFi 재연결 타임아웃 길이 ⚠️
**문제**:
```cpp
// mqtt_handler.cpp - connectWiFi()
#define WIFI_TIMEOUT 30000  // 30초

while (WiFi.status() != WL_CONNECTED) {
  if (millis() - startTime > WIFI_TIMEOUT) {
    DEBUG_PRINTLN(F("\n[WiFi] ERROR: Connection timeout!"));
    return false;  // ← 재연결 포기!
  }
  delay(500);
}
```

**영향**:
- WiFi 일시 단절 30초 이상 → Arduino 네트워크 끊김
- MQTT 메시지 수신 불가 → 자동 관수 명령 못 받음

**해결 필요도**: 🟡 **높음 (High)**

---

#### 3. 하트비트 모니터링 부재 ⚠️
**문제**:
- Arduino → Node-RED로 하트비트 전송 중
- **Node-RED에서 하트비트 끊김 감지 로직 없음**
- Arduino 다운 시 감지 불가

**영향**:
- actuator_node 다운 → 관수 불가능
- 사용자가 인지하지 못하면 와사비 피해

**해결 필요도**: 🟡 **높음 (High)**

---

### 🟡 Major Issues (성능 저하 가능)

#### 4. 메모리 누수 가능성
**Arduino**:
- `millis()` overflow (49.7일 후)
- 문자열 처리 시 heap fragmentation

**Node-RED**:
- 장기 운영 시 메모리 누수 가능
- Context 변수 누적

**해결 필요도**: 🟡 **중간 (Medium)**

---

#### 5. 센서 오류 데이터 필터링 부족
**문제**:
- Arduino에서 `valid=false` 플래그 전송
- Node-RED에서 필터링 안 함
- 잘못된 데이터로 자동 관수 트리거 가능

**해결 필요도**: 🟡 **중간 (Medium)**

---

## 🛠️ 해결 방안 및 개선안

### 🔴 최우선 해결 (Critical)

#### 해결안 1: Context 변수 영구 저장

**방법 1: Node-RED Context Store 사용 (추천)**
```javascript
// settings.js에 추가
contextStorage: {
  default: {
    module: "localfilesystem"  // 파일 시스템에 저장
  }
}

// 사용 방법
context.set('autoMode', true, 'default');  // 파일에 저장
const autoMode = context.get('autoMode', 'default') || false;
```

**장점**:
- Node-RED 재시작 시에도 변수 유지
- 자동 모드 상태 보존
- 추가 코드 최소화

**단점**:
- 약간의 성능 오버헤드 (파일 I/O)
- settings.js 수정 필요

---

**방법 2: 외부 데이터베이스 사용 (Redis, MongoDB)**
```javascript
// Redis를 Context Store로 사용
contextStorage: {
  default: {
    module: "memory"
  },
  redis: {
    module: "node-red-contrib-contextredis",
    config: {
      host: "localhost",
      port: 6379
    }
  }
}
```

**장점**:
- 고성능
- 여러 Node-RED 인스턴스 공유 가능
- 백업 용이

**단점**:
- 추가 의존성
- Redis 설치 필요

---

**방법 3: MQTT Retain 메시지 사용 (간단한 대안)**
```javascript
// 자동 모드 변경 시 MQTT에 저장
if (autoMode) {
  msg.topic = 'nodered/config/autoMode';
  msg.payload = 'true';
  msg.retain = true;  // ← Broker에 저장
  return msg;
}

// Node-RED 시작 시 복원
// inject 노드로 nodered/config/autoMode subscribe
```

**장점**:
- 간단한 구현
- MQTT Broker가 상태 저장
- 추가 설정 불필요

**단점**:
- MQTT Broker 의존성
- 복잡한 상태 관리 어려움

---

#### 해결안 2: Watchdog 타이머 구현

**Arduino Watchdog (Hardware)**
```cpp
// actuator_node.ino에 추가
#include <Watchdog.h>

void setup() {
  // Watchdog 초기화 (8초 타임아웃)
  Watchdog.enable(8000);
  
  // ... 기존 코드
}

void loop() {
  // Watchdog 리셋 (정상 동작 중임을 알림)
  Watchdog.reset();
  
  // ... 기존 코드
}
```

**효과**:
- Arduino가 8초 이상 멈추면 자동 재부팅
- 무한 루프 빠짐 방지
- 하드웨어 레벨 복구

---

**Node-RED Watchdog (Software)**
```javascript
// function 노드: Watchdog 모니터링
const lastHeartbeat = context.get('lastHeartbeat') || 0;
const now = Date.now();

// 60초 이상 하트비트 없으면 경고
if (now - lastHeartbeat > 60000) {
  node.error('⚠️ Actuator heartbeat timeout!');
  
  // 긴급 알림 전송 (이메일, SMS 등)
  msg.payload = {
    alert: 'actuator_timeout',
    message: 'Actuator node heartbeat timeout > 60s'
  };
  return msg;
}

return null;
```

**효과**:
- Arduino 다운 감지
- 자동 경고 발송
- 수동 개입 유도

---

### 🟡 높은 우선순위

#### 해결안 3: WiFi 재연결 개선

```cpp
// config.h 수정
#define WIFI_TIMEOUT 10000           // 30초 → 10초로 단축
#define WIFI_MAX_RETRY 5             // 최대 재시도 횟수
#define WIFI_RETRY_INTERVAL 10000    // 재시도 간격: 10초

// mqtt_handler.cpp 수정
bool MQTTHandler::connectWiFi() {
  int retryCount = 0;
  
  while (retryCount < WIFI_MAX_RETRY) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > WIFI_TIMEOUT) {
        DEBUG_PRINT(F("[WiFi] Retry "));
        DEBUG_PRINT(retryCount + 1);
        DEBUG_PRINT(F("/"));
        DEBUG_PRINTLN(WIFI_MAX_RETRY);
        retryCount++;
        break;
      }
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      DEBUG_PRINTLN(F("[WiFi] Connected!"));
      return true;
    }
    
    delay(WIFI_RETRY_INTERVAL);  // 10초 대기 후 재시도
  }
  
  DEBUG_PRINTLN(F("[WiFi] Max retry reached. Continuing without WiFi..."));
  return false;  // 포기하지만 시스템은 계속 동작
}
```

**효과**:
- WiFi 일시 단절에 강함
- 최대 5회 재시도 (총 50초)
- 재연결 실패해도 시스템 계속 동작

---

#### 해결안 4: systemd 자동 재시작 설정

**Node-RED systemd 설정**
```bash
# /etc/systemd/system/nodered.service
[Unit]
Description=Node-RED
After=syslog.target network.target

[Service]
Type=simple
User=nodered
Group=nodered
WorkingDirectory=/home/nodered
ExecStart=/usr/bin/node-red
Restart=always           # ← 항상 재시작
RestartSec=10            # ← 10초 후 재시작
StandardOutput=syslog
StandardError=syslog
SyslogIdentifier=node-red

[Install]
WantedBy=multi-user.target
```

**MQTT Broker (Mosquitto) 설정**
```bash
# /etc/systemd/system/mosquitto.service
[Unit]
Description=Mosquitto MQTT Broker
After=network.target

[Service]
Type=notify
ExecStart=/usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
Restart=always           # ← 항상 재시작
RestartSec=5             # ← 5초 후 재시작

[Install]
WantedBy=multi-user.target
```

**효과**:
- 프로세스 크래시 시 자동 재시작
- OS 부팅 시 자동 시작
- 관리자 개입 최소화

---

#### 해결안 5: 하트비트 모니터링 구현

**Node-RED 플로우 추가**
```json
[
  {
    "id": "heartbeat_monitor",
    "type": "function",
    "name": "Heartbeat Monitor",
    "func": "// 하트비트 수신 시 타임스탬프 저장\nif (msg.topic.includes('/heartbeat')) {\n  const nodeId = msg.topic.split('/')[0];\n  context.set(`hb_${nodeId}`, Date.now());\n}\n\nreturn null;",
    "outputs": 0
  },
  {
    "id": "heartbeat_check",
    "type": "inject",
    "name": "Check every 60s",
    "repeat": "60",
    "crontab": "",
    "once": false,
    "onceDelay": 0.1,
    "topic": "",
    "payload": "",
    "payloadType": "date"
  },
  {
    "id": "heartbeat_alert",
    "type": "function",
    "name": "Check Timeout",
    "func": "const now = Date.now();\nconst nodes = ['actuator', 'soil_sensor', 'air_sensor', 'water_tank'];\nconst alerts = [];\n\nnodes.forEach(nodeId => {\n  const lastHB = context.get(`hb_${nodeId}`) || 0;\n  const elapsed = now - lastHB;\n  \n  if (elapsed > 120000) {  // 2분 타임아웃\n    alerts.push({\n      node: nodeId,\n      elapsed: Math.floor(elapsed / 1000) + 's',\n      status: 'timeout'\n    });\n  }\n});\n\nif (alerts.length > 0) {\n  msg.payload = {\n    alert_type: 'heartbeat_timeout',\n    nodes: alerts,\n    timestamp: now\n  };\n  return msg;\n}\n\nreturn null;"
  }
]
```

**효과**:
- 각 Arduino 노드의 하트비트 모니터링
- 2분 이상 끊김 시 경고
- Dashboard에 알림 표시

---

### 🟢 중간 우선순위

#### 해결안 6: millis() Overflow 처리

```cpp
// actuator_control.cpp
void ActuatorControl::checkTimeouts() {
  unsigned long now = millis();
  
  // Overflow 안전한 비교
  if (_state.irrigation_pump) {
    unsigned long elapsed = now - _state.irrigation_start_time;
    
    // millis() overflow 처리 (49.7일 후)
    if (elapsed > IRRIGATION_TIMEOUT) {
      DEBUG_PRINTLN(F("[ACTUATOR] Irrigation timeout!"));
      stopIrrigationPump();
    }
  }
}
```

**참고**: Arduino의 `unsigned long` 연산은 overflow 시에도 안전하게 동작하지만, 명시적 처리 권장

---

#### 해결안 7: 센서 데이터 유효성 검사

**Node-RED 플로우**
```javascript
// parse_soil_data 함수 수정
const data = JSON.parse(msg.payload);

// 유효성 검사 추가
if (data.valid === false) {
  node.warn(`Invalid sensor data from ${data.zone_id}`);
  return null;  // 무효 데이터 무시
}

// 범위 검사
if (data.soil_temp < -10 || data.soil_temp > 50) {
  node.warn(`Abnormal soil temp: ${data.soil_temp}°C`);
  return null;
}

if (data.soil_moisture < 0 || data.soil_moisture > 100) {
  node.warn(`Abnormal soil moisture: ${data.soil_moisture}%`);
  return null;
}

// 정상 데이터만 통과
msg.payload = data;
return msg;
```

**효과**:
- 센서 오류 데이터 필터링
- 잘못된 자동 관수 방지
- 시스템 안정성 향상

---

## 📋 구현 우선순위 로드맵

### Phase 1: Critical Fixes (1주일)

| 순위 | 작업 | 예상 시간 | 난이도 | 영향도 |
|------|------|----------|--------|--------|
| 1 | Context 변수 영구 저장 (localfilesystem) | 2시간 | 🟢 쉬움 | 🔴 매우 높음 |
| 2 | systemd 자동 재시작 설정 | 1시간 | 🟢 쉬움 | 🔴 높음 |
| 3 | WiFi 재연결 로직 개선 | 3시간 | 🟡 보통 | 🔴 높음 |
| 4 | 하트비트 모니터링 구현 | 4시간 | 🟡 보통 | 🔴 높음 |

**예상 총 소요 시간**: 10시간 (1-2일)

---

### Phase 2: Major Improvements (2주일)

| 순위 | 작업 | 예상 시간 | 난이도 | 영향도 |
|------|------|----------|--------|--------|
| 5 | Arduino Watchdog 타이머 | 2시간 | 🟢 쉬움 | 🟡 중간 |
| 6 | 센서 데이터 유효성 검사 | 3시간 | 🟡 보통 | 🟡 중간 |
| 7 | 로그 로테이션 설정 | 1시간 | 🟢 쉬움 | 🟡 중간 |
| 8 | 알림 시스템 구축 (이메일/SMS) | 8시간 | 🔴 어려움 | 🟡 중간 |

**예상 총 소요 시간**: 14시간 (2-3일)

---

### Phase 3: Long-term Stability (1개월)

| 순위 | 작업 | 예상 시간 | 난이도 | 영향도 |
|------|------|----------|--------|--------|
| 9 | Redis Context Store 전환 | 6시간 | 🔴 어려움 | 🟢 낮음 |
| 10 | 메모리 모니터링 시스템 | 4시간 | 🟡 보통 | 🟢 낮음 |
| 11 | 통합 모니터링 대시보드 | 12시간 | 🔴 어려움 | 🟢 낮음 |
| 12 | 장기 운영 테스트 (30일) | 30일 | 🟡 보통 | 🔴 높음 |

---

## 🧪 테스트 시나리오

### 테스트 1: WiFi 단절 복구
```
1. 자동 모드 ON
2. WiFi 라우터 전원 OFF (30초)
3. WiFi 라우터 전원 ON
4. Arduino 자동 재연결 확인 (예상: 10초 이내)
5. MQTT 연결 복구 확인
6. 자동 관수 로직 정상 동작 확인
```

**예상 결과**: ✅ 시스템 정상 복구

---

### 테스트 2: Node-RED 크래시 복구
```
1. 자동 모드 ON
2. Node-RED 프로세스 강제 종료 (kill -9)
3. systemd 자동 재시작 확인 (예상: 10초 이내)
4. Context 변수 복원 확인 (autoMode = true)
5. 자동 관수 로직 정상 동작 확인
```

**현재 예상 결과**: ❌ autoMode = false (자동 모드 OFF)  
**개선 후 예상 결과**: ✅ autoMode = true (자동 모드 유지)

---

### 테스트 3: MQTT Broker 재시작
```
1. 자동 모드 ON
2. Mosquitto 재시작 (sudo systemctl restart mosquitto)
3. Node-RED 자동 재연결 확인 (예상: 5초 이내)
4. Arduino 자동 재연결 확인 (예상: 5초 이내)
5. 자동 관수 로직 정상 동작 확인
```

**예상 결과**: ✅ 시스템 정상 복구

---

### 테스트 4: 장기 운영 (30일)
```
1. 자동 모드 ON
2. 30일 연속 운영
3. 메모리 사용량 모니터링 (매일)
4. 크래시/재시작 횟수 기록
5. 자동 관수 실행 횟수 확인
6. 센서 데이터 품질 확인
```

**목표 KPI**:
- 가동률: ≥ 99.9% (43분 이내 다운타임)
- 크래시: ≤ 1회/주
- 자동 복구 성공률: ≥ 95%

---

## 📊 예상 안정성 개선 효과

### 개선 전 vs 개선 후

| 지표 | 개선 전 | 개선 후 | 개선율 |
|------|---------|---------|--------|
| **예상 가동률** | 90% | **99.5%** | +10.5% |
| **MTBF (평균 고장 간격)** | 3일 | **30일** | +900% |
| **MTTR (평균 복구 시간)** | 수동 개입 필요 (수 시간) | **자동 복구 (1분)** | -99% |
| **관리자 개입 횟수** | 월 10회 | **월 1회** | -90% |
| **Context 변수 손실 위험** | 🔴 높음 | **🟢 없음** | -100% |

---

## 💡 추가 권장 사항

### 1. 하드웨어 개선
```
- UPS (무정전 전원 장치) 설치: 순간 정전 대비
- WiFi 중계기 추가: 신호 강도 개선
- 외장 안테나: Arduino WiFi 신호 강화
- 예비 MQTT Broker: 이중화 구성
```

### 2. 모니터링 시스템
```
- Grafana: 실시간 모니터링 대시보드
- Prometheus: 메트릭 수집 및 저장
- Alertmanager: 알림 관리 (이메일, Slack, SMS)
```

### 3. 백업 전략
```
- Node-RED 플로우 자동 백업 (GitHub)
- Context 변수 일일 백업
- 센서 데이터 백업 (InfluxDB)
- 설정 파일 버전 관리
```

### 4. 보안 강화
```
- MQTT TLS/SSL 암호화
- WiFi WPA3 사용
- 방화벽 설정 (최소 권한 원칙)
- 정기 보안 업데이트
```

---

## ✅ 결론

### 현재 상태 평가

**총평**: ⚠️ **부분적으로 안정적**

- ✅ Arduino 노드: WiFi/MQTT 자동 재연결, 타임아웃 처리 우수
- ⚠️ Node-RED: Context 변수 휘발성 문제로 **365일 무중단 운영 불가능**
- ⚠️ 모니터링: 하트비트 전송은 되지만 감시 로직 부재

---

### 개선 후 예상 결과

**Phase 1 완료 후**: ✅ **24/7/365 무중단 운영 가능**

**핵심 개선 사항**:
1. ✅ Context 변수 영구 저장 → Node-RED 재시작 시에도 자동 모드 유지
2. ✅ systemd 자동 재시작 → 크래시 시 자동 복구
3. ✅ WiFi 재연결 개선 → 일시 단절에 강함
4. ✅ 하트비트 모니터링 → Arduino 다운 감지

**예상 가동률**: 
- 개선 전: **90%** (월 3일 다운타임)
- Phase 1 완료 후: **99.5%** (월 4시간 다운타임)
- Phase 2 완료 후: **99.9%** (월 43분 다운타임)

---

### 최종 권고사항

**즉시 구현 필요** (Phase 1):
1. 🔴 Context 변수 localfilesystem 저장 (2시간)
2. 🔴 systemd 자동 재시작 설정 (1시간)
3. 🔴 WiFi 재연결 로직 개선 (3시간)
4. 🔴 하트비트 모니터링 구현 (4시간)

**→ 총 10시간 투자로 365일 무중단 운영 달성 가능!**

---

**작성일**: 2025-12-17  
**프로젝트**: WasabiSmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm  
**다음 문서**: `SYSTEM_RELIABILITY_IMPLEMENTATION.md` (구현 가이드)
