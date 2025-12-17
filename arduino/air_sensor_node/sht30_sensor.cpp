/*
 * Wasabi SmartFarm - SHT30 센서 라이브러리 구현
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "sht30_sensor.h"

// SHT30 명령어 정의
#define SHT30_CMD_READ_STATUS       0xF32D  // 상태 레지스터 읽기
#define SHT30_CMD_CLEAR_STATUS      0x3041  // 상태 레지스터 클리어
#define SHT30_CMD_SOFT_RESET        0x30A2  // 소프트 리셋
#define SHT30_CMD_HEATER_ENABLE     0x306D  // 히터 켜기
#define SHT30_CMD_HEATER_DISABLE    0x3066  // 히터 끄기

// 측정 명령어 (High repeatability)
#define SHT30_CMD_MEASURE_HIGH      0x2400  // Clock stretching disabled
#define SHT30_CMD_MEASURE_HIGH_CS   0x2C06  // Clock stretching enabled

// CRC8 다항식 (SHT30 사양)
#define SHT30_CRC8_POLYNOMIAL       0x31

// ============================================
// 생성자
// ============================================
SHT30Sensor::SHT30Sensor(uint8_t i2c_address) 
  : _i2c_address(i2c_address), _initialized(false) {
}

// ============================================
// 센서 초기화
// ============================================
bool SHT30Sensor::begin() {
  Wire.begin();
  
  DEBUG_PRINTLN(F("[SHT30] Initializing sensor..."));
  
  // 센서 연결 확인
  if (!isConnected()) {
    DEBUG_PRINTLN(F("[SHT30] ERROR: Sensor not found!"));
    return false;
  }
  
  // 소프트 리셋
  delay(10);
  if (!softReset()) {
    DEBUG_PRINTLN(F("[SHT30] WARNING: Soft reset failed"));
  }
  
  delay(50);  // 리셋 후 안정화 대기
  
  _initialized = true;
  DEBUG_PRINTLN(F("[SHT30] Sensor initialized successfully"));
  
  return true;
}

// ============================================
// 센서 연결 확인
// ============================================
bool SHT30Sensor::isConnected() {
  Wire.beginTransmission(_i2c_address);
  return (Wire.endTransmission() == 0);
}

// ============================================
// 명령 전송
// ============================================
bool SHT30Sensor::sendCommand(uint16_t command) {
  Wire.beginTransmission(_i2c_address);
  Wire.write(command >> 8);    // MSB
  Wire.write(command & 0xFF);  // LSB
  return (Wire.endTransmission() == 0);
}

// ============================================
// 데이터 읽기
// ============================================
bool SHT30Sensor::readData(uint8_t *data, uint8_t len) {
  uint8_t bytesReceived = Wire.requestFrom(_i2c_address, len);
  
  if (bytesReceived != len) {
    DEBUG_PRINT(F("[SHT30] ERROR: Expected "));
    DEBUG_PRINT(len);
    DEBUG_PRINT(F(" bytes, got "));
    DEBUG_PRINTLN(bytesReceived);
    return false;
  }
  
  for (uint8_t i = 0; i < len; i++) {
    data[i] = Wire.read();
  }
  
  return true;
}

// ============================================
// CRC8 체크섬 계산
// ============================================
uint8_t SHT30Sensor::calculateCRC8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0xFF;  // 초기값
  
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ SHT30_CRC8_POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }
  
  return crc;
}

// ============================================
// 온도와 습도 동시 읽기
// ============================================
bool SHT30Sensor::readTempHumidity(float &temp, float &humidity) {
  if (!_initialized) {
    DEBUG_PRINTLN(F("[SHT30] ERROR: Sensor not initialized"));
    return false;
  }
  
  // 측정 시작 명령 전송
  if (!sendCommand(SHT30_CMD_MEASURE_HIGH)) {
    DEBUG_PRINTLN(F("[SHT30] ERROR: Failed to send measurement command"));
    return false;
  }
  
  delay(20);  // 측정 완료 대기 (High repeatability: ~15ms)
  
  // 6바이트 읽기: [Temp MSB][Temp LSB][Temp CRC][Hum MSB][Hum LSB][Hum CRC]
  uint8_t data[6];
  if (!readData(data, 6)) {
    DEBUG_PRINTLN(F("[SHT30] ERROR: Failed to read sensor data"));
    return false;
  }
  
  // CRC 검증 (온도)
  uint8_t temp_crc = calculateCRC8(data, 2);
  if (temp_crc != data[2]) {
    DEBUG_PRINT(F("[SHT30] ERROR: Temperature CRC mismatch (expected: 0x"));
    DEBUG_PRINT(temp_crc, HEX);
    DEBUG_PRINT(F(", got: 0x"));
    DEBUG_PRINT(data[2], HEX);
    DEBUG_PRINTLN(F(")"));
    return false;
  }
  
  // CRC 검증 (습도)
  uint8_t hum_crc = calculateCRC8(data + 3, 2);
  if (hum_crc != data[5]) {
    DEBUG_PRINT(F("[SHT30] ERROR: Humidity CRC mismatch (expected: 0x"));
    DEBUG_PRINT(hum_crc, HEX);
    DEBUG_PRINT(F(", got: 0x"));
    DEBUG_PRINT(data[5], HEX);
    DEBUG_PRINTLN(F(")"));
    return false;
  }
  
  // 온도 변환 (공식: -45 + 175 * (raw / 65535))
  uint16_t temp_raw = (data[0] << 8) | data[1];
  temp = -45.0 + 175.0 * ((float)temp_raw / 65535.0);
  
  // 습도 변환 (공식: 100 * (raw / 65535))
  uint16_t hum_raw = (data[3] << 8) | data[4];
  humidity = 100.0 * ((float)hum_raw / 65535.0);
  
  // 범위 체크
  if (temp < SHT30_TEMP_MIN || temp > SHT30_TEMP_MAX) {
    DEBUG_PRINT(F("[SHT30] WARNING: Temperature out of range: "));
    DEBUG_PRINT(temp, 2);
    DEBUG_PRINTLN(F("°C"));
  }
  
  if (humidity < SHT30_HUMIDITY_MIN || humidity > SHT30_HUMIDITY_MAX) {
    DEBUG_PRINT(F("[SHT30] WARNING: Humidity out of range: "));
    DEBUG_PRINT(humidity, 2);
    DEBUG_PRINTLN(F("%"));
  }
  
  DEBUG_PRINT(F("[SHT30] Temp: "));
  DEBUG_PRINT(temp, 2);
  DEBUG_PRINT(F("°C, Humidity: "));
  DEBUG_PRINT(humidity, 2);
  DEBUG_PRINTLN(F("%"));
  
  return true;
}

// ============================================
// 온도 읽기
// ============================================
float SHT30Sensor::readTemperature() {
  float temp, humidity;
  if (readTempHumidity(temp, humidity)) {
    return temp;
  }
  return NAN;
}

// ============================================
// 습도 읽기
// ============================================
float SHT30Sensor::readHumidity() {
  float temp, humidity;
  if (readTempHumidity(temp, humidity)) {
    return humidity;
  }
  return NAN;
}

// ============================================
// 소프트 리셋
// ============================================
bool SHT30Sensor::softReset() {
  DEBUG_PRINTLN(F("[SHT30] Performing soft reset..."));
  
  bool result = sendCommand(SHT30_CMD_SOFT_RESET);
  
  if (result) {
    delay(50);  // 리셋 완료 대기
    DEBUG_PRINTLN(F("[SHT30] Soft reset successful"));
  } else {
    DEBUG_PRINTLN(F("[SHT30] Soft reset failed"));
  }
  
  return result;
}

// ============================================
// 히터 제어
// ============================================
bool SHT30Sensor::enableHeater(bool enable) {
  uint16_t command = enable ? SHT30_CMD_HEATER_ENABLE : SHT30_CMD_HEATER_DISABLE;
  
  DEBUG_PRINT(F("[SHT30] Heater "));
  DEBUG_PRINTLN(enable ? F("ENABLED") : F("DISABLED"));
  
  return sendCommand(command);
}
