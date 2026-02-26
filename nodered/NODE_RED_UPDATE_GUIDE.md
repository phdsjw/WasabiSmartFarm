# Node-RED Flow 수정 가이드

**버전**: v1.0.0  
**작성일**: 2025-12-27  
**대상**: flows_wasabi_03.json

---

## 개요

센서 노드 재구성에 따라 Node-RED Flow와 Dashboard UI를 수정해야 합니다.

---

## 1. 하트비트 모니터링 수정 (21개 노드)

### 변경 사항

#### AS-IS (기존)
- 대기 센서 노드: Zone 01 (1개)
- 토양 센서 노드: Tank 01~18 (18개)
- 물탱크 센서 노드: 1개
- 액추에이터 노드: 1개
- 총 21개

#### TO-BE (변경 후)
- 대기+토양 통합 노드: Zone 13, 14, 15 (3개)
- 토양 센서 노드: Tank 01~12, 16~18 (15개)
- 물탱크 센서 노드: 1개
- 수위 센서 노드: 1개 (신규)
- 액추에이터 노드: 1개
- 총 21개

### 수정할 MQTT 토픽

#### 제거할 토픽
```
sensor/air/zone01/heartbeat
sensor/soil/tank13/heartbeat
sensor/soil/tank14/heartbeat
sensor/soil/tank15/heartbeat
```

#### 추가할 토픽
```
sensor/combined/zone13/heartbeat
sensor/combined/zone14/heartbeat
sensor/combined/zone15/heartbeat
sensor/water_level/heartbeat
```

### 하트비트 노드 리스트 (21개)

| 번호 | 노드 이름 | MQTT 토픽 |
|------|-----------|-----------|
| 01 | 토양 Tank 01 | `sensor/soil/tank01/heartbeat` |
| 02 | 토양 Tank 02 | `sensor/soil/tank02/heartbeat` |
| 03 | 토양 Tank 03 | `sensor/soil/tank03/heartbeat` |
| 04 | 토양 Tank 04 | `sensor/soil/tank04/heartbeat` |
| 05 | 토양 Tank 05 | `sensor/soil/tank05/heartbeat` |
| 06 | 토양 Tank 06 | `sensor/soil/tank06/heartbeat` |
| 07 | 토양 Tank 07 | `sensor/soil/tank07/heartbeat` |
| 08 | 토양 Tank 08 | `sensor/soil/tank08/heartbeat` |
| 09 | 토양 Tank 09 | `sensor/soil/tank09/heartbeat` |
| 10 | 토양 Tank 10 | `sensor/soil/tank10/heartbeat` |
| 11 | 토양 Tank 11 | `sensor/soil/tank11/heartbeat` |
| 12 | 토양 Tank 12 | `sensor/soil/tank12/heartbeat` |
| 13 | 대기+토양 Zone 13 | `sensor/combined/zone13/heartbeat` |
| 14 | 대기+토양 Zone 14 | `sensor/combined/zone14/heartbeat` |
| 15 | 대기+토양 Zone 15 | `sensor/combined/zone15/heartbeat` |
| 16 | 토양 Tank 16 | `sensor/soil/tank16/heartbeat` |
| 17 | 토양 Tank 17 | `sensor/soil/tank17/heartbeat` |
| 18 | 토양 Tank 18 | `sensor/soil/tank18/heartbeat` |
| 19 | 물탱크 | `sensor/water_tank/heartbeat` |
| 20 | 수위 센서 | `sensor/water_level/heartbeat` |
| 21 | 액추에이터 | `actuator/heartbeat` |

---

## 2. 센서 데이터 수신 노드 추가

### 2.1 대기 센서 데이터 (Zone 13, 14, 15)

#### MQTT 구독 노드 추가
- **토픽**: `sensor/air/zone13/data`, `sensor/air/zone14/data`, `sensor/air/zone15/data`
- **QoS**: 0
- **타입**: JSON

#### 데이터 형식
```json
{
  "zone_id": "13",
  "air_temp": 22.5,
  "air_humidity": 65.2,
  "timestamp": 123456
}
```

### 2.2 토양 센서 데이터 (Tank 13, 14, 15)

#### MQTT 구독 노드 추가
- **토픽**: `sensor/soil/tank13/data`, `sensor/soil/tank14/data`, `sensor/soil/tank15/data`
- **QoS**: 0
- **타입**: JSON

#### 데이터 형식
```json
{
  "tank_id": "13",
  "soil_temp": 20.1,
  "soil_moisture": 85.5,
  "soil_ec": 1250.0,
  "soil_ph": 6.50,
  "timestamp": 123456
}
```

### 2.3 수위 센서 데이터 (Node 20)

#### MQTT 구독 노드 추가
- **토픽**: `sensor/water_level/data`
- **QoS**: 0
- **타입**: JSON

#### 데이터 형식
```json
{
  "node_id": "20",
  "distance_cm": 45.3,
  "water_level_percent": 54.7,
  "timestamp": 123456
}
```

---

## 3. Dashboard UI 수정

### 3.1 대기 센서 그룹

#### AS-IS
- Zone 01 대기 온도
- Zone 01 대기 습도

#### TO-BE
- Zone 13 대기 온도
- Zone 13 대기 습도
- Zone 14 대기 온도
- Zone 14 대기 습도
- Zone 15 대기 온도
- Zone 15 대기 습도

### 3.2 토양 센서 그룹

#### 기존 유지
- Tank 01~12, 16~18 (15개)

#### 추가
- Tank 13, 14, 15 (3개)

### 3.3 수위 센서 그룹 (신규)

#### 추가할 UI 요소
- **수위 게이지**: `water_level_percent` (0-100%)
- **거리 표시**: `distance_cm` (cm)
- **상태 표시**: 정상/경고/위험

#### 경고 임계값
- 위험: < 20%
- 경고: 20% ~ 40%
- 정상: > 40%

---

## 4. Context Storage 업데이트

### 추가할 Context 변수
- `zone13_air_temp`
- `zone13_air_humidity`
- `zone14_air_temp`
- `zone14_air_humidity`
- `zone15_air_temp`
- `zone15_air_humidity`
- `water_level_percent`
- `water_distance_cm`

---

## 5. 수정 작업 순서

1. **백업**: 기존 `flows_wasabi_03.json` 백업
2. **하트비트 수정**:
   - Zone 01 삭제
   - Zone 13, 14, 15 추가
   - 수위 센서 추가
3. **센서 데이터 수신 노드 추가**:
   - 대기 Zone 13, 14, 15
   - 토양 Tank 13, 14, 15
   - 수위 센서
4. **Dashboard UI 수정**:
   - 대기 센서 그룹 재구성
   - 토양 센서 추가
   - 수위 센서 그룹 생성
5. **테스트**:
   - MQTT 메시지 수신 확인
   - Dashboard 표시 확인
   - 하트비트 타임아웃 테스트

---

## 6. 테스트 방법

### 6.1 MQTT 메시지 발행 테스트

```bash
# 대기 센서 테스트 (Zone 13)
mosquitto_pub -h 192.168.0.100 -t sensor/air/zone13/data \
  -m '{"zone_id":"13","air_temp":22.5,"air_humidity":65.2,"timestamp":123456}'

# 토양 센서 테스트 (Tank 13)
mosquitto_pub -h 192.168.0.100 -t sensor/soil/tank13/data \
  -m '{"tank_id":"13","soil_temp":20.1,"soil_moisture":85.5,"soil_ec":1250.0,"soil_ph":6.50,"timestamp":123456}'

# 수위 센서 테스트
mosquitto_pub -h 192.168.0.100 -t sensor/water_level/data \
  -m '{"node_id":"20","distance_cm":45.3,"water_level_percent":54.7,"timestamp":123456}'

# 하트비트 테스트 (Zone 13)
mosquitto_pub -h 192.168.0.100 -t sensor/combined/zone13/heartbeat \
  -m '{"zone_id":"13","tank_id":"13","node_type":"combined","uptime":123456,"wifi_rssi":-45,"free_memory":0,"timestamp":123456}'
```

### 6.2 Dashboard 확인
1. Node-RED Dashboard 접속: `http://localhost:1880/ui`
2. 각 센서 데이터 표시 확인
3. 하트비트 상태 (녹색/빨강) 확인

---

## 7. 예상 작업 시간

| 작업 항목 | 예상 시간 |
|----------|----------|
| 하트비트 노드 수정 | 30분 |
| 센서 데이터 수신 노드 추가 | 30분 |
| Dashboard UI 수정 | 1시간 |
| 테스트 및 검증 | 30분 |
| **총 예상 시간** | **2.5시간** |

---

## 8. 주의 사항

1. **백업 필수**: 수정 전 반드시 백업
2. **단계별 테스트**: 각 수정 후 즉시 테스트
3. **MQTT 토픽 확인**: 오타 주의
4. **타임아웃 설정**: 하트비트 2분 (120초)
5. **Context Storage**: 영구 저장 설정 확인

---

## 작성자

서준원

## 날짜

2025-12-27
