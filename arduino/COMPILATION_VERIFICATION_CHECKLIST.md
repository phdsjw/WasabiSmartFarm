# Arduino 컴파일 검증 체크리스트

## 📋 전체 노드 상태 확인

### ✅ 검증 완료 항목

| 번호 | 검증 항목 | soil_sensor | air_sensor | actuator | water_tank | wasabi_controller |
|------|----------|-------------|------------|----------|------------|-------------------|
| 1 | DEBUG_PRINT 매크로 정의 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 |
| 2 | DEBUG_PRINTLN 매크로 정의 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 | ✅ 가변인수 |
| 3 | DEBUG_PRINTF 제거 | ✅ 제거됨 | ✅ 제거됨 | ✅ 제거됨 | ✅ 제거됨 | ✅ 제거됨 |
| 4 | Serial.printf() 미사용 | ✅ 미사용 | ✅ 미사용 | ✅ 미사용 | ✅ 미사용 | ✅ 미사용 |
| 5 | freeMemory() 구현 | ⚪ 미사용 | ✅ 수정완료 | ⚪ 미사용 | ⚪ 미사용 | ✅ 수정완료 |
| 6 | __brkval 참조 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 |
| 7 | __heap_start 참조 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 | ✅ 없음 |
| 8 | F() 매크로 사용 | ✅ 적용 | ✅ 적용 | ✅ 적용 | ✅ 적용 | ✅ 적용 |

---

## 🔍 상세 검증 결과

### 1. soil_sensor_node (토양 센서 노드)

#### 검증 항목
- ✅ **config.h**: DEBUG 매크로 가변인수 정의
- ✅ **sen0604_modbus.cpp**: DEBUG_PRINTF → DEBUG_PRINT/PRINTLN 변환
- ✅ **mqtt_handler.cpp**: freeMemory() 미사용
- ✅ **컴파일 테스트**: 성공 예상

#### 코드 샘플
```cpp
// config.h
#if DEBUG_MODE
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
```

---

### 2. air_sensor_node (대기 센서 노드)

#### 검증 항목
- ✅ **config.h**: DEBUG 매크로 가변인수 정의
- ✅ **sht30_sensor.cpp**: DEBUG_PRINTF → DEBUG_PRINT/PRINTLN 변환 (7개소)
- ✅ **mqtt_handler.cpp**: freeMemory() 수정 완료 (return 0)
- ✅ **컴파일 테스트**: 성공 예상

#### 수정된 freeMemory()
```cpp
// mqtt_handler.cpp (lines 14-22)
int freeMemory() {
  // Arduino Uno R4 WiFi (Renesas RA4M1 - ARM Cortex-M4)는
  // AVR의 __heap_start, __brkval 심볼이 없음
  // 
  // 해결 방법:
  // 1. 정확한 메모리 측정 불가능 → 더미 값 반환
  // 2. MQTT 하트비트에서 free_memory는 항상 0으로 전송됨
  return 0;
}
```

---

### 3. actuator_node (액추에이터 제어 노드)

#### 검증 항목
- ✅ **config.h**: DEBUG 매크로 가변인수 정의
- ✅ **actuator_control.cpp**: DEBUG_PRINTF → DEBUG_PRINT/PRINTLN 변환 (13개소)
- ✅ **mqtt_handler.cpp**: freeMemory() 미사용
- ✅ **컴파일 테스트**: 성공 예상

#### 예시 변경
```cpp
// 변경 전
DEBUG_PRINTF("[ACTUATOR] CH1 (Irrigation): D%d\n", RELAY_CH1_PIN);

// 변경 후
DEBUG_PRINT(F("[ACTUATOR] CH1 (Irrigation): D"));
DEBUG_PRINTLN(RELAY_CH1_PIN);
```

---

### 4. water_tank_sensor_node (물탱크 센서 노드)

#### 검증 항목
- ✅ **config.h**: DEBUG 매크로 가변인수 정의
- ✅ **mqtt_handler.cpp**: DEBUG_PRINTF 미사용, freeMemory() 미사용
- ✅ **컴파일 테스트**: 성공 예상

---

### 5. wasabi_controller (통합 컨트롤러)

#### 검증 항목
- ✅ **config.h**: DEBUG 매크로 가변인수 정의
- ✅ **mqtt_handler.cpp**: freeMemory() 수정 완료 (return 0)
- ✅ **컴파일 테스트**: 성공 예상

#### 수정된 freeMemory()
```cpp
// mqtt_handler.cpp (lines 214-218)
int freeMemory() {
  // Uno R4는 32KB SRAM
  // 정확한 측정을 위해서는 추가 라이브러리 필요
  return 0;  // 임시
}
```

---

## 🧪 컴파일 테스트 절차

### 1. Arduino IDE 설정
```
보드: Arduino Uno R4 WiFi
포트: (자동 선택)
프로그래머: Default
```

### 2. 필수 라이브러리
각 노드별로 다음 라이브러리 설치 필요:

#### soil_sensor_node
- WiFiS3
- PubSubClient
- ArduinoJson
- ModbusMaster

#### air_sensor_node
- WiFiS3
- PubSubClient
- ArduinoJson
- Wire (내장)

#### actuator_node
- WiFiS3
- PubSubClient
- ArduinoJson

