# 토양 센서 노드 컴파일 오류 해결 가이드

**작성자**: 서준원  
**버전**: v1.0.0  
**날짜**: 2024-12-17

---

## 📋 목차

1. [문제 증상](#문제-증상)
2. [발생 원인](#발생-원인)
3. [해결 방법](#해결-방법)
4. [추가 경고 해결](#추가-경고-해결)
5. [검증 방법](#검증-방법)

---

## ❌ 문제 증상

Arduino IDE에서 `soil_sensor_node.ino` 컴파일 시 다음 오류 발생:

```
C:\Users\q\Desktop\WasabiSmartFarm-main\WasabiSmartFarm-main\arduino\soil_sensor_node\soil_sensor_node.ino:153:34: error: macro "DEBUG_PRINT" passed 2 arguments, but takes just 1
     DEBUG_PRINT(data.soil_temp, 1);
                                  ^
```

**추가 오류 라인:**
- Line 153: `DEBUG_PRINT(data.soil_temp, 1);`
- Line 157: `DEBUG_PRINT(data.soil_moisture, 1);`
- Line 161: `DEBUG_PRINT(data.soil_ec, 1);`
- Line 165: `DEBUG_PRINTLN(data.soil_ph, 2);`

**경고 메시지:**
```
WARNING: library ArduinoModbus claims to run on megaavr, samd, mbed_nano, mbed_portenta, mbed_opta architecture(s) and may be incompatible with your current board which runs on renesas_uno architecture(s).
```

---

## 🔍 발생 원인

### 1. DEBUG 매크로 정의 문제

**이전 코드 (`config.h`):**
```cpp
#if DEBUG_MODE
  #define DEBUG_PRINT(x)    Serial.print(x)      // ❌ 1개 인자만 허용
  #define DEBUG_PRINTLN(x)  Serial.println(x)    // ❌ 1개 인자만 허용
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif
```

**문제점:**
- 매크로가 고정된 1개 인자 `(x)`만 받도록 정의됨
- `Serial.print(value, precision)` 형태로 소수점 자릿수 지정 시 2개 인자 필요
- 코드에서 `DEBUG_PRINT(data.soil_temp, 1)`처럼 2개 인자 사용 → 컴파일 에러

### 2. ArduinoModbus 라이브러리 경고

- ArduinoModbus 라이브러리가 공식적으로 `renesas_uno` 아키텍처를 지원 목록에 포함하지 않음
- **실제로는 정상 작동함** (경고일 뿐 에러 아님)

---

## ✅ 해결 방법

### 1. config.h 수정 (DEBUG 매크로 가변 인자 지원)

**수정된 코드:**
```cpp
// ============================================
// 디버그 설정
// ============================================
#define DEBUG_MODE       true   // 시리얼 디버그 출력 활성화

#if DEBUG_MODE
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)    // ✅ 가변 인자 지원
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)  // ✅ 가변 인자 지원
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
```

**변경 사항:**
- `(x)` → `(...)`
- `x` → `__VA_ARGS__`

**효과:**
- 1개 인자: `DEBUG_PRINT("Hello")` ✅
- 2개 인자: `DEBUG_PRINT(value, precision)` ✅
- 3개 이상: `DEBUG_PRINT(a, b, c, ...)` ✅

### 2. 사용 예시

```cpp
// ✅ 정상 동작하는 코드
DEBUG_PRINT(F("│ Soil Temperature  : "));
DEBUG_PRINT(data.soil_temp, 1);          // 소수점 1자리
DEBUG_PRINTLN(F(" °C       │"));

DEBUG_PRINT(F("│ Soil Moisture     : "));
DEBUG_PRINT(data.soil_moisture, 1);      // 소수점 1자리
DEBUG_PRINTLN(F(" %        │"));

DEBUG_PRINT(F("│ Soil pH           : "));
DEBUG_PRINTLN(data.soil_ph, 2);          // 소수점 2자리
```

---

## ⚠️ 추가 경고 해결

### ArduinoModbus 라이브러리 경고

**경고 메시지:**
```
WARNING: library ArduinoModbus claims to run on megaavr, samd, mbed_nano, mbed_portenta, mbed_opta architecture(s) and may be incompatible with your current board which runs on renesas_uno architecture(s).
```

**해결 방법:**

1. **무시해도 됨 (권장)**
   - 이 경고는 단순히 공식 지원 목록에 `renesas_uno`가 없다는 의미
   - 실제로 Arduino Uno R4 WiFi에서 정상 작동 확인됨
   - 컴파일 및 업로드에 문제없음

2. **라이브러리 버전 확인**
   ```
   Arduino IDE → 도구 → 라이브러리 관리 → ArduinoModbus
   권장 버전: 1.0.9 이상
   ```

3. **보드 패키지 업데이트**
   ```
   Arduino IDE → 보드 매니저 → Arduino UNO R4 Boards
   최신 버전으로 업데이트 (1.2.0 이상)
   ```

---

## ✅ 검증 방법

### 1. 컴파일 테스트

1. Arduino IDE에서 프로젝트 열기
2. 보드 선택: `Arduino Uno R4 WiFi`
3. 포트 선택
4. `검증` 버튼 클릭 (Ctrl+R)

**예상 결과:**
```
스케치는 프로그램 저장 공간 XXXXX 바이트(XX%)를 사용. 최대 262144 바이트.
전역 변수는 동적 메모리 XXXX바이트(X%)를 사용, XXXXX바이트의 지역변수가 남음. 최대 32768 바이트.
```

### 2. 업로드 및 실행 테스트

1. Arduino 보드 연결
2. `업로드` 버튼 클릭 (Ctrl+U)
3. 시리얼 모니터 열기 (Ctrl+Shift+M)
4. Baudrate 115200 설정

**예상 출력:**
```
╔══════════════════════════════════════╗
║   WASABI SmartFarm - Soil Sensor    ║
║            Node v1.0.0               ║
╠══════════════════════════════════════╣
║  Author    : 서준원                   ║
║  Date      : 2024-12-11              ║
║  Tank ID   : 01                      ║
...
┌─────────────────────────────────────┐
│       Soil Sensor Data              │
├─────────────────────────────────────┤
│ Soil Temperature  : 25.3 °C        │
│ Soil Moisture     : 45.2 %         │
│ Soil EC           : 1234.5 μS/cm   │
│ Soil pH           : 6.85           │
└─────────────────────────────────────┘
```

---

## 🔧 기타 참고 사항

### 1. Serial vs Serial1 사용

- **Serial (USB)**: 디버그 출력용 (`Serial.begin(115200)`)
- **Serial1 (D0/D1)**: Modbus RTU 통신용 (라이브러리 자동 사용)

**참고 문서**: `SERIAL1_GUIDE.md`

### 2. 다른 노드의 매크로 정의

**Air Sensor Node (`air_sensor_node/config.h`):**
```cpp
#define DEBUG_PRINT(x) Serial.print(x)      // ❌ 미수정
#define DEBUG_PRINTLN(x) Serial.println(x)  // ❌ 미수정
```

**Water Tank Node (`water_tank_sensor_node/config.h`):**
```cpp
#define DEBUG_PRINT(x) Serial.print(x)            // ❌ 미수정
#define DEBUG_PRINTLN(x) Serial.println(x)        // ❌ 미수정
#define DEBUG_PRINTF(x, y) Serial.printf(x, y)    // ⚠️ 2개 인자만 지원
```

**Actuator Node (`actuator_node/config.h`):**
```cpp
#define DEBUG_PRINT(x) Serial.print(x)                      // ❌ 미수정
#define DEBUG_PRINTLN(x) Serial.println(x)                  // ❌ 미수정
#define DEBUG_PRINTF(x, ...) Serial.printf(x, ##__VA_ARGS__)  // ✅ 가변 인자 지원
```

**권장 사항**: 모든 노드의 `config.h`를 아래 형식으로 통일

```cpp
#if DEBUG_MODE
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif
```

---

## 📝 변경 이력

### v1.0.0 (2024-12-17)
- DEBUG 매크로 가변 인자 지원 추가
- ArduinoModbus 경고 해결 가이드 추가
- 컴파일 오류 수정

---

## 🔗 관련 문서

- `README.md` - 토양 센서 노드 전체 가이드
- `SERIAL1_GUIDE.md` - Serial vs Serial1 사용 가이드
- `CHANGELOG.md` - 변경 이력

---

## 📧 문의

문제가 지속되거나 추가 도움이 필요하시면 프로젝트 관리자에게 문의하세요.

**작성자**: 서준원  
**프로젝트**: WASABI SmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm
