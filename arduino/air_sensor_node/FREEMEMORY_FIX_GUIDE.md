# Air Sensor Node - freeMemory() 링크 오류 수정 가이드

**프로젝트**: Wasabi SmartFarm - Air Sensor Node  
**작성일**: 2024-12-17  
**수정 버전**: v1.0.2

---

## 🐛 문제 상황

### 링크 오류 메시지

```
mqtt_handler.cpp.o: In function `freeMemory()':
mqtt_handler.cpp:26: undefined reference to `__brkval'
mqtt_handler.cpp:26: undefined reference to `__heap_start'
collect2.exe: error: ld returned 1 exit status
exit status 1
Compilation error: exit status 1
```

### 발생 위치

- `arduino/air_sensor_node/mqtt_handler.cpp`: Line 14-26 (freeMemory 함수)
- `arduino/air_sensor_node/mqtt_handler.cpp`: Line 211 (freeMemory 호출)

---

## 🔍 원인 분석

### 1. Arduino Uno R4 WiFi 아키텍처 차이

**Arduino Uno (AVR)**:
- ✅ AVR 아키텍처 (ATmega328P)
- ✅ `__heap_start`, `__brkval` 심볼 존재
- ✅ AVR libc 메모리 관리

**Arduino Uno R4 WiFi (ARM)**:
- ❌ ARM Cortex-M4 (Renesas RA4M1)
- ❌ `__heap_start`, `__brkval` 심볼 **없음**
- ❌ AVR libc가 아닌 ARM 표준 라이브러리

### 2. freeMemory() 함수 구현 문제

**원래 구현** (AVR 전용):
```cpp
int freeMemory() {
  extern char __heap_start;      // ❌ ARM에서 미정의
  extern char *__brkval;         // ❌ ARM에서 미정의
  int free_memory;
  
  if (__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  
  return free_memory;
}
```

**문제점**:
- `__heap_start`: AVR 힙 시작 주소 (ARM에 없음)
- `__brkval`: AVR 힙 끝 주소 (ARM에 없음)
- ARM Cortex-M4는 다른 메모리 관리 방식 사용

---

## ✅ 해결 방법

### Option 1: 더미 값 반환 (채택)

**수정된 코드**:
```cpp
int freeMemory() {
  // Arduino Uno R4 WiFi (Renesas RA4M1 - ARM Cortex-M4)는
  // AVR의 __heap_start, __brkval 심볼이 없음
  // 
  // 해결 방법:
  // 1. 정확한 메모리 측정 불가능 → 더미 값 반환
  // 2. 또는 외부 라이브러리 필요 (예: MemoryFree 라이브러리)
  //
  // 현재는 하트비트 전송을 위해 0 반환 (참고용)
  return 0;  // 메모리 측정 불가 (ARM 아키텍처)
}
```

**장점**:
- ✅ 컴파일 오류 해결
- ✅ 기존 MQTT 하트비트 구조 유지
- ✅ 추가 라이브러리 불필요

**단점**:
- ⚠️ 실제 메모리 사용량 측정 불가
- ⚠️ 항상 0 반환

### Option 2: free_memory 필드 제거

```cpp
// mqtt_handler.cpp의 publishHeartbeat() 수정
doc["zone_id"] = ZONE_ID;
doc["status"] = "alive";
doc["uptime"] = millis();
doc["rssi"] = WiFi.RSSI();
// doc["free_memory"] = freeMemory();  // ← 제거

// freeMemory() 함수 자체를 삭제
```

**장점**:
- ✅ 불필요한 함수 제거
- ✅ JSON 크기 감소

**단점**:
- ⚠️ 기존 Node-RED가 `free_memory` 필드를 기대할 수 있음
- ⚠️ 하트비트 구조 변경

### Option 3: 외부 라이브러리 사용 (미채택)

Arduino의 MemoryFree 라이브러리는 AVR 전용이므로 ARM에서 작동하지 않습니다.

---

## 📦 수정된 파일 목록

1. ✅ `arduino/air_sensor_node/mqtt_handler.cpp`
   - `freeMemory()` 함수를 더미 값(0) 반환으로 변경

2. ✅ `arduino/air_sensor_node/FREEMEMORY_FIX_GUIDE.md`
   - 수정 가이드 문서 생성 (이 파일)

---

## 🧪 검증 방법

### 1. 컴파일 확인

```bash
# Arduino IDE에서 컴파일
1. Arduino IDE 열기
2. 파일 → 열기 → arduino/air_sensor_node/air_sensor_node.ino
3. 보드 선택: Arduino Uno R4 WiFi
4. 스케치 → 확인/컴파일
5. ✅ "컴파일 완료" 메시지 확인 (링크 오류 없음)
```

**기대 결과**:
```
Sketch uses XXXXX bytes (XX%) of program storage space.
Global variables use XXXXX bytes (XX%) of dynamic memory.
Done compiling.
```

### 2. 시리얼 모니터 확인

업로드 후 시리얼 모니터 (115200 baud):
```
[MQTT] Heartbeat sent
```

### 3. MQTT 메시지 확인

MQTT 클라이언트로 하트비트 확인:
```bash
mosquitto_sub -h 192.168.0.100 -t "sensor/air/zone01/heartbeat" -v
```

**예상 메시지**:
```json
{
  "zone_id": "01",
  "status": "alive",
  "uptime": 12345,
  "rssi": -45,
  "free_memory": 0
}
```

---

## 🔄 이전 버전과의 차이점

| 항목 | v1.0.1 (이전) | v1.0.2 (수정) |
|-----|--------------|--------------|
| freeMemory() | AVR 방식 (오류) | ARM 호환 (더미값) |
| __heap_start | 사용 (미정의 오류) | **미사용** ✅ |
| __brkval | 사용 (미정의 오류) | **미사용** ✅ |
| 컴파일 | ❌ 링크 오류 | ✅ 정상 |
| free_memory 값 | (측정 불가) | 0 (고정값) |

---

## 💡 Arduino Uno R4 메모리 관리

### Arduino Uno R4 WiFi 사양

- **MCU**: Renesas RA4M1 (ARM Cortex-M4 48MHz)
- **SRAM**: 32KB
- **Flash**: 256KB
- **아키텍처**: ARM Cortex-M4 (32-bit)

### 메모리 측정의 어려움

ARM Cortex-M4는 다음과 같은 이유로 간단한 메모리 측정이 어렵습니다:

1. **복잡한 메모리 레이아웃**: 힙/스택이 명확히 분리되지 않음
2. **동적 메모리 관리**: `malloc()` 사용 시 힙이 동적으로 증가
3. **OS 레이어**: FreeRTOS 등 사용 시 더 복잡함
4. **표준 라이브러리 차이**: AVR libc와 ARM libc의 구조 차이

### 대안

실제 메모리 사용량을 확인하려면:
- Arduino IDE의 컴파일 메시지 확인
- 정적 메모리 사용량 계산
- 외부 디버거 사용 (예: J-Link)

---

## ⚠️ 주의 사항

### 1. free_memory 값은 참고용

```cpp
// ⚠️ 실제 메모리 사용량이 아님!
doc["free_memory"] = freeMemory();  // 항상 0
```

Node-RED에서 `free_memory` 필드를 사용한다면:
- 값이 항상 0임을 인지
- 메모리 모니터링 기능 비활성화 권장

### 2. 다른 노드도 동일한 문제 가능

다음 노드들도 `freeMemory()` 함수를 사용한다면 동일한 수정 필요:
- ✅ `soil_sensor_node`: 확인 필요
- ✅ `water_tank_sensor_node`: 확인 필요
- ✅ `actuator_node`: 확인 필요

### 3. AVR 보드와의 호환성

만약 Arduino Uno (AVR)와 Uno R4 (ARM)를 동시에 사용한다면:
```cpp
#if defined(__AVR__)
  // AVR 방식
  extern char __heap_start;
  extern char *__brkval;
  // ...
#elif defined(ARDUINO_ARCH_RENESAS)
  // ARM 방식 (더미값)
  return 0;
#endif
```

---

## 📚 참고 자료

1. **Arduino Uno R4 WiFi 공식 문서**  
   https://docs.arduino.cc/hardware/uno-r4-wifi/

2. **Renesas RA4M1 데이터시트**  
   https://www.renesas.com/us/en/products/microcontrollers-microprocessors/ra-cortex-m-mcus/ra4m1-32-bit-microcontrollers-48mhz-arm-cortex-m4-and-lcd-controller-and-cap-touch-hmi

3. **ARM Cortex-M4 Technical Reference Manual**  
   https://developer.arm.com/documentation/100166/0001

4. **Arduino Memory Management**  
   https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

---

## 📝 요약

| 문제 | 원인 | 해결 방법 | 결과 |
|-----|-----|---------|------|
| 링크 오류 | `__heap_start`, `__brkval` 미정의 | `freeMemory()` 더미값 반환 | ✅ 컴파일 성공 |
| AVR vs ARM | 아키텍처 차이 | ARM 호환 코드 사용 | ✅ Uno R4 지원 |
| 메모리 측정 불가 | ARM 메모리 관리 복잡 | `free_memory = 0` 고정값 | ⚠️ 참고용 |

**수정 후 상태**: ✅ **컴파일 성공** (Arduino Uno R4 WiFi 호환)

---

**작성자**: AI Code Assistant  
**검증 일시**: 2024-12-17  
**문서 버전**: v1.0

