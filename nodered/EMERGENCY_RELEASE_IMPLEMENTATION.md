# 긴급 정지 해제 버튼 구현 가이드

## 📋 AI 답변 검토 결과

### ✅ 올바른 부분

1. **기본 개념**: 긴급 정지 해제는 `autoMode`와 `isIrrigating` 컨텍스트 변수를 초기화하는 것이 맞습니다.
2. **안전성 고려**: 해제 후 바로 자동 모드를 활성화하지 않고 수동으로 켜게 하는 것이 안전합니다.
3. **MQTT 토픽 사용**: `actuator/emergency_release` 같은 별도 토픽을 사용하는 것이 좋습니다.

### ❌ 수정이 필요한 부분

1. **JSON 구조 오류**
   - `z`, `group`, `wires` ID가 실제 플로우와 맞지 않음
   - 좌표(`x`, `y`) 값이 부정확함
   - 실제 Node-RED 플로우에서는 import 시 자동으로 조정되지 않음

2. **context.set() 로직 불완전**
   - 추가로 초기화해야 할 변수들이 누락됨:
     - `lastIrrigationTime` (마지막 관수 시간)
     - `irrigationStartTime` (관수 시작 시간)

3. **UI 일관성 문제**
   - 기존 버튼들과 스타일이 다름
   - 버튼 크기와 배치가 맞지 않음

4. **상태 피드백 부족**
   - 긴급 정지 해제 후 사용자에게 시각적 피드백이 없음
   - Dashboard에 상태 표시가 필요함

---

## 🔍 현재 플로우 분석

### 기존 긴급 정지 버튼 (Emergency Stop)

```javascript
// Node ID: emergency_stop
msg.topic = 'actuator/emergency_stop';
msg.payload = '';
context.set('autoMode', false);
context.set('isIrrigating', false);
node.error('🚨 긴급 정지 활성화!');
return msg;
```

**위치**: 
- Tab: `main_tab` (와사비 스마트팜 메인)
- Group: `ui_group_control` (자동 관수 제어)
- Order: 6
- 좌표: x=140, y=680
- 연결: `mqtt_out_actuator` (x=1140, y=140)

### 자동 관수 제어 로직에서 사용하는 Context 변수

```javascript
// 자동 관수 로직 (Node ID: auto_irrigation_logic)
const autoMode = context.get('autoMode') || false;
const isIrrigating = context.get('isIrrigating') || false;
const lastIrrigationTime = context.get('lastIrrigationTime') || 0;
const irrigationStartTime = context.get('irrigationStartTime') || now;
```

**변수 설명**:
- `autoMode`: 자동 모드 활성화 여부 (true/false)
- `isIrrigating`: 현재 관수 중 여부 (true/false)
- `lastIrrigationTime`: 마지막 관수 완료 시간 (timestamp)
- `irrigationStartTime`: 현재 관수 시작 시간 (timestamp)

---

## ✅ 올바른 구현 방법

### 1. 긴급 정지 해제 Function 노드 코드

```javascript
// 긴급 정지 해제 로직
msg.topic = 'actuator/emergency_release';
msg.payload = 'RELEASED';

// 모든 관수 관련 컨텍스트 변수 초기화
context.set('autoMode', false);           // 자동 모드는 수동으로 다시 켜도록
context.set('isIrrigating', false);       // 관수 중 상태 해제
context.set('irrigationStartTime', 0);    // 관수 시작 시간 초기화
// lastIrrigationTime은 유지 (1시간 대기 시간 로직 보존)

// 상태 알림
node.warn('✅ 긴급 정지가 해제되었습니다. 시스템이 대기 상태로 전환됩니다.');
node.warn('⚠️ 자동 모드를 다시 활성화하려면 "자동 모드 ON" 버튼을 눌러주세요.');

return msg;
```

### 2. 완전한 Node-RED JSON 플로우

아래 JSON을 복사하여 Node-RED의 **[우측 상단 메뉴] → [Import] → [clipboard]**에 붙여넣으세요.

