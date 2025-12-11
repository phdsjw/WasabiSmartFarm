/*
 * Sensor Data Collection
 * All sensor reading functions
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
#include "config.h"

// ============================================
// 센서 데이터 구조체
// ============================================
struct SensorData {
  // 대기 환경
  float air_temp;
  float air_humidity;
  
  // 물탱크
  float water_temp;
  float water_ph;
  float water_tds;
  float water_ec;
  
  // 토양 센서 (탱크별)
  float soil_temp[TANK_COUNT];
  float soil_moisture[TANK_COUNT];
  float soil_ec[TANK_COUNT];
  float soil_ph[TANK_COUNT];
  
  // 유효성 플래그
  bool air_valid;
  bool water_valid;
  bool soil_valid[TANK_COUNT];
};

// ============================================
// 전역 센서 객체
// ============================================
extern Adafruit_SHT31 sht31;
extern OneWire oneWire;
extern DallasTemperature dallas;

// ============================================
// 함수 선언
// ============================================

// 센서 초기화
void initSensors();

// 환경 센서 읽기
SensorData readEnvironmentSensors();

// 개별 센서 읽기 함수
float readAirTemp();
float readAirHumidity();
float readWaterTemp();
float readPH();
float readTDS();
float readEC();

// 토양 센서 읽기 (Modbus RTU)
bool readSoilSensor(uint8_t slaveId, float* temp, float* moisture, float* ec, float* ph);

// 수위 센서 읽기
void readAndPublishWaterLevels();
int readWaterLevel(uint8_t tankNum);

// 센서 값 유효성 검사
bool isValidFloat(float value);
bool isValidTemperature(float temp);

#endif // SENSORS_H
