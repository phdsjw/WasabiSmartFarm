/*
 * Wasabi SmartFarm - 수조 센서 라이브러리 구현
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2024-12-11
 */

#include "water_sensors.h"

// ============================================
// 생성자
// ============================================
WaterSensors::WaterSensors() 
  : _oneWire(ONE_WIRE_BUS_PIN),
    _tempSensor(&_oneWire),
    _tempSensorInitialized(false),
    _currentWaterTemp(25.0) {  // 기본값: 25°C
}

// ============================================
// 센서 초기화
// ============================================
bool WaterSensors::begin() {
  DEBUG_PRINTLN(F("[SENSORS] Initializing water tank sensors..."));
  
  // DS18B20 온도 센서 초기화
  _tempSensor.begin();
  
  int deviceCount = _tempSensor.getDeviceCount();
  DEBUG_PRINT(F("[SENSORS] DS18B20 devices found: "));
  DEBUG_PRINTLN(deviceCount);
  
  if (deviceCount > 0) {
    _tempSensor.setResolution(TEMPERATURE_PRECISION);
    _tempSensorInitialized = true;
    DEBUG_PRINTLN(F("[SENSORS] DS18B20 initialized successfully"));
  } else {
    DEBUG_PRINTLN(F("[SENSORS] WARNING: No DS18B20 sensor found!"));
  }
  
  // 아날로그 핀 설정
  pinMode(PH_SENSOR_PIN, INPUT);
  pinMode(TDS_SENSOR_PIN, INPUT);
  pinMode(EC_SENSOR_PIN, INPUT);
  
  DEBUG_PRINTLN(F("[SENSORS] Analog sensors initialized"));
  DEBUG_PRINTF("[SENSORS] pH Pin: A%d\n", PH_SENSOR_PIN - A0);
  DEBUG_PRINTF("[SENSORS] TDS Pin: A%d\n", TDS_SENSOR_PIN - A0);
  DEBUG_PRINTF("[SENSORS] EC Pin: A%d\n", EC_SENSOR_PIN - A0);
  
  return true;
}

// ============================================
// DS18B20 수온 읽기
// ============================================
float WaterSensors::readWaterTemperature() {
  if (!_tempSensorInitialized) {
    DEBUG_PRINTLN(F("[SENSORS] ERROR: DS18B20 not initialized"));
    return NAN;
  }
  
  // 온도 변환 요청
  _tempSensor.requestTemperatures();
  
  // 변환 완료 대기
  delay(DS18B20_CONVERSION_TIME);
  
  // 온도 읽기
  float temp = _tempSensor.getTempCByIndex(0);
  
  // 유효성 검사
  if (temp == DEVICE_DISCONNECTED_C || temp < WATER_TEMP_MIN || temp > WATER_TEMP_MAX) {
    DEBUG_PRINTLN(F("[SENSORS] ERROR: Invalid temperature reading"));
    return NAN;
  }
  
  _currentWaterTemp = temp;  // 온도 보상용으로 저장
  
  DEBUG_PRINTF("[SENSORS] Water Temperature: %.2f°C\n", temp);
  
  return temp;
}

// ============================================
// 아날로그 센서 평균값 읽기
// ============================================
float WaterSensors::readAnalogAverage(uint8_t pin) {
  long sum = 0;
  
  for (int i = 0; i < ANALOG_SAMPLE_COUNT; i++) {
    sum += analogRead(pin);
    delay(ANALOG_SAMPLE_DELAY);
  }
  
  return (float)sum / ANALOG_SAMPLE_COUNT;
}

// ============================================
// pH 센서 읽기 (SEN0161)
// ============================================
float WaterSensors::readPH() {
  // 아날로그 값 읽기 (평균)
  float avgValue = readAnalogAverage(PH_SENSOR_PIN);
  
  // 전압으로 변환
  float voltage = avgValue * (PH_VOLTAGE_REF / PH_ADC_RESOLUTION);
  
  // pH 값으로 변환
  float ph = voltageToPH(voltage);
  
  // 유효성 검사
  if (ph < PH_MIN || ph > PH_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: pH out of range: %.2f\n", ph);
  }
  
  // 정상 범위 경고
  if (ph < PH_NORMAL_MIN || ph > PH_NORMAL_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: pH outside normal range: %.2f (normal: %.1f-%.1f)\n", 
                 ph, PH_NORMAL_MIN, PH_NORMAL_MAX);
  }
  
  DEBUG_PRINTF("[SENSORS] pH: %.2f (voltage: %.2fV)\n", ph, voltage);
  
  return ph;
}

// ============================================
// 전압을 pH로 변환
// ============================================
float WaterSensors::voltageToPH(float voltage) {
  // SEN0161 공식: pH = 7.0 - (voltage - 2.5) / 0.18
  // 보정 공식: pH = 7.0 + OFFSET - (voltage - 2.5) / SLOPE
  
  float ph = 7.0 + PH_CALIBRATION_OFFSET - ((voltage - 2.5) / (PH_CALIBRATION_SLOPE / 18.0));
  
  return ph;
}

