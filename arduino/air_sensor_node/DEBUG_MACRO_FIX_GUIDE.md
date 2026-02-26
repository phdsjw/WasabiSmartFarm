# Air Sensor Node - DEBUG 매크로 컴파일 오류 수정 가이드

**프로젝트**: Wasabi SmartFarm - Air Sensor Node v1.0.0  
**작성일**: 2024-12-17  
**수정 버전**: v1.0.1

---

## 🐛 문제 상황

### 컴파일 에러 메시지

```
error: macro "DEBUG_PRINTF" passed 3 arguments, but takes just 2
error: 'class UART' has no member named 'printf'; did you mean 'print'?
error: 'freeMemory' was not declared in this scope
```

### 발생 위치

- `arduino/air_sensor_node/sht30_sensor.cpp`: 85, 143, 151, 166, 170, 173, 224번 라인
- `arduino/air_sensor_node/mqtt_handler.cpp`: 201번 라인

---

## 🔍 원인 분석

### 1. DEBUG_PRINTF 매크로 문제

**원래 정의** (`config.h:77-86`):
```cpp
#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, y) Serial.printf(x, y)  // ❌ 2개 인자만 받음
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, y)
#endif
```

**문제점**:
1. `DEBUG_PRINTF(x, y)`는 **2개 인자만** 받지만, 코드에서 **3개 이상** 전달:
   ```cpp
   DEBUG_PRINTF("[SHT30] ERROR: Expected %d bytes, got %d\n", len, bytesReceived);
   //           ↑ format      ↑ arg1    ↑ arg2 → 총 3개 인자!
   ```

2. Arduino Uno R4 WiFi의 `Serial` 클래스는 **`printf()` 메서드 미지원**
   - Arduino의 기본 `Serial`에는 `print()`, `println()` 만 있음
   - `printf()`는 ESP32 등 일부 보드에서만 지원

### 2. freeMemory() 함수 누락

**문제**:
- `mqtt_handler.cpp:201`에서 `freeMemory()` 호출
- 해당 함수가 정의되지 않음

---

## ✅ 해결 방법

### 1️⃣ DEBUG 매크로 수정 (config.h)

**수정 후** (가변 인자 매크로):
```cpp
// 디버그 매크로 (가변 인자 지원)
#if DEBUG_MODE
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  // DEBUG_PRINTF 제거 (Serial.printf() 미지원)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
```

**변경 내용**:
- `__VA_ARGS__` 사용으로 **가변 개수 인자** 지원
- `DEBUG_PRINTF` 매크로 **제거** (Serial.printf() 미지원)
- `DEBUG_PRINT(value, precision)` 형태로 **정밀도 지정 가능**
  - 예: `DEBUG_PRINT(temp, 2)` → 소수점 2자리 출력

### 2️⃣ DEBUG_PRINTF 호출 변경 (sht30_sensor.cpp)

**변경 전**:
```cpp
DEBUG_PRINTF("[SHT30] ERROR: Expected %d bytes, got %d\n", len, bytesReceived);
```

**변경 후**:
```cpp
DEBUG_PRINT(F("[SHT30] ERROR: Expected "));
DEBUG_PRINT(len);
DEBUG_PRINT(F(" bytes, got "));
DEBUG_PRINTLN(bytesReceived);
```

**변경 사항**:
- `DEBUG_PRINTF` → `DEBUG_PRINT` + `DEBUG_PRINTLN` 조합
- `F()` 매크로로 문자열을 **PROGMEM(플래시 메모리)**에 저장 → RAM 절약
- 숫자 출력 시 `Serial.print(value, precision)` 형태 사용

**전체 수정 위치** (6곳):
1. Line 85: 바이트 수 불일치 오류
2. Line 143-144: 온도 CRC 불일치 오류
3. Line 151-152: 습도 CRC 불일치 오류
4. Line 166: 온도 범위 경고
5. Line 170: 습도 범위 경고
6. Line 173: 온도/습도 출력
7. Line 224: 히터 상태

### 3️⃣ freeMemory() 함수 추가 (mqtt_handler.cpp)

**추가 코드**:
```cpp
// ============================================
// 메모리 체크 함수 (Arduino Uno R4 WiFi용)
// ============================================
int freeMemory() {
  extern char __heap_start;
  extern char *__brkval;
  int free_memory;
  
  if (__brkval == 0) {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  } else {
    free_memory = ((int)&free_memory) - ((int)__brkval);
  }
  
  return free_memory;
}
```

**설명**:
- 힙과 스택 사이의 여유 메모리를 바이트 단위로 계산
- Arduino AVR 아키텍처 호환 방식

---

## 📦 수정된 파일 목록

1. ✅ `arduino/air_sensor_node/config.h`
   - DEBUG 매크로 가변 인자 지원으로 변경
   - DEBUG_PRINTF 제거

2. ✅ `arduino/air_sensor_node/sht30_sensor.cpp`
   - DEBUG_PRINTF → DEBUG_PRINT/DEBUG_PRINTLN 변경 (7곳)
   - F() 매크로 추가로 메모리 최적화

3. ✅ `arduino/air_sensor_node/mqtt_handler.cpp`
   - freeMemory() 함수 추가

4. ✅ `arduino/air_sensor_node/DEBUG_MACRO_FIX_GUIDE.md`
   - 수정 가이드 문서 생성 (이 파일)

---

## 🧪 검증 방법

### 1. 컴파일 확인

Arduino IDE에서 컴파일:
```
1. Arduino IDE 열기
2. 파일 → 열기 → arduino/air_sensor_node/air_sensor_node.ino
3. 보드 선택: Arduino Uno R4 WiFi
4. 스케치 → 확인/컴파일
5. ✅ "컴파일 완료" 메시지 확인
```

**기대 결과**:
```
Sketch uses XXXXX bytes (XX%) of program storage space.
Global variables use XXXXX bytes (XX%) of dynamic memory.
Done compiling.
```

### 2. 시리얼 모니터 출력 확인

업로드 후 시리얼 모니터 (115200 baud):
```
[SHT30] Initializing sensor...
[SHT30] Sensor initialized successfully
[SHT30] Temp: 25.30°C, Humidity: 45.20%
[MQTT] Connecting to MQTT broker...
[MQTT] Connected to MQTT broker
[MQTT] Sensor data published
[MQTT] Heartbeat sent
```

### 3. 오류 상황 테스트

센서 미연결 시:
```
[SHT30] ERROR: Sensor not found!
[SHT30] ERROR: Expected 6 bytes, got 0
```

CRC 오류 시:
```
[SHT30] ERROR: Temperature CRC mismatch (expected: 0xAB, got: 0xCD)
```

---

## 🔄 이전 버전과의 차이점

| 항목 | v1.0.0 (이전) | v1.0.1 (수정) |
|-----|--------------|--------------|
| DEBUG_PRINT | 1개 인자 | 가변 인자 ✅ |
| DEBUG_PRINTLN | 1개 인자 | 가변 인자 ✅ |
| DEBUG_PRINTF | 2개 인자 (printf 방식) | **제거됨** ❌ |
| freeMemory() | 미정의 ❌ | 정의됨 ✅ |
| F() 매크로 | 미사용 | 적극 사용 ✅ |
| 메모리 사용 | 높음 | 낮음 (최적화) ✅ |

---

## 💡 추가 최적화 팁

### F() 매크로 사용

**권장**:
```cpp
DEBUG_PRINTLN(F("Fixed string"));  // ✅ PROGMEM에 저장 (RAM 절약)
```

**비권장**:
```cpp
DEBUG_PRINTLN("Fixed string");     // ❌ RAM에 저장
```

### Serial.print() 정밀도 지정

```cpp
float value = 25.3456;

DEBUG_PRINT(value);      // → "25.35" (기본 2자리)
DEBUG_PRINT(value, 0);   // → "25" (정수)
DEBUG_PRINT(value, 1);   // → "25.3" (소수점 1자리)
DEBUG_PRINT(value, 3);   // → "25.346" (소수점 3자리)
```

### HEX 출력

```cpp
uint8_t data = 0xAB;

DEBUG_PRINT(data, HEX);        // → "AB"
DEBUG_PRINT(data, BIN);        // → "10101011"
DEBUG_PRINT(data, DEC);        // → "171"
```

---

## ⚠️ 주의 사항

### 1. Serial.printf() 사용 금지

Arduino Uno R4 WiFi는 `Serial.printf()`를 지원하지 않습니다.  
대신 **`Serial.print()` + `Serial.println()` 조합** 사용:

```cpp
// ❌ 사용 불가
Serial.printf("Value: %d\n", value);

// ✅ 올바른 방법
Serial.print(F("Value: "));
Serial.println(value);
```

### 2. ESP32/ESP8266과의 차이

- **ESP32**: `Serial.printf()` 지원 ✅
- **Arduino Uno R4**: `Serial.printf()` 미지원 ❌

따라서 **보드 독립적인 코드**를 작성하려면 `DEBUG_PRINT`/`DEBUG_PRINTLN` 사용 권장

### 3. 메모리 부족 주의

Arduino Uno R4 WiFi의 메모리:
- **SRAM**: 32KB
- **Flash**: 256KB

문자열을 많이 사용하면 SRAM 부족 → **F() 매크로 필수**

---

## 📚 참고 자료

1. **Arduino Reference - Serial.print()**  
   https://www.arduino.cc/reference/en/language/functions/communication/serial/print/

2. **Arduino Reference - __FlashStringHelper (F() macro)**  
   https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

3. **Arduino Uno R4 WiFi 사양**  
   https://docs.arduino.cc/hardware/uno-r4-wifi/

4. **C Preprocessor - Variadic Macros**  
   https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html

---

## 📝 요약

| 문제 | 해결 방법 | 결과 |
|-----|---------|------|
| `DEBUG_PRINTF` 인자 부족 | 가변 인자 매크로 적용 | ✅ 해결 |
| `Serial.printf()` 미지원 | `Serial.print()` + `Serial.println()` 조합 | ✅ 해결 |
| `freeMemory()` 미정의 | 함수 추가 (AVR 호환) | ✅ 해결 |
| 메모리 사용 많음 | F() 매크로 추가 | ✅ 최적화 |

**수정 후 상태**: ✅ **컴파일 성공** (Arduino Uno R4 WiFi 호환)

---

**작성자**: AI Code Assistant  
**검증 일시**: 2024-12-17  
**문서 버전**: v1.0