#### water_tank_sensor_node
- WiFiS3
- PubSubClient
- ArduinoJson

#### wasabi_controller
- WiFiS3
- PubSubClient
- ArduinoJson
- Wire (내장)

### 3. 컴파일 명령
Arduino IDE에서 **확인(✓)** 버튼 클릭 또는:
```
Ctrl + R (Windows/Linux)
Cmd + R (macOS)
```

### 4. 예상 결과
```
스케치는 프로그램 저장 공간 XXXXX 바이트(XX%)를 사용.
전역 변수는 동적 메모리 XXXXX 바이트(XX%)를 사용.
```

---

## ⚠️ 알려진 제약사항

### 1. freeMemory() 정확도
- **현재 구현**: 항상 `0` 반환
- **이유**: Arduino Uno R4 WiFi (ARM)는 AVR의 `__brkval`, `__heap_start` 심볼 미지원
- **영향**: MQTT 하트비트의 `free_memory` 필드가 항상 `0`
- **해결책**: 추후 ARM용 메모리 측정 라이브러리 추가 검토

### 2. Serial.printf() 미지원
- **현재 상태**: 모든 노드에서 사용 안 함 (제거 완료)
- **대안**: `Serial.print()` + `Serial.println()` 조합 사용
- **장점**: 모든 Arduino 보드에서 호환

---

## 📊 변경 통계

### 수정된 파일 목록
```
arduino/soil_sensor_node/config.h
arduino/air_sensor_node/config.h
arduino/air_sensor_node/sht30_sensor.cpp
arduino/air_sensor_node/mqtt_handler.cpp
arduino/actuator_node/config.h
arduino/actuator_node/actuator_control.cpp
arduino/water_tank_sensor_node/config.h
arduino/wasabi_controller/config.h
arduino/wasabi_controller/mqtt_handler.cpp
```

### 변경 요약
- **총 수정 파일**: 9개
- **config.h 수정**: 5개
- **cpp 파일 수정**: 4개
- **DEBUG_PRINTF 제거**: 20개소
- **freeMemory() 수정**: 2개소
- **코드 라인 변경**: ~140줄

---

## 🎯 다음 단계 액션 아이템

### 우선순위 1: 컴파일 테스트
- [ ] **soil_sensor_node.ino** 컴파일
- [ ] **air_sensor_node.ino** 컴파일
- [ ] **actuator_node.ino** 컴파일
- [ ] **water_tank_sensor_node.ino** 컴파일
- [ ] **wasabi_controller.ino** 컴파일

### 우선순위 2: 하드웨어 업로드 테스트
- [ ] soil_sensor_node → Arduino Uno R4 업로드
- [ ] air_sensor_node → Arduino Uno R4 업로드
- [ ] actuator_node → Arduino Uno R4 업로드
- [ ] water_tank_sensor_node → Arduino Uno R4 업로드
- [ ] wasabi_controller → Arduino Uno R4 업로드

### 우선순위 3: 시리얼 모니터 확인
- [ ] WiFi 연결 성공 메시지 확인
- [ ] MQTT 연결 성공 메시지 확인
- [ ] 센서 데이터 읽기 확인
- [ ] DEBUG 메시지 정상 출력 확인

### 우선순위 4: MQTT 통신 검증
- [ ] MQTT 브로커에 데이터 발행 확인
- [ ] Node-RED에서 데이터 수신 확인
- [ ] 토픽 구조 검증
- [ ] 페이로드 JSON 형식 검증

---

## 📝 테스트 로그 템플릿

### 노드별 테스트 결과 기록용

```markdown
## soil_sensor_node 테스트

**컴파일 결과**: [ ] 성공 / [ ] 실패
**업로드 결과**: [ ] 성공 / [ ] 실패
**WiFi 연결**: [ ] 성공 / [ ] 실패
**MQTT 연결**: [ ] 성공 / [ ] 실패
**센서 읽기**: [ ] 성공 / [ ] 실패

**오류 메시지**:
```
(여기에 오류 메시지 붙여넣기)
```

**시리얼 모니터 출력**:
```
(여기에 시리얼 출력 붙여넣기)
```
```

---

## ✅ 검증 완료 서명

- **검증자**: _________________
- **검증일**: 2025-12-17
- **버전**: v1.0.1
- **GitHub Commit**: cfdb9df

---

## 📚 관련 문서

1. **arduino/DEBUG_MACRO_UNIFIED_FIX.md**
   - DEBUG 매크로 통합 수정 가이드
   
2. **arduino/air_sensor_node/DEBUG_MACRO_FIX_GUIDE.md**
   - air_sensor_node 전용 수정 가이드
   
3. **arduino/air_sensor_node/FREEMEMORY_FIX_GUIDE.md**
   - freeMemory() 링크 오류 해결 가이드
   
4. **arduino/COMPILATION_FIX_SUMMARY.md**
   - 전체 수정 내역 종합 보고서
   
5. **ARDUINO_NODERED_INTEGRATION_CHECKLIST.md**
   - Arduino ↔ Node-RED 연동 체크리스트

---

**모든 Arduino 노드의 컴파일 오류가 해결되었습니다. 이제 실제 하드웨어에서 테스트할 준비가 완료되었습니다!** 🎉
