# Arduino Uno R4 WiFi - Serial vs Serial1 사용 가이드

**작성자**: 서준원  
**날짜**: 2024-12-11  
**버전**: v1.0

---

## 🎯 결론: **수정 필요 여부**

### ✅ **현재 코드는 수정 필요 없음**

ArduinoRS485/ArduinoModbus 라이브러리가 Arduino Uno R4 WiFi에서 **자동으로 Serial1을 사용**하도록 설계되어 있습니다.

**단, 다음 사항을 확인하세요**:
1. ✅ ArduinoRS485 라이브러리 버전 **1.0.6 이상**
2. ✅ ArduinoModbus 라이브러리 버전 **1.0.9 이상**
3. ✅ Arduino UNO R4 Board Package **최신 버전**

---

## 📌 Arduino Uno R4 WiFi의 Serial 포트

Arduino Uno R4 WiFi는 **2개의 독립적인 하드웨어 Serial 포트**를 제공합니다:

| Serial 객체 | 물리적 연결 | 핀 번호 | 주 용도 |
|------------|-----------|--------|---------|
| **`Serial`** | USB-C 포트 | USB | PC 통신 (시리얼 모니터, 디버깅) |
| **`Serial1`** | UART 핀 | D0(RX), D1(TX) | 외부 장치 통신 (센서, 모듈) |

### 📖 공식 문서 인용

> "The UNO R4 WiFi board features 2 separate hardware serial ports.  
> - One port is exposed via USB-C®, and uses `Serial` object  
> - One is exposed via RX/TX pins, and uses `Serial1` object"

**출처**: [Arduino UNO R4 WiFi Cheat Sheet](https://docs.arduino.cc/tutorials/uno-r4-wifi/cheat-sheet)

---

## 🔍 우리 프로젝트 상황

### 1️⃣ **토양 센서 노드 (SEN0604 - RS485 Modbus)**

#### 하드웨어 연결

```
Arduino Uno R4 WiFi          RS485 확장보드 (DFR0259)          SEN0604 센서
├─ D0 (RX) ─────────────┼─── RS485 RX ──────────────┼─── A
├─ D1 (TX) ─────────────┼─── RS485 TX ──────────────┼─── B
├─ D2 (DE/RE) ──────────┼─── DE/RE
├─ 5V ──────────────────┼─── VCC ───────────────────┼─── VCC
└─ GND ─────────────────┼─── GND ───────────────────┼─── GND
```

#### 현재 코드 분석

**sen0604_modbus.cpp**:
```cpp
bool SEN0604Modbus::begin() {
    // RS485 TX Enable 핀 설정
    pinMode(RS485_TX_ENABLE_PIN, OUTPUT);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    // Modbus RTU 시작 (Serial1, 보드레이트)
    if (!ModbusRTUClient.begin(MODBUS_BAUDRATE)) {
        return false;
    }
    
    // ... 나머지 코드
}
```

#### ✅ **판정: 수정 불필요**

**이유**:
1. ArduinoModbus 라이브러리는 내부적으로 **Serial1을 자동 사용**
2. 라이브러리 내부 코드:
   ```cpp
   #if defined(ARDUINO_ARCH_RENESAS)
     #define SERIAL_PORT_HARDWARE Serial1
   #endif
   ```
3. 주석에 "Serial1 사용" 명시되어 있음

---

### 2️⃣ **대기 센서 노드 (SHT30 - I2C)**

#### 현재 코드

```cpp
void setup() {
    Serial.begin(115200);  // USB 디버깅용
    
    // SHT30은 I2C 통신 (Wire 라이브러리)
    if (!airSensor.begin()) {
        Serial.println("센서 초기화 실패");
    }
}
```

#### ✅ **판정: 수정 불필요**

**이유**:
- `Serial`은 USB 디버깅 용도로만 사용
- I2C 통신은 `Wire` 라이브러리 사용 (A4/A5 핀)
- UART(D0/D1)를 사용하지 않음

---

### 3️⃣ **수조 센서 노드 (DS18B20 + 아날로그 센서)**

#### 현재 코드

```cpp
void setup() {
    Serial.begin(115200);  // USB 디버깅용
    
    // DS18B20은 1-Wire 통신 (OneWire 라이브러리)
    // 아날로그 센서는 A0, A1, A2 핀 사용
}
```

#### ✅ **판정: 수정 불필요**

**이유**:
- `Serial`은 USB 디버깅 용도로만 사용
- 1-Wire 통신은 D4 핀 사용
- 아날로그 센서는 ADC 핀 사용
- UART(D0/D1)를 사용하지 않음

---

## 🚨 주의사항 (중요!)

### 1️⃣ **USB와 Serial1 동시 사용 시 충돌**

**문제 상황**:
```cpp
// ❌ 이렇게 사용하면 문제 발생 가능
void setup() {
    Serial.begin(115200);   // USB 디버깅
    Serial1.begin(4800);    // UART 통신
}

void loop() {
    // Serial.println()과 Modbus 통신 동시 사용
    Serial.println("Debug: Reading sensor...");
    ModbusRTUClient.requestFrom(1, 0x0000, 4);  // Serial1 사용
}
```

**해결 방법**:

#### 방법 A: USB 케이블 제거 (권장)
```cpp
void setup() {
    // Serial 초기화 제거 또는 주석 처리
    // Serial.begin(115200);  
    
    Serial1.begin(4800);  // UART만 사용
}
```

#### 방법 B: 다른 핀 사용 (SoftwareSerial)
```cpp
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3);  // RX, TX를 D2, D3으로 변경

void setup() {
    Serial.begin(115200);     // USB 디버깅
    mySerial.begin(4800);     // D2/D3로 Modbus 통신
}
```

**주의**: SoftwareSerial은 성능이 낮으므로 **테스트 용도로만 사용**

---

### 2️⃣ **ArduinoRS485 라이브러리 버전**

#### ❌ **구버전 (1.0.5 이하) - 오류 발생**

```
error: 'SERIAL_PORT_HARDWARE' was not declared in this scope
error: 'A6' was not declared in this scope
```

**원인**: 라이브러리가 Arduino Uno R4 WiFi를 지원하지 않음

#### ✅ **신버전 (1.0.6 이상) - 정상 작동**

Arduino IDE > Library Manager에서 업데이트:
```
ArduinoRS485 버전 1.0.6 이상
ArduinoModbus 버전 1.0.9 이상
```

---

### 3️⃣ **flush() 버그 (Arduino Uno R4 WiFi)**

#### 문제 상황

Arduino Uno R4 WiFi의 HardwareSerial 라이브러리에 `flush()` 함수 버그 존재:

```cpp
// Serial1.flush()가 제대로 작동하지 않음
Serial1.write(data, length);
Serial1.flush();  // ❌ 버그: 전송 완료를 기다리지 않음
```

**영향**: RS485 통신에서 CRC 오류 발생 가능

#### 해결 방법 (임시)

```cpp
// flush() 대신 delay() 사용
Serial1.write(data, length);
delay(10);  // 10ms 대기 (보드레이트에 따라 조정)
```

**또는**:

```cpp
// 전송 완료 대기
while (Serial1.availableForWrite() < 64) {
    delay(1);
}
```

**참고**: Arduino 팀이 수정 중이므로 향후 보드 패키지 업데이트 시 해결될 예정

---

## 📝 수정이 필요한 경우 예시

### ❌ **잘못된 코드 패턴**

#### 패턴 1: Serial을 UART 통신에 사용

```cpp
// ❌ 잘못된 코드
void setup() {
    Serial.begin(4800);  // USB 포트를 4800으로 설정
    
    // Modbus가 Serial(USB)을 사용하려고 시도 → 실패
    ModbusRTUClient.begin(Serial, 4800);
}
```

**문제**: USB 포트는 PC 통신용이므로 외부 장치와 통신 불가

#### 패턴 2: Serial1 명시적 초기화 누락

```cpp
// ⚠️ 주의: 라이브러리가 자동 초기화하지만, 명시하는 것이 안전
void setup() {
    // Serial1.begin(4800);  // 주석 처리됨
    
    ModbusRTUClient.begin(4800);  // 내부에서 Serial1.begin() 호출
}
```

**해결**: 명시적으로 Serial1 초기화 (권장)

---

### ✅ **올바른 코드 패턴**

#### 패턴 1: USB 디버깅 + UART 통신

```cpp
void setup() {
    // USB 디버깅 (115200 baud)
    Serial.begin(115200);
    Serial.println("Starting Modbus...");
    
    // UART 통신 (4800 baud)
    Serial1.begin(4800);
    
    // Modbus 초기화 (Serial1 사용)
    if (!ModbusRTUClient.begin(4800)) {
        Serial.println("Modbus init failed!");
    }
}
```

#### 패턴 2: UART 통신만 사용 (USB 없음)

```cpp
void setup() {
    // USB 디버깅 비활성화
    // Serial.begin(115200);  // 주석 처리
    
    // UART 통신만 사용
    Serial1.begin(4800);
    
    // Modbus 초기화
    ModbusRTUClient.begin(4800);
    
    // LED로 상태 표시 (디버깅 대신)
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    if (모드버스_읽기_성공) {
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN, LOW);
    }
}
```

---

## 🔧 권장 코드 구조

### 토양 센서 노드 (최종 권장 버전)

```cpp
/*
 * Arduino Uno R4 WiFi - 토양 센서 노드
 * SEN0604 Modbus RTU (RS485)
 */

#include "config.h"
#include "sen0604_modbus.h"
#include "mqtt_handler.h"

SEN0604Modbus soilSensor;
MQTTHandler mqttHandler;

void setup() {
    // ==========================================
    // Option 1: USB 디버깅 활성화 (개발 중)
    // ==========================================
    Serial.begin(115200);
    Serial.println("[START] Soil Sensor Node");
    
    // ==========================================
    // Option 2: USB 디버깅 비활성화 (배포 시)
    // ==========================================
    // Serial.begin(115200);  // 주석 처리
    // pinMode(LED_BUILTIN, OUTPUT);  // LED로 상태 표시
    
    // ==========================================
    // UART 초기화 (D0/D1, Serial1)
    // ==========================================
    // Note: ModbusRTUClient.begin()이 내부적으로 
    //       Serial1.begin()을 호출하지만, 
    //       명시적으로 초기화하는 것을 권장
    
    // WiFi 연결
    mqttHandler.connectWiFi();
    
    // Modbus 센서 초기화 (Serial1 자동 사용)
    if (!soilSensor.begin()) {
        Serial.println("[ERROR] Sensor init failed!");
        // LED 빠른 깜빡임으로 오류 표시
        while(1) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(100);
        }
    }
    
    Serial.println("[OK] Setup complete");
}

void loop() {
    mqttHandler.loop();
    
    // 10초마다 센서 데이터 읽기
    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 10000) {
        lastRead = millis();
        
        SoilSensorData data = soilSensor.readSensorData();
        
        if (data.valid) {
            Serial.print("Temp: "); Serial.print(data.soil_temp);
            Serial.print(", Moisture: "); Serial.println(data.soil_moisture);
            
            mqttHandler.publishSensorData(data);
        } else {
            Serial.println("[ERROR] Failed to read sensor");
        }
    }
    
    delay(100);
}
```

---

## 📊 테스트 체크리스트

### ✅ USB 디버깅 활성화 모드

```bash
# 1. USB 케이블 연결
# 2. 시리얼 모니터 115200 baud로 설정
# 3. 업로드 후 시리얼 모니터 확인

[START] Soil Sensor Node
[WIFI] Connecting...
[WIFI] Connected! IP: 192.168.0.100
[MODBUS] Initializing...
[SEN0604] Sensor connected!
[OK] Setup complete
Temp: 20.5, Moisture: 96.3
[MQTT] Published successfully
```

### ✅ UART 통신 확인

```bash
# 1. RS485 테스터 또는 오실로스코프로 D0/D1 핀 확인
# 2. 보드레이트: 4800 bps
# 3. 데이터 형식: 8N1
# 4. Modbus RTU 프레임 확인

TX (D1) ─┬─┬─┬─┬─┬─┐
         └─┘ └─┘ └─┘  → Modbus Request
RX (D0) ┌─┐     ┌─┐
        └─┴─────┴─┘    ← Modbus Response
```

---

## 🎓 핵심 요약

### ✅ **해야 할 것**

1. ✅ Arduino UNO R4 Board Package 최신 버전 사용
2. ✅ ArduinoRS485 라이브러리 1.0.6 이상
3. ✅ ArduinoModbus 라이브러리 1.0.9 이상
4. ✅ USB 디버깅과 UART 통신을 명확히 구분
5. ✅ 배포 시 USB 디버깅 비활성화 고려

### ❌ **하지 말아야 할 것**

1. ❌ Serial을 UART 통신에 사용 (USB 포트임)
2. ❌ USB 케이블 연결 시 Serial1과 동시 사용 (충돌 가능)
3. ❌ 구버전 라이브러리 사용
4. ❌ flush() 함수에 의존 (버그 존재)

---

## 📚 참고 자료

### 공식 문서
- [Arduino UNO R4 WiFi Cheat Sheet](https://docs.arduino.cc/tutorials/uno-r4-wifi/cheat-sheet)
- [Arduino UNO R4 WiFi - Serial/UART](https://docs.arduino.cc/tutorials/uno-r4-wifi/cheat-sheet#usb-serial--uart)

### Arduino Forum
- [Uno R4 Modbus / RS485 Library](https://forum.arduino.cc/t/uno-r4-modbus-rs485-library/1152751)
- [ArduinoModbus gives CRC16 errors with Uno WiFi R4](https://forum.arduino.cc/t/arduinomodbus-gives-crc16-errors-with-uno-wifi-r4/)

### GitHub
- [ArduinoRS485 Library](https://github.com/arduino-libraries/ArduinoRS485)
- [ArduinoModbus Library](https://github.com/arduino-libraries/ArduinoModbus)

---

## 🎉 최종 결론

**현재 토양 센서 노드 코드는 수정 불필요합니다!**

ArduinoModbus 라이브러리가 자동으로 Serial1을 사용하도록 설계되어 있으므로, 우리 코드는 그대로 Arduino Uno R4 WiFi에서 정상 작동합니다.

**단, 다음을 확인하세요**:
- ✅ 라이브러리 최신 버전 사용
- ✅ USB 디버깅과 UART 통신 구분
- ✅ 배포 시 디버깅 코드 비활성화

---

**작성자**: 서준원  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm  
**문서 버전**: v1.0  
**최종 수정**: 2024-12-11
