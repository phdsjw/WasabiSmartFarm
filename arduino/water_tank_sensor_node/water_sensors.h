/*
 * Wasabi SmartFarm - 수조 센서 라이브러리
 * 
 * 기능:
 * - DS18B20 수온 센서 (1-Wire)
 * - SEN0161 pH 센서 (아날로그)
 * - SEN0244 TDS 센서 (아날로그)
 * - SEN0451 Pro EC 센서 (아날로그)
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#ifndef WATER_SENSORS_H
#define WATER_SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class WaterSensors {
private:
  // 1-Wire 객체
  OneWire _oneWire;
  DallasTemperature _tempSensor;
  
  // 센서 초기화 상태
  bool _tempSensorInitialized;
  
  // 현재 수온 (온도 보상용)
  float _currentWaterTemp;
  
  // 아날로그 센서 읽기 (평균값)
  float readAnalogAverage(uint8_t pin);
  
  // 전압을 pH로 변환
  float voltageToPH(float voltage);
  
  // 전압을 TDS로 변환 (온도 보상 포함)
  int voltageToTDS(float voltage, float temperature);
  
  // 전압을 EC로 변환 (온도 보상 포함)
  float voltageToEC(float voltage, float temperature);

public:
  // 생성자
  WaterSensors();
  
  // 센서 초기화
  bool begin();
  
  // DS18B20 수온 읽기 (°C)
  float readWaterTemperature();
  
  // SEN0161 pH 읽기
  float readPH();
  
  // SEN0244 TDS 읽기 (ppm)
  int readTDS();
  
  // SEN0451 Pro EC 읽기 (mS/cm)
  float readEC();
  
  // 모든 센서 데이터 읽기
  bool readAllSensors(WaterTankSensorData &data);
  
  // DS18B20 센서 개수 확인
  int getTemperatureSensorCount();
  
  // 현재 수온 가져오기 (온도 보상용)
  float getCurrentWaterTemp();
};

#endif // WATER_SENSORS_H
