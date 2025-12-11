/*
 * SEN0604 토양 센서 Modbus RTU 통신 헤더
 */

#ifndef SEN0604_MODBUS_H
#define SEN0604_MODBUS_H

#include <Arduino.h>
#include <ArduinoModbus.h>
#include "config.h"

class SEN0604Modbus {
public:
    SEN0604Modbus();
    
    // 초기화
    bool begin();
    
    // 센서 데이터 읽기
    SoilSensorData readSensorData();
    
    // 개별 값 읽기
    float readSoilMoisture();
    float readSoilTemp();
    float readSoilEC();
    float readSoilPH();
    
    // 센서 연결 확인
    bool isConnected();
    
private:
    // Modbus 레지스터 읽기 헬퍼 함수
    uint16_t readHoldingRegister(uint16_t address);
    bool readHoldingRegisters(uint16_t startAddress, uint16_t count, uint16_t* buffer);
    
    // 데이터 변환 함수
    float convertToFloat(uint16_t rawValue, float scale);
    
    bool _initialized;
    unsigned long _lastReadTime;
};

#endif // SEN0604_MODBUS_H