```json
[
    {
        "id": "ui_button_emergency_release",
        "type": "ui_button",
        "z": "main_tab",
        "name": "긴급 정지 해제",
        "group": "ui_group_control",
        "order": 7,
        "width": "3",
        "height": "1",
        "passthru": false,
        "label": "✅ 긴급 정지 해제",
        "tooltip": "시스템을 정상 상태로 복구합니다",
        "color": "#ffffff",
        "bgcolor": "#388e3c",
        "className": "",
        "icon": "",
        "payload": "RELEASE",
        "payloadType": "str",
        "topic": "emergency_release",
        "topicType": "str",
        "x": 140,
        "y": 720,
        "wires": [
            ["emergency_release_function"]
        ]
    },
    {
        "id": "emergency_release_function",
        "type": "function",
        "z": "main_tab",
        "name": "긴급 정지 해제",
        "func": "// 긴급 정지 해제 로직\nmsg.topic = 'actuator/emergency_release';\nmsg.payload = 'RELEASED';\n\n// 모든 관수 관련 컨텍스트 변수 초기화\ncontext.set('autoMode', false);           // 자동 모드는 수동으로 다시 켜도록\ncontext.set('isIrrigating', false);       // 관수 중 상태 해제\ncontext.set('irrigationStartTime', 0);    // 관수 시작 시간 초기화\n// lastIrrigationTime은 유지 (1시간 대기 시간 로직 보존)\n\n// 상태 알림\nnode.warn('✅ 긴급 정지가 해제되었습니다. 시스템이 대기 상태로 전환됩니다.');\nnode.warn('⚠️ 자동 모드를 다시 활성화하려면 \"자동 모드 ON\" 버튼을 눌러주세요.');\n\nreturn msg;",
        "outputs": 1,
        "noerr": 0,
        "initialize": "",
        "finalize": "",
        "libs": [],
        "x": 380,
        "y": 720,
        "wires": [
            ["mqtt_out_actuator", "emergency_release_status"]
        ]
    },
    {
        "id": "emergency_release_status",
        "type": "ui_text",
        "z": "main_tab",
        "group": "ui_group_control",
        "order": 8,
        "width": "6",
        "height": "1",
        "name": "긴급 정지 해제 상태",
        "label": "긴급 정지 해제",
        "format": "{{msg.payload}}",
        "layout": "row-spread",
        "className": "",
        "x": 620,
        "y": 720,
        "wires": []
    },
    {
        "id": "emergency_release_status_formatter",
        "type": "function",
        "z": "main_tab",
        "name": "상태 메시지 포맷",
        "func": "if (msg.topic === 'actuator/emergency_release') {\n    msg.payload = '✅ 긴급 정지 해제됨 - 자동 모드를 다시 켜주세요';\n} else if (msg.topic === 'actuator/emergency_stop') {\n    msg.payload = '🚨 긴급 정지 활성화됨';\n}\nreturn msg;",
        "outputs": 1,
        "noerr": 0,
        "initialize": "",
        "finalize": "",
        "libs": [],
        "x": 620,
        "y": 680,
        "wires": [
            ["emergency_release_status"]
        ]
    }
]
```

### 3. 수동 연결이 필요한 부분

JSON import 후 다음 작업을 수동으로 해주세요:

1. **emergency_release_function 노드 연결**:
   - 출력선 1: 기존 `mqtt_out_actuator` 노드에 연결
   - 출력선 2: `emergency_release_status_formatter` 노드에 연결 (선택사항)

2. **emergency_stop 노드 연결** (기존 긴급 정지 버튼):
   - 출력선에 `emergency_release_status_formatter` 추가 연결 (선택사항)

---

## 🎨 UI 레이아웃

### 제어 버튼 배치 (ui_group_control)

```
┌─────────────────────────────────────────┐
│ 자동 관수 제어                            │
├─────────────────────────────────────────┤
│ Order 1: 자동 모드 ON/OFF 스위치           │
│ Order 2: 자동 모드 상태 표시                │
│ Order 3: 관수 펌프 상태                     │
│ Order 4: 수동 관수 시작 버튼                │
│ Order 5: 수동 관수 정지 버튼                │
│ Order 6: 🚨 긴급 정지 (빨간색)             │
│ Order 7: ✅ 긴급 정지 해제 (초록색) ← 신규  │
│ Order 8: 긴급 정지 해제 상태 표시 ← 신규    │
└─────────────────────────────────────────┘
```

### 버튼 색상 가이드

| 버튼 | 배경색 | HEX | 의미 |
|------|--------|-----|------|
| 긴급 정지 | 빨간색 | `#d32f2f` | 위험, 즉시 정지 |
| 긴급 정지 해제 | 초록색 | `#388e3c` | 안전, 정상 복구 |
| 수동 관수 시작 | 파란색 | `#1976d2` | 작동 시작 |
| 수동 관수 정지 | 회색 | `#757575` | 작동 중지 |

---

## 🔄 동작 흐름도

### 긴급 정지 → 해제 → 자동 모드 재개 시나리오

```
[정상 운영]
   │
   ↓
[사용자가 🚨 긴급 정지 버튼 클릭]
   │
   ├─→ context.set('autoMode', false)
   ├─→ context.set('isIrrigating', false)
   └─→ MQTT: actuator/emergency_stop
   │
   ↓
[시스템 완전 정지 상태]
   │  (모든 관수 동작 중단)
   │  (자동 모드 비활성화)
   │
   ↓
[사용자가 ✅ 긴급 정지 해제 버튼 클릭]
   │
   ├─→ context.set('autoMode', false)      ← 여전히 false 유지
   ├─→ context.set('isIrrigating', false)  ← 확실히 false로
   ├─→ context.set('irrigationStartTime', 0)
   └─→ MQTT: actuator/emergency_release
   │
   ↓
[시스템 대기 상태]
   │  (긴급 상황 해제)
   │  (자동 모드는 꺼진 상태)
   │
   ↓
[사용자가 수동으로 "자동 모드 ON" 스위치 켬]
   │
   └─→ context.set('autoMode', true)
   │
   ↓
[정상 운영 재개]
   (자동 관수 로직 작동)
```

---

## 🧪 테스트 시나리오

### 테스트 1: 기본 긴급 정지 해제

