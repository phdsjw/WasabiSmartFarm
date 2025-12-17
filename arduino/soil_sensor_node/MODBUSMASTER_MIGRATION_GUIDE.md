# ModbusMaster 라이브러리 마이그레이션 가이드

**작성자**: 서준원  
**버전**: v2.0.0  
**날짜**: 2024-12-17

---

## 📋 목차

1. [문제 상황](#문제-상황)
2. [해결 방법](#해결-방법)
3. [라이브러리 설치](#라이브러리-설치)
4. [코드 변경 사항](#코드-변경-사항)
5. [검증 방법](#검증-방법)

---

## ❌ 문제 상황

### ArduinoModbus 라이브러리 호환성 문제

Arduino IDE에서 `soil_sensor_node.ino` 컴파일 시 다음 오류 발생:

```
WARNING: library ArduinoModbus claims to run on megaavr, samd, mbed_nano, mbed_portenta, mbed_opta architecture(s) and may be incompatible with your current board which runs on renesas_uno architecture(s).

In file included from c:\Users\q\Documents\Arduino\libraries\ArduinoModbus\src\libmodbus\modbus-private.h:23:0,
                 from c:\Users\q\Documents\Arduino\libraries\ArduinoModbus\src\libmodbus\modbus-tcp.cpp:80:
c:\users\q\appdata\local\arduino15\packages\arduino\tools\arm-none-eabi-gcc\7-2017q4\arm-none-eabi\include\sys\time.h:263:17: error: field 'it_interval' has incomplete type 'timeval'
  struct timeval it_interval; /* timer interval */
                 ^~~~~~~~~~~
...
exit status 1
Compilation error: exit status 1
```

### 원인 분석

- **ArduinoModbus 라이브러리**가 `renesas_uno` 아키텍처를 공식 지원하지 않음
- Arduino Uno R4 WiFi는 `renesas_uno` 아키텍처 사용
- `sys/time.h`와 `fd_set` 관련 타입 정의 오류 발생
- **이 문제는 라이브러리 자체의 호환성 문제로 해결 불가능**

---

## ✅ 해결 방법

### ModbusMaster 라이브러리로 전환

**ModbusMaster 라이브러리**는 Arduino Uno R4 WiFi와 완벽히 호환됩니다.

**장점:**
- ✅ Arduino Uno R4 WiFi (renesas_uno) 완벽 지원
- ✅ 간단한 API
- ✅ RS485 송수신 제어 지원
- ✅ 안정적인 Modbus RTU 통신
- ✅ 활발한 커뮤니티 지원

---

## 📦 라이브러리 설치

### 1. Arduino IDE에서 설치

```
Arduino IDE → 스케치 → 라이브러리 포함하기 → 라이브러리 관리...
```

**검색**: `ModbusMaster`  
**작성자**: Doc Walker  
**버전**: 2.0.1 이상  
**설치** 버튼 클릭

### 2. 수동 설치 (선택)

GitHub에서 다운로드:
```
https://github.com/4-20ma/ModbusMaster
```

압축 해제 후 Arduino 라이브러리 폴더에 복사:
```
Documents/Arduino/libraries/ModbusMaster/
```

---

## 🔄 코드 변경 사항

### 1. sen0604_modbus.h

#### 변경 전 (ArduinoModbus)

```cpp
#include <Arduino.h>
#include <ArduinoModbus.h>
#include "config.h"

class SEN0604Modbus {
public:
    SEN0604Modbus();
    bool begin();
    // ...
    
private:
    bool _initialized;
    unsigned long _lastReadTime;
};
```

#### 변경 후 (ModbusMaster)

```cpp
#include <Arduino.h>
#include <ModbusMaster.h>
#include "config.h"

class SEN0604Modbus {
public:
    SEN0604Modbus();
    bool begin();
    
    // RS485 송수신 제어 (ModbusMaster 콜백용)
    static void preTransmission();
    static void postTransmission();
    // ...
    
private:
    ModbusMaster _modbus;  // ModbusMaster 객체
    bool _initialized;
    unsigned long _lastReadTime;
};
```

**주요 변경점:**
- `#include <ArduinoModbus.h>` → `#include <ModbusMaster.h>`
- `ModbusMaster _modbus;` 객체 추가
- `preTransmission()`, `postTransmission()` 정적 메서드 추가 (RS485 제어용)

---

### 2. sen0604_modbus.cpp

#### 변경 전 (ArduinoModbus)

```cpp
#include "sen0604_modbus.h"

SEN0604Modbus::SEN0604Modbus() {
    _initialized = false;
    _lastReadTime = 0;
}

bool SEN0604Modbus::begin() {
    pinMode(RS485_TX_ENABLE_PIN, OUTPUT);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    if (!ModbusRTUClient.begin(MODBUS_BAUDRATE)) {
        return false;
    }
    
    ModbusRTUClient.setTimeout(MODBUS_TIMEOUT);
    _initialized = true;
    return true;
}

uint16_t SEN0604Modbus::readHoldingRegister(uint16_t address) {
    digitalWrite(RS485_TX_ENABLE_PIN, HIGH);
    delayMicroseconds(100);
    
    ModbusRTUClient.requestFrom(MODBUS_SLAVE_ID, HOLDING_REGISTERS, address, 1);
    
    delayMicroseconds(100);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    if (ModbusRTUClient.available()) {
        return ModbusRTUClient.read();
    }
    
    return 0xFFFF;
}
```

#### 변경 후 (ModbusMaster)

```cpp
#include "sen0604_modbus.h"

SEN0604Modbus::SEN0604Modbus() {
    // ModbusMaster는 기본 생성자만 제공 (파라미터 없음)
    _initialized = false;
    _lastReadTime = 0;
}

// RS485 송신 전 콜백 (TX Enable ON)
void SEN0604Modbus::preTransmission() {
    digitalWrite(RS485_TX_ENABLE_PIN, HIGH);
    delayMicroseconds(100);
}

// RS485 수신 전 콜백 (TX Enable OFF)
void SEN0604Modbus::postTransmission() {
    delayMicroseconds(100);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
}

bool SEN0604Modbus::begin() {
    pinMode(RS485_TX_ENABLE_PIN, OUTPUT);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    // Serial1 시작 (Modbus RTU는 Serial1 사용)
    Serial1.begin(MODBUS_BAUDRATE);
    
    // ModbusMaster 시작 (Slave ID와 Serial 포트 설정)
    _modbus.begin(MODBUS_SLAVE_ID, Serial1);
    
    // RS485 송수신 제어 콜백 등록
    _modbus.preTransmission(preTransmission);
    _modbus.postTransmission(postTransmission);
    
    _initialized = true;
    return true;
}

uint16_t SEN0604Modbus::readHoldingRegister(uint16_t address) {
    // Modbus Function Code 0x03: Read Holding Registers
    uint8_t result = _modbus.readHoldingRegisters(address, 1);
    
    if (result == _modbus.ku8MBSuccess) {
        return _modbus.getResponseBuffer(0);
    } else {
        DEBUG_PRINT(F("[SEN0604] Modbus error: 0x"));
        DEBUG_PRINTLN(result, HEX);
        return 0xFFFF;
    }
}
```

**주요 변경점:**

1. **생성자**:
   - `: _modbus(MODBUS_SLAVE_ID)` 초기화 리스트 추가

2. **RS485 제어 콜백**:
   - `preTransmission()`: 송신 전 TX Enable ON
   - `postTransmission()`: 수신 전 TX Enable OFF

3. **begin() 메서드**:
   - `Serial1.begin(MODBUS_BAUDRATE)` 명시적 호출
   - `_modbus.begin()` 호출
   - `_modbus.preTransmission()`, `_modbus.postTransmission()` 콜백 등록

4. **readHoldingRegister() 메서드**:
   - `_modbus.readHoldingRegisters(address, count)` 사용
   - `_modbus.getResponseBuffer(index)` 로 데이터 읽기
   - `_modbus.ku8MBSuccess` 로 성공 확인
   - 수동 RS485 제어 제거 (콜백이 자동 처리)

5. **readHoldingRegisters() 메서드**:
   - 동일한 방식으로 여러 레지스터 읽기
   - for 루프로 버퍼 복사

---

## 📊 API 비교표

| 기능 | ArduinoModbus | ModbusMaster |
|-----|---------------|--------------|
| 라이브러리 헤더 | `#include <ArduinoModbus.h>` | `#include <ModbusMaster.h>` |
| 객체 생성 | `ModbusRTUClient` (글로벌) | `ModbusMaster _modbus(slaveID)` |
| 초기화 | `ModbusRTUClient.begin(baudrate)` | `_modbus.begin(baudrate, Serial1)` |
| 레지스터 읽기 | `ModbusRTUClient.requestFrom(id, type, addr, count)` | `_modbus.readHoldingRegisters(addr, count)` |
| 응답 읽기 | `ModbusRTUClient.read()` | `_modbus.getResponseBuffer(index)` |
| 성공 확인 | `ModbusRTUClient.available()` | `result == _modbus.ku8MBSuccess` |
| RS485 제어 | 수동 (digitalWrite) | 콜백 (preTransmission, postTransmission) |
| renesas_uno 지원 | ❌ 미지원 | ✅ 지원 |

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
╠══════════════════════════════════════╣
║  WiFi SSID : YOUR_WIFI_SSID          ║
║  MQTT      : 192.168.0.100:1883      ║
║  Topic     : sensor/soil/tank01/data ║
╠══════════════════════════════════════╣
║  Modbus    : RTU (4800 baud)         ║
║  Slave ID  : 1                       ║
╚══════════════════════════════════════╝

[SEN0604] Initializing Modbus RTU with ModbusMaster...
[SEN0604] Modbus RTU initialized successfully
[SEN0604] Sensor connected!
[MQTT] Connecting to broker...
[MQTT] Connected!

┌─────────────────────────────────────┐
│       Soil Sensor Data              │
├─────────────────────────────────────┤
│ Soil Temperature  : 25.3 °C        │
│ Soil Moisture     : 45.2 %         │
│ Soil EC           : 1234.5 μS/cm   │
│ Soil pH           : 6.85           │
└─────────────────────────────────────┘

[MQTT] Publishing to: sensor/soil/tank01/data
```

---

## 🔧 문제 해결

### Modbus 통신 오류

**증상:**
```
[SEN0604] Modbus error: 0xE2
```

**원인 및 해결:**

| 에러 코드 | 의미 | 해결 방법 |
|----------|------|----------|
| 0xE0 | INVALID_SLAVE_ID | Slave ID 확인 (기본값: 1) |
| 0xE1 | INVALID_FUNCTION | Function Code 확인 |
| 0xE2 | RESPONSE_TIMEOUT | 배선 확인, 보드레이트 확인 (4800) |
| 0xE3 | INVALID_CRC | RS485 신호 품질 확인 |

**체크리스트:**
1. RS485 배선: A-A, B-B 확인
2. 보드레이트: `config.h`에서 `MODBUS_BAUDRATE` 확인 (기본 4800)
3. Slave ID: SEN0604 설정값 확인 (기본 1)
4. RS485 TX Enable 핀: D2 연결 확인
5. 전원: SEN0604는 12V 전원 필요

---

## 📝 변경 이력

### v2.0.0 (2024-12-17)
- ArduinoModbus → ModbusMaster 라이브러리로 전환
- Arduino Uno R4 WiFi (renesas_uno) 완전 지원
- RS485 송수신 제어 콜백 추가
- 컴파일 오류 완전 해결

### v1.0.0 (2024-12-11)
- 초기 버전 (ArduinoModbus 사용)
- ❌ Arduino Uno R4 WiFi 호환성 문제

---

## 🔗 관련 문서

- `README.md` - 토양 센서 노드 전체 가이드
- `COMPILATION_FIX_GUIDE.md` - 컴파일 오류 해결 가이드
- `SERIAL1_GUIDE.md` - Serial vs Serial1 사용 가이드
- `CHANGELOG.md` - 변경 이력

---

## 📧 참고 자료

- **ModbusMaster 라이브러리**: https://github.com/4-20ma/ModbusMaster
- **Arduino Forum**: https://forum.arduino.cc/t/uno-r4-modbus-rs485-library/1152751
- **SEN0604 Wiki**: https://wiki.dfrobot.com/SKU_SEN0604

---

**작성자**: 서준원  
**프로젝트**: WASABI SmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm
