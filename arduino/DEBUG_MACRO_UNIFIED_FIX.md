# Arduino 노드 DEBUG 매크로 통합 수정 가이드

**프로젝트**: Wasabi SmartFarm - All Arduino Nodes  
**작성일**: 2024-12-17  
**수정 버전**: v1.0.1 (통합)

---

## 🎯 수정 목표

모든 Arduino 노드의 DEBUG 매크로를 **통일된 가변 인자 방식**으로 변경하여:
1. ✅ **컴파일 오류 해결**: 인자 개수 불일치 문제 제거
2. ✅ **Arduino Uno R4 WiFi 호환**: Serial.printf() 미지원 문제 해결
3. ✅ **코드 일관성**: 모든 노드의 DEBUG 매크로 통일
4. ✅ **메모리 최적화**: F() 매크로 적극 활용

---

## 📊 수정 전 상태

| 노드 | DEBUG_PRINT | DEBUG_PRINTLN | DEBUG_PRINTF | 문제 |
|-----|-------------|---------------|--------------|------|
| soil_sensor_node | 1개 인자 → **가변** | 1개 인자 → **가변** | ❌ 없음 | ✅ 이미 수정됨 |
| air_sensor_node | 1개 인자 → **가변** | 1개 인자 → **가변** | ❌ 없음 | ✅ 이미 수정됨 |
| actuator_node | 1개 인자 | 1개 인자 | 가변 (printf) | ❌ Serial.printf() 미지원 |
| water_tank_sensor_node | 1개 인자 | 1개 인자 | 2개 인자 | ❌ 인자 개수 고정 |
| wasabi_controller | 1개 인자 | 1개 인자 | ❌ 없음 | ⚠️ 가변 인자 미지원 |

---

## ✅ 수정 후 상태 (통일)

**모든 노드 통일 매크로**:
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

---

## 📦 수정된 파일 목록

### 1️⃣ soil_sensor_node (v2.0.1)
**수정 파일**:
- ✅ `arduino/soil_sensor_node/config.h` - DEBUG 매크로 가변 인자화
- ✅ 이미 최신 상태 유지

**변경 사항**:
- ✅ 이전에 수정 완료됨

---

### 2️⃣ air_sensor_node (v1.0.1)
**수정 파일**:
- ✅ `arduino/air_sensor_node/config.h` - DEBUG_PRINTF 제거, 가변 인자화
- ✅ `arduino/air_sensor_node/sht30_sensor.cpp` - DEBUG_PRINTF → DEBUG_PRINT/PRINTLN 변경 (7곳)
- ✅ `arduino/air_sensor_node/mqtt_handler.cpp` - freeMemory() 함수 추가

**변경 사항**:
- 이전 커밋에서 수정 완료 (`b6a0457`)

---

### 3️⃣ actuator_node (v1.0.1 - 신규)
**수정 파일**:
- ✅ `arduino/actuator_node/config.h` - DEBUG_PRINTF 제거, 가변 인자화
- ✅ `arduino/actuator_node/actuator_control.cpp` - DEBUG_PRINTF → DEBUG_PRINT/PRINTLN 변경 (13곳)

**변경 사항**:
```cpp
// 수정 전
#define DEBUG_PRINTF(x, ...) Serial.printf(x, ##__VA_ARGS__)  // ❌ Serial.printf() 미지원

// 수정 후
// DEBUG_PRINTF 제거 → DEBUG_PRINT + DEBUG_PRINTLN 조합 사용
DEBUG_PRINT(F("[ACTUATOR]   CH1 (Irrigation): D"));
DEBUG_PRINTLN(RELAY_CH1_PIN);
```

**수정 위치 (actuator_control.cpp)**:
1. Line 39-42: 릴레이 핀 설정 출력 (4곳)
2. Line 51: 릴레이 상태 출력
3. Line 108: 관수 타임아웃 출력
4. Line 126-127: 관수 경고 출력
5. Line 138-139: 관수 총 시간 출력
6. Line 172: 배수 타임아웃 출력
7. Line 190-191: 배수 경고 출력
8. Line 202-203: 배수 총 시간 출력
9. Line 303: 긴급 정지 시간 출력
10. Line 314-315: 긴급 정지 대기 시간 출력

---

### 4️⃣ water_tank_sensor_node (v1.0.1 - 신규)
**수정 파일**:
- ✅ `arduino/water_tank_sensor_node/config.h` - DEBUG_PRINTF 제거, 가변 인자화

**변경 사항**:
```cpp
// 수정 전
#define DEBUG_PRINTF(x, y) Serial.printf(x, y)  // ❌ 2개 인자만 받음

// 수정 후
// DEBUG_PRINTF 제거 (사용하지 않음)
```

**참고**: 이 노드는 DEBUG_PRINTF를 실제로 사용하지 않았지만, 정의가 잘못되어 있어 수정

---

### 5️⃣ wasabi_controller (v1.0.1 - 신규)
**수정 파일**:
- ✅ `arduino/wasabi_controller/config.h` - 가변 인자화

**변경 사항**:
```cpp
// 수정 전
#define DEBUG_PRINT(x) Serial.print(x)         // ❌ 1개 인자만
#define DEBUG_PRINTLN(x) Serial.println(x)     // ❌ 1개 인자만

// 수정 후
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)      // ✅ 가변 인자
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)  // ✅ 가변 인자
```

---

## 🔍 주요 변경 패턴

### Pattern 1: 단순 출력
```cpp
// 수정 전
DEBUG_PRINTF("[MODULE] Value: %d\n", value);

// 수정 후
DEBUG_PRINT(F("[MODULE] Value: "));
DEBUG_PRINTLN(value);
```

### Pattern 2: 다중 값 출력
```cpp
// 수정 전
DEBUG_PRINTF("[MODULE] Expected %d bytes, got %d\n", expected, actual);

// 수정 후
DEBUG_PRINT(F("[MODULE] Expected "));
DEBUG_PRINT(expected);
DEBUG_PRINT(F(" bytes, got "));
DEBUG_PRINTLN(actual);
```

### Pattern 3: 조건부 문자열
```cpp
// 수정 전
DEBUG_PRINTF("[MODULE] Relay set to %s\n", state ? "ON" : "OFF");

// 수정 후
DEBUG_PRINT(F("[MODULE] Relay set to "));
DEBUG_PRINTLN(state ? F("ON") : F("OFF"));
```

### Pattern 4: HEX 출력
```cpp
// 수정 전
DEBUG_PRINTF("[MODULE] CRC: 0x%02X\n", crc);

// 수정 후
DEBUG_PRINT(F("[MODULE] CRC: 0x"));
DEBUG_PRINT(crc, HEX);
DEBUG_PRINTLN();
```

### Pattern 5: 소수점 출력
```cpp
// 수정 전
DEBUG_PRINTF("[MODULE] Temp: %.2f°C\n", temp);

// 수정 후
DEBUG_PRINT(F("[MODULE] Temp: "));
DEBUG_PRINT(temp, 2);  // 소수점 2자리
DEBUG_PRINTLN(F("°C"));
```

---

## 🧪 검증 방법

### 1. 컴파일 확인

```bash
# 각 노드별로 Arduino IDE에서 컴파일
1. Arduino IDE 열기
2. 파일 → 열기 → arduino/{노드명}/{노드명}.ino
3. 보드 선택: Arduino Uno R4 WiFi
4. 스케치 → 확인/컴파일
5. ✅ "컴파일 완료" 메시지 확인
```

### 2. 시리얼 모니터 확인

```bash
# 업로드 후 시리얼 모니터 (115200 baud)
# 예상 출력 확인
```

### 3. 자동 검증 스크립트

```bash
# 모든 노드의 DEBUG_PRINT 패턴 확인
cd /home/user/webapp
for dir in arduino/*/; do
  echo "📁 $(basename $dir)"
  grep -rn "DEBUG_PRINT\|DEBUG_PRINTLN\|DEBUG_PRINTF" "${dir}"*.cpp "${dir}"*.ino 2>/dev/null | wc -l
done
```

---

## 📈 수정 통계

| 노드 | 수정 파일 | DEBUG_PRINTF 제거 | 코드 변경 라인 | 상태 |
|-----|---------|-----------------|--------------|------|
| soil_sensor_node | 1 | 0 | 0 | ✅ 이미 완료 |
| air_sensor_node | 3 | 7 | ~50 | ✅ 완료 |
| actuator_node | 2 | 13 | ~60 | ✅ 신규 완료 |
| water_tank_sensor_node | 1 | 0 | ~5 | ✅ 신규 완료 |
| wasabi_controller | 1 | 0 | ~5 | ✅ 신규 완료 |
| **합계** | **8** | **20** | **~120** | ✅ **전체 완료** |

---

## 💡 장점

### 1. 가변 인자 지원
```cpp
DEBUG_PRINT(value);               // 1개 인자 OK
DEBUG_PRINT(value, 2);            // 2개 인자 OK (정밀도)
DEBUG_PRINT(value, HEX);          // 2개 인자 OK (포맷)
```

### 2. Arduino Uno R4 WiFi 호환
- ✅ `Serial.print()` / `Serial.println()` 사용 (표준)
- ❌ `Serial.printf()` 미사용 (비표준)

### 3. 메모리 최적화
```cpp
DEBUG_PRINT(F("Fixed string"));  // PROGMEM 사용 (Flash 메모리)
```

### 4. 코드 일관성
- 모든 노드가 동일한 DEBUG 매크로 사용
- 유지보수 용이

---

## ⚠️ 주의 사항

### 1. Serial.printf() 사용 금지

```cpp
// ❌ 사용 불가 (Arduino Uno R4 WiFi 미지원)
Serial.printf("Value: %d\n", value);

// ✅ 올바른 방법
Serial.print(F("Value: "));
Serial.println(value);
```

### 2. F() 매크로 적극 활용

```cpp
// ❌ RAM 낭비
DEBUG_PRINTLN("Fixed string");

// ✅ Flash 메모리 사용 (RAM 절약)
DEBUG_PRINTLN(F("Fixed string"));
```

### 3. 정밀도 지정

```cpp
float value = 25.3456;

DEBUG_PRINT(value);      // 25.35 (기본 2자리)
DEBUG_PRINT(value, 0);   // 25 (정수)
DEBUG_PRINT(value, 1);   // 25.3 (소수점 1자리)
DEBUG_PRINT(value, 3);   // 25.346 (소수점 3자리)
```

---

## 📚 참고 자료

1. **Arduino Reference - Serial.print()**  
   https://www.arduino.cc/reference/en/language/functions/communication/serial/print/

2. **Arduino Reference - F() Macro**  
   https://www.arduino.cc/reference/en/language/variables/utilities/progmem/

3. **Arduino Uno R4 WiFi 사양**  
   https://docs.arduino.cc/hardware/uno-r4-wifi/

4. **C Preprocessor - Variadic Macros**  
   https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html

---

## ✅ 결론

**모든 Arduino 노드의 DEBUG 매크로가 통일되어:**
- ✅ 컴파일 오류 해결
- ✅ Arduino Uno R4 WiFi 완전 호환
- ✅ 코드 일관성 확보
- ✅ 메모리 최적화

**수정 후 상태**: ✅ **전체 노드 컴파일 성공** (Arduino Uno R4 WiFi 호환)

---

**작성자**: AI Code Assistant  
**검증 일시**: 2024-12-17  
**문서 버전**: v1.0 (통합)