1. ✅ 긴급 정지 버튼 클릭
2. Debug 탭 확인: `🚨 긴급 정지 활성화!` 메시지 확인
3. ✅ 긴급 정지 해제 버튼 클릭
4. Debug 탭 확인:
   - `✅ 긴급 정지가 해제되었습니다. 시스템이 대기 상태로 전환됩니다.`
   - `⚠️ 자동 모드를 다시 활성화하려면 "자동 모드 ON" 버튼을 눌러주세요.`
5. Dashboard 확인: "긴급 정지 해제 상태" 텍스트 표시 확인

### 테스트 2: 관수 중 긴급 정지 → 해제

1. 자동 모드 ON
2. 관수 시작 (자동 또는 수동)
3. 관수 중 긴급 정지 버튼 클릭
4. 펌프가 즉시 정지하는지 확인
5. 긴급 정지 해제 버튼 클릭
6. Context 변수 확인:
   - `autoMode`: false
   - `isIrrigating`: false
   - `irrigationStartTime`: 0

### 테스트 3: 해제 후 자동 모드 재개

1. 긴급 정지 → 해제
2. 자동 모드 ON 스위치 켜기
3. 10분 대기 (평균값 계산 주기)
4. 자동 관수 로직이 정상 작동하는지 확인

---

## 📊 Context 변수 상태표

| 상태 | autoMode | isIrrigating | irrigationStartTime | 설명 |
|------|----------|--------------|---------------------|------|
| 정상 운영 (자동 꺼짐) | false | false | 0 | 기본 상태 |
| 정상 운영 (자동 켜짐) | true | false | 0 | 자동 모드 활성화 |
| 자동 관수 중 | true | true | timestamp | 관수 진행 중 |
| 긴급 정지 | false | false | 0 | 강제 정지 |
| 긴급 정지 해제 | false | false | 0 | 대기 상태 |

---

## ⚠️ 주의사항

### 1. lastIrrigationTime 보존
```javascript
// ❌ 잘못된 구현
context.set('lastIrrigationTime', 0);  // 1시간 대기 시간 로직이 깨짐

// ✅ 올바른 구현
// lastIrrigationTime은 건드리지 않음
// 긴급 정지 해제 후에도 1시간 간격 로직은 유지됨
```

### 2. MQTT 토픽 일관성
```javascript
// 긴급 정지 관련 토픽
'actuator/emergency_stop'     // 긴급 정지
'actuator/emergency_release'  // 긴급 정지 해제

// 관수 관련 토픽
'actuator/irrigation_pump/on'   // 관수 시작
'actuator/irrigation_pump/off'  // 관수 정지
```

### 3. Arduino 연동
Arduino 코드에서 `actuator/emergency_release` 토픽을 subscribe하여 처리해야 합니다:

```cpp
// Arduino actuator_node에 추가 필요
if (strcmp(topic, "actuator/emergency_release") == 0) {
    // 모든 액추에이터 안전 정지
    digitalWrite(RELAY_CH1_PIN, LOW);  // 관수 펌프 OFF
    digitalWrite(RELAY_CH2_PIN, LOW);  // 배수 펌프 OFF
    digitalWrite(RELAY_CH3_PIN, LOW);  // 팬 OFF
    digitalWrite(RELAY_CH4_PIN, LOW);  // LED OFF
    
    Serial.println("✅ 긴급 정지 해제 - 시스템 대기 상태");
}
```

---

## 🚀 추가 개선 제안

### 1. 긴급 정지 확인 다이얼로그
```javascript
// ui_button에 confirm 추가 (Node-RED에서 지원하지 않으므로 별도 구현 필요)
// 대신 ui_notification 사용
```

### 2. 긴급 정지 이력 로깅
```javascript
// InfluxDB에 긴급 정지 이벤트 기록
const logData = {
    measurement: 'emergency_events',
    fields: {
        event_type: 'emergency_stop',
        timestamp: Date.now()
    }
};
```

### 3. 긴급 정지 카운터
```javascript
// Dashboard에 긴급 정지 횟수 표시
let emergencyCount = context.get('emergencyCount') || 0;
emergencyCount++;
context.set('emergencyCount', emergencyCount);
```

---

## ✅ 검증 체크리스트

- [ ] JSON import 성공
- [ ] 긴급 정지 해제 버튼이 Dashboard에 표시됨
- [ ] 긴급 정지 → 해제 흐름이 정상 동작
- [ ] Debug 메시지가 올바르게 출력됨
- [ ] Context 변수가 정확히 초기화됨
- [ ] MQTT 메시지가 올바른 토픽으로 전송됨
- [ ] 해제 후 자동 모드 재개 가능
- [ ] UI 상태 표시가 정확함

---

## 📚 관련 문서

- **ARDUINO_NODERED_INTEGRATION_CHECKLIST.md**: Arduino ↔ Node-RED 연동 체크리스트
- **flows_improved_ui.json**: 현재 Node-RED 플로우
- **arduino/actuator_node/**: 액추에이터 제어 Arduino 코드

---

**작성일**: 2025-12-17  
**버전**: v1.0.0  
**프로젝트**: WasabiSmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm
