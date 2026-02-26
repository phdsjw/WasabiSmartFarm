/*
 * Wasabi SmartFarm - SHT30 센서 라이브러리
 * 
 * 기능:
 * - SHT30 온습도 센서 초기화
 * - 대기 온도/습도 측정
 * - I2C 통신
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef SHT30_SENSOR_H
#define SHT30_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class SHT30Sensor {
private:
  uint8_t _i2c_address;
  bool _initialized;
  
  // CRC8 체크섬 계산 (SHT30 데이터 검증용)
  uint8_t calculateCRC8(const uint8_t *data, uint8_t len);
  
  // 센서에 명령 전송
  bool sendCommand(uint16_t command);
  
  // 센서 데이터 읽기
  bool readData(uint8_t *data, uint8_t len);

public:
  // 생성자
  SHT30Sensor(uint8_t i2c_address = SHT30_I2C_ADDRESS);
  
  // 센서 초기화
  bool begin();
  
  // 센서 연결 확인
  bool isConnected();
  
  // 대기 온도 읽기 (°C)
  float readTemperature();
  
  // 대기 습도 읽기 (%)
  float readHumidity();
  
  // 온도와 습도를 한 번에 읽기
  bool readTempHumidity(float &temp, float &humidity);
  
  // 센서 소프트 리셋
  bool softReset();
  
  // 히터 제어 (습도 센서 드리프트 방지용)
  bool enableHeater(bool enable);
};

#endif // SHT30_SENSOR_H
