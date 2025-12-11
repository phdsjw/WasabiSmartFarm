/*
 * SEN0604 토양 센서 Modbus RTU 통신 구현
 */

#include "sen0604_modbus.h"

SEN0604Modbus::SEN0604Modbus() {
    _initialized = false;
    _lastReadTime = 0;
}

bool SEN0604Modbus::begin() {
    DEBUG_PRINTLN(F("[SEN0604] Initializing Modbus RTU..."));
    
    // RS485 TX Enable 핀 설정
    pinMode(RS485_TX_ENABLE_PIN, OUTPUT);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);  // 수신 모드
    
    // Modbus RTU 시작 (Serial1, 보드레이트)
    if (!ModbusRTUClient.begin(MODBUS_BAUDRATE)) {
        DEBUG_PRINTLN(F("[SEN0604] Failed to start Modbus RTU!"));
        return false;
    }
    
    // Modbus 타임아웃 설정
    ModbusRTUClient.setTimeout(MODBUS_TIMEOUT);
    
    _initialized = true;
    DEBUG_PRINTLN(F("[SEN0604] Modbus RTU initialized successfully"));
    
    // 센서 연결 확인
    delay(1000);
    if (isConnected()) {
        DEBUG_PRINTLN(F("[SEN0604] Sensor connected!"));
        return true;
    } else {
        DEBUG_PRINTLN(F("[SEN0604] Warning: Sensor not responding"));
        return false;
    }
}

bool SEN0604Modbus::isConnected() {
    // 습도 레지스터 읽기 시도
    uint16_t value = readHoldingRegister(REG_SOIL_MOISTURE);
    return (value != 0xFFFF);  // 0xFFFF는 읽기 실패
}

SoilSensorData SEN0604Modbus::readSensorData() {
    SoilSensorData data;
    data.valid = false;
    data.timestamp = millis();
    
    if (!_initialized) {
        DEBUG_PRINTLN(F("[SEN0604] Error: Not initialized"));
        return data;
    }
    
    // 4개 레지스터 한 번에 읽기 (효율적)
    uint16_t buffer[NUM_REGISTERS];
    
    DEBUG_PRINT(F("[SEN0604] Reading registers from slave ID "));
    DEBUG_PRINTLN(MODBUS_SLAVE_ID);
    
    if (readHoldingRegisters(REG_SOIL_MOISTURE, NUM_REGISTERS, buffer)) {
        // 데이터 변환
        data.soil_moisture = buffer[0] * 0.1;           // 0.1% 단위
        
        // 온도: int16_t로 캐스팅 (음수 온도 처리 - 2의 보수)
        int16_t temp_raw = (int16_t)buffer[1];
        data.soil_temp = temp_raw * 0.1;                // 0.1°C 단위
        
        data.soil_ec = buffer[2] * 1.0;                 // 1 μS/cm 단위
        data.soil_ph = buffer[3] * 0.1;                 // 0.1 pH 단위 (수정: 0.01 → 0.1)
        
        data.valid = true;
        _lastReadTime = millis();
        
        // 디버그 출력
        DEBUG_PRINTLN(F("[SEN0604] Data read successfully:"));
        DEBUG_PRINT(F("  Moisture: ")); DEBUG_PRINT(data.soil_moisture); DEBUG_PRINTLN(F(" %"));
        DEBUG_PRINT(F("  Temp: ")); DEBUG_PRINT(data.soil_temp); DEBUG_PRINTLN(F(" °C"));
        DEBUG_PRINT(F("  EC: ")); DEBUG_PRINT(data.soil_ec); DEBUG_PRINTLN(F(" μS/cm"));
        DEBUG_PRINT(F("  pH: ")); DEBUG_PRINTLN(data.soil_ph);
    } else {
        DEBUG_PRINTLN(F("[SEN0604] Error: Failed to read sensor data"));
    }
    
    return data;
}

float SEN0604Modbus::readSoilMoisture() {
    uint16_t value = readHoldingRegister(REG_SOIL_MOISTURE);
    if (value == 0xFFFF) return -999.0;
    return value * 0.1;  // 0.1% 단위
}

float SEN0604Modbus::readSoilTemp() {
    uint16_t value = readHoldingRegister(REG_SOIL_TEMP);
    if (value == 0xFFFF) return -999.0;
    
    // int16_t로 캐스팅 (음수 온도 처리)
    int16_t temp_raw = (int16_t)value;
    return temp_raw * 0.1;  // 0.1°C 단위
}

float SEN0604Modbus::readSoilEC() {
    uint16_t value = readHoldingRegister(REG_SOIL_EC);
    if (value == 0xFFFF) return -999.0;
    return value * 1.0;  // 1 μS/cm 단위
}

float SEN0604Modbus::readSoilPH() {
    uint16_t value = readHoldingRegister(REG_SOIL_PH);
    if (value == 0xFFFF) return -999.0;
    return value * 0.1;  // 0.1 pH 단위
}

uint16_t SEN0604Modbus::readHoldingRegister(uint16_t address) {
    // TX Enable ON (송신 모드)
    digitalWrite(RS485_TX_ENABLE_PIN, HIGH);
    delayMicroseconds(100);
    
    // Modbus RTU Read Holding Registers (Function Code 0x03)
    ModbusRTUClient.requestFrom(MODBUS_SLAVE_ID, HOLDING_REGISTERS, address, 1);
    
    // TX Enable OFF (수신 모드)
    delayMicroseconds(100);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    // 응답 대기
    if (ModbusRTUClient.available()) {
        return ModbusRTUClient.read();
    }
    
    return 0xFFFF;  // 읽기 실패
}

bool SEN0604Modbus::readHoldingRegisters(uint16_t startAddress, uint16_t count, uint16_t* buffer) {
    // TX Enable ON (송신 모드)
    digitalWrite(RS485_TX_ENABLE_PIN, HIGH);
    delayMicroseconds(100);
    
    // Modbus RTU Read Holding Registers (Function Code 0x03)
    ModbusRTUClient.requestFrom(MODBUS_SLAVE_ID, HOLDING_REGISTERS, startAddress, count);
    
    // TX Enable OFF (수신 모드)
    delayMicroseconds(100);
    digitalWrite(RS485_TX_ENABLE_PIN, LOW);
    
    // 응답 대기 및 읽기
    if (ModbusRTUClient.available() >= count) {
        for (uint16_t i = 0; i < count; i++) {
            buffer[i] = ModbusRTUClient.read();
        }
        return true;
    }
    
    DEBUG_PRINT(F("[SEN0604] Modbus error: "));
    DEBUG_PRINTLN(ModbusRTUClient.lastError());
    
    return false;
}

float SEN0604Modbus::convertToFloat(uint16_t rawValue, float scale) {
    return rawValue * scale;
}
