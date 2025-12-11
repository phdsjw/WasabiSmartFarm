# 🏗️ 와사비 스마트팜 시스템 아키텍처 개요

**최종 업데이트**: 2024-12-11  
**버전**: 2.0 (분산 센서 노드 방식)

---

## 📊 시스템 구조

### 전체 구성

```
센서 노드 (18개) → MQTT Broker → Node-RED → 액추에이터 노드 (1개)
                      ↓
                  InfluxDB + Google Sheets
```

---

## 🌱 센서 노드 (18개)

### 하드웨어 구성 (각 노드)
- **MCU**: Arduino Uno R4 WiFi
- **센서**: SEN0604 (4-in-1) - Modbus RTU
- **통신**: RS485 확장보드 (DFR0259)
- **전원**: 5V/3A

### 측정 데이터
- ✅ 토양 온도 (°C)
- ✅ 토양 습도 (%)
- ✅ 토양 EC (mS/cm)
- ✅ 토양 pH

### 통신
- **프로토콜**: MQTT over WiFi
- **전송 주기**: 10초
- **MQTT Topic**: `sensor/tank{01~18}/data`

### 데이터 포맷 (JSON)
```json
{
  "tank_id": "01",
  "soil_temp": 20.5,
  "soil_moisture": 92.3,
  "soil_ec": 3.2,
  "soil_ph": 6.5,
  "timestamp": 1702284000000
}
```

---

## 🎛️ 액추에이터 노드 (1개)

### 하드웨어 구성
- **MCU**: Arduino Uno R4 WiFi
- **릴레이**: 4채널 릴레이 모듈
- **SSR**: 40A SSR × 2
- **전자개폐기**: MC-18b (2HP), MC-12b (1HP)

### 제어 대상
- ✅ 관수 펌프 (2HP) - 18개 재배상 일괄 관수
- ✅ 퇴수 펌프 (1HP) - 배수 처리
- ⚙️ (선택) 환풍기, LED, 측창 모터

### 통신
- **프로토콜**: MQTT over WiFi
- **Subscribe Topic**: 
  - `actuator/irrigation_pump/on`
  - `actuator/irrigation_pump/off`
  - `actuator/drain_pump/on`
  - `actuator/drain_pump/off`

### 명령 포맷 (JSON)
```json
{
  "command": "on",
  "duration": 240000,
  "reason": "평균 토양습도 낮음 (92.3%)"
}
```

---

## 🖥️ Node-RED 서버

### 역할
1. **데이터 수집**: 18개 센서 노드 MQTT 데이터 수신
2. **평균값 계산**: 토양 온도/습도/EC/pH 평균 계산
3. **제어 로직 실행**: 평균값 기반 관수 조건 판단
4. **제어 명령 전송**: 액추에이터 노드에 MQTT 명령 전송
5. **데이터 저장**: InfluxDB + Google Sheets
6. **대시보드**: 웹 UI 제공
7. **알림**: 이상 상황 알림

### 평균값 계산 로직
```javascript
// 18개 센서 데이터 수집
for (let tank = 1; tank <= 18; tank++) {
    토양습도_합계 += sensor_data[tank].soil_moisture;
    토양EC_합계 += sensor_data[tank].soil_ec;
    토양온도_합계 += sensor_data[tank].soil_temp;
}

// 평균값 계산
토양습도_평균 = 토양습도_합계 / 18;
토양EC_평균 = 토양EC_합계 / 18;
토양온도_평균 = 토양온도_합계 / 18;
```

---

## 💧 관수 제어 로직

### 제어 철학
**18개 센서의 평균값이 조건을 만족하면 전체 재배상을 일괄 관수**

### 관수 조건 (OR 조건)
1. ✅ **토양 습도 평균 ≤ 95%**
2. ✅ **토양 EC 평균 ≥ 5.0 μS/cm**
3. ✅ **토양 온도 평균 ≥ 22°C**

### 관수 방식
- **펌프 ON** → **수압 발생** → **18개 스프링 쿨러 자동 분사**
- **개별 솔레노이드 없음** (압력에 의한 자동 작동)
- **작동 시간**: 4분/시간
- **제어 간격**: 1시간 최소 간격

### 제어 흐름
```
1분마다 체크
    ↓
18개 센서 데이터 수집
    ↓
평균값 계산
    ↓
조건 판단 (OR)
    ↓
조건 만족 시
    ↓
마지막 관수 후 1시간 경과 확인
    ↓
관수 펌프 ON (4분)
    ↓
18개 재배상 일괄 관수
    ↓
4분 후 자동 OFF
```

---

## 📡 MQTT 토픽 구조

### 센서 데이터 (Publish)
```
sensor/tank01/data    # Tank 01 센서 데이터
sensor/tank02/data    # Tank 02 센서 데이터
...
sensor/tank18/data    # Tank 18 센서 데이터
```

### 제어 명령 (Subscribe - 액추에이터 노드)
```
actuator/irrigation_pump/on     # 관수 펌프 ON
actuator/irrigation_pump/off    # 관수 펌프 OFF
actuator/drain_pump/on          # 퇴수 펌프 ON
actuator/drain_pump/off         # 퇴수 펌프 OFF
actuator/emergency_stop         # 비상정지
```

### 상태 피드백 (Publish - 액추에이터 노드)
```
actuator/status                 # 액추에이터 상태
actuator/irrigation_pump/status # 관수 펌프 상태
actuator/drain_pump/status      # 퇴수 펌프 상태
```

### 하트비트
```
sensor/tank{01~18}/heartbeat    # 센서 노드 생존 확인
actuator/heartbeat               # 액추에이터 노드 생존 확인
```

---

## 💾 데이터 저장

### InfluxDB (시계열 데이터)
```
Measurement: sensor_data
Tags:
  - tank_id: 01~18
  - sensor_type: soil
Fields:
  - soil_temp: float
  - soil_moisture: float
  - soil_ec: float
  - soil_ph: float
Timestamp: nanosecond precision
```

```
Measurement: irrigation_log
Tags:
  - event: irrigation_start / irrigation_stop
Fields:
  - avg_soil_temp: float
  - avg_soil_moisture: float
  - avg_soil_ec: float
  - duration: integer (seconds)
  - reason: string
```

### Google Sheets (일일 리포트)
- 일일 평균값
- 관수 이력
- 센서 오류 로그

---

## 🔧 하드웨어 요약

| 구분 | 항목 | 수량 | 단가 | 합계 |
|------|------|------|------|------|
| **센서 노드** | Arduino Uno R4 WiFi | 18 | 45,000 | 810,000 |
| | RS485 확장보드 (DFR0259) | 18 | 25,000 | 450,000 |
| | 토양 센서 (SEN0604) | 18 | 120,000 | 2,160,000 |
| | 전원 공급장치 (5V/3A) | 18 | 15,000 | 270,000 |
| **액추에이터 노드** | Arduino Uno R4 WiFi | 1 | 45,000 | 45,000 |
| | 4채널 릴레이 | 1 | 15,000 | 15,000 |
| | SSR (40A) | 2 | 30,000 | 60,000 |
| | 전자개폐기 (MC-18b) | 1 | 50,000 | 50,000 |
| | 전자개폐기 (MC-12b) | 1 | 40,000 | 40,000 |
| | 전원 공급장치 (5V/3A) | 1 | 15,000 | 15,000 |
| **서버** | Raspberry Pi 4 (4GB) | 1 | 80,000 | 80,000 |
| | SD 카드 (64GB) | 2 | 15,000 | 30,000 |
| **기타** | 배선/커넥터/박스 | - | - | 200,000 |
| | 예비 부품 | - | - | 100,000 |
| **총계** | | | | **4,325,000** |

**약 432만원**

---

## 🚀 시스템 특징

### ✅ 장점
1. **독립성**: 센서 노드 간 독립 동작, 하나 고장 시 영향 최소화
2. **확장성**: 재배상 추가 시 센서 노드만 추가
3. **단순성**: 솔레노이드 없이 펌프만으로 제어
4. **신뢰성**: 평균값 기반으로 이상치 영향 최소화
5. **비용 효율**: 개별 솔레노이드 제거로 비용 절감

### ⚠️ 고려사항
1. **개별 제어 불가**: 특정 재배상만 관수 불가
2. **센서 동기화**: 최소 12개 센서 유효 필요
3. **WiFi 의존성**: WiFi 불안정 시 전체 시스템 영향
4. **평균값 편차**: 일부 재배상 과습/건조 가능성

### 🔄 향후 개선 방향
1. **LoRa 통신**: WiFi 대신 LoRa로 통신 안정성 향상
2. **개별 솔레노이드**: 필요 시 개별 제어 추가
3. **AI 예측**: 생육 데이터 기반 관수 예측
4. **모바일 앱**: 원격 모니터링 및 제어

---

## 📖 참고 문서
- [프로젝트 기획서](WASABI_SMARTFARM_PROJECT_PROPOSAL.md)
- [Step 1 가이드](STEP1_GUIDE.md)
- [GitHub 저장소](https://github.com/phdsjw/WasabiSmartFarm)

---

**작성자**: 서준원  
**버전**: 2.0  
**최종 수정**: 2024-12-11