// ============================================
// TDS 센서 읽기 (SEN0244)
// ============================================
int WaterSensors::readTDS() {
  // 아날로그 값 읽기 (평균)
  float avgValue = readAnalogAverage(TDS_SENSOR_PIN);
  
  // 전압으로 변환
  float voltage = avgValue * (TDS_VOLTAGE_REF / TDS_ADC_RESOLUTION);
  
  // TDS 값으로 변환 (온도 보상 포함)
  int tds = voltageToTDS(voltage, _currentWaterTemp);
  
  // 유효성 검사
  if (tds < TDS_MIN || tds > TDS_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: TDS out of range: %d ppm\n", tds);
  }
  
  // 정상 범위 경고
  if (tds < TDS_NORMAL_MIN || tds > TDS_NORMAL_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: TDS outside normal range: %d ppm (normal: %d-%d)\n", 
                 tds, TDS_NORMAL_MIN, TDS_NORMAL_MAX);
  }
  
  DEBUG_PRINTF("[SENSORS] TDS: %d ppm (voltage: %.2fV, temp: %.1f°C)\n", 
               tds, voltage, _currentWaterTemp);
  
  return tds;
}

// ============================================
// 전압을 TDS로 변환 (온도 보상 포함)
// ============================================
int WaterSensors::voltageToTDS(float voltage, float temperature) {
  // 온도 보상 계수
  float compensationCoefficient = 1.0 + TDS_COMPENSATION_COEFFICIENT * (temperature - 25.0);
  
  // TDS 계산 (SEN0244 공식)
  // TDS (ppm) = (133.42 * voltage^3 - 255.86 * voltage^2 + 857.39 * voltage) * K_VALUE / compensationCoefficient
  
  float tdsValue = (133.42 * voltage * voltage * voltage 
                    - 255.86 * voltage * voltage 
                    + 857.39 * voltage) * TDS_K_VALUE / compensationCoefficient;
  
  return (int)tdsValue;
}

// ============================================
// EC 센서 읽기 (SEN0451 Pro)
// ============================================
float WaterSensors::readEC() {
  // 아날로그 값 읽기 (평균)
  float avgValue = readAnalogAverage(EC_SENSOR_PIN);
  
  // 전압으로 변환
  float voltage = avgValue * (EC_VOLTAGE_REF / EC_ADC_RESOLUTION);
  
  // EC 값으로 변환 (온도 보상 포함)
  float ec = voltageToEC(voltage, _currentWaterTemp);
  
  // 유효성 검사
  if (ec < EC_MIN || ec > EC_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: EC out of range: %.2f mS/cm\n", ec);
  }
  
  // 정상 범위 경고
  if (ec < EC_NORMAL_MIN || ec > EC_NORMAL_MAX) {
    DEBUG_PRINTF("[SENSORS] WARNING: EC outside normal range: %.2f mS/cm (normal: %.1f-%.1f)\n", 
                 ec, EC_NORMAL_MIN, EC_NORMAL_MAX);
  }
  
  DEBUG_PRINTF("[SENSORS] EC: %.2f mS/cm (voltage: %.2fV, temp: %.1f°C)\n", 
               ec, voltage, _currentWaterTemp);
  
  return ec;
}

// ============================================
// 전압을 EC로 변환 (온도 보상 포함)
// ============================================
float WaterSensors::voltageToEC(float voltage, float temperature) {
  // 온도 보상 (25°C 기준)
  float temperatureCoefficient = 1.0 + EC_TEMP_COEFFICIENT * (temperature - EC_REFERENCE_TEMP);
  
  // EC 계산 (SEN0451 Pro 공식)
  // EC (mS/cm) = voltage * K_VALUE / temperatureCoefficient
  
  float ecValue = (voltage * EC_K_VALUE) / temperatureCoefficient;
  
  return ecValue;
}

// ============================================
// 모든 센서 데이터 읽기
// ============================================
bool WaterSensors::readAllSensors(WaterTankSensorData &data) {
  DEBUG_PRINTLN(F("\n--- Reading Water Tank Sensors ---"));
  
  data.timestamp = millis();
  data.is_valid = true;
  
  // 1. 수온 읽기 (먼저 읽어서 온도 보상에 사용)
  data.water_temp = readWaterTemperature();
  if (isnan(data.water_temp)) {
    DEBUG_PRINTLN(F("[SENSORS] ERROR: Failed to read water temperature"));
    data.is_valid = false;
    return false;
  }
  
  // 2. pH 읽기
  data.water_ph = readPH();
  if (isnan(data.water_ph)) {
    DEBUG_PRINTLN(F("[SENSORS] ERROR: Failed to read pH"));
    data.is_valid = false;
  }
  
  // 3. TDS 읽기
  data.water_tds = readTDS();
  
  // 4. EC 읽기
  data.water_ec = readEC();
  if (isnan(data.water_ec)) {
    DEBUG_PRINTLN(F("[SENSORS] ERROR: Failed to read EC"));
    data.is_valid = false;
  }
  
  DEBUG_PRINTLN(F("----------------------------------\n"));
  
  return data.is_valid;
}

// ============================================
// DS18B20 센서 개수 확인
// ============================================
int WaterSensors::getTemperatureSensorCount() {
  return _tempSensor.getDeviceCount();
}

// ============================================
// 현재 수온 가져오기
// ============================================
float WaterSensors::getCurrentWaterTemp() {
  return _currentWaterTemp;
}
