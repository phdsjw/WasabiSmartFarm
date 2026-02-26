/*
 * Sensor Implementation
 */

#include "sensors.h"

// ============================================
// 전역 센서 객체 정의
// ============================================
Adafruit_SHT31 sht31 = Adafruit_SHT31();
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature dallas(&oneWire);

// ============================================
// 센서 초기화
// ============================================
void initSensors() {
  // I2C 초기화
  Wire.begin();
  
  // SHT30 초기화
  if (!sht31.begin(SHT30_I2C_ADDRESS)) {
    DEBUG_PRINTLN(F("[ERROR] SHT30 sensor not found!"));
  } else {
    DEBUG_PRINTLN(F("[OK] SHT30 initialized"));
  }
  
  // DS18B20 초기화
  dallas.begin();
  int deviceCount = dallas.getDeviceCount();
  DEBUG_PRINT(F("[OK] Found "));
  DEBUG_PRINT(deviceCount);
  DEBUG_PRINTLN(F(" DS18B20 sensor(s)"));
  
  // Modbus RTU 초기화 (Serial1 사용)
  if (!ModbusRTUClient.begin(9600)) {
    DEBUG_PRINTLN(F("[ERROR] Failed to start Modbus RTU Client!"));
  } else {
    DEBUG_PRINTLN(F("[OK] Modbus RTU initialized"));
  }
  
  // 아날로그 핀 설정
  pinMode(PH_SENSOR_PIN, INPUT);
  pinMode(TDS_SENSOR_PIN, INPUT);
  pinMode(EC_SENSOR_PIN, INPUT);
  pinMode(WATER_LEVEL_TANK01, INPUT);
  
  DEBUG_PRINTLN(F("[OK] All sensors initialized\n"));
}

// ============================================
// 환경 센서 데이터 읽기
// ============================================
SensorData readEnvironmentSensors() {
  SensorData data;
  
  // 구조체 초기화
  memset(&data, 0, sizeof(SensorData));
  
  // 대기 온습도 (SHT30)
  data.air_temp = readAirTemp();
  data.air_humidity = readAirHumidity();
  data.air_valid = isValidTemperature(data.air_temp) && isValidFloat(data.air_humidity);
  
  if (data.air_valid) {
    DEBUG_PRINT(F("  Air Temp: "));
    DEBUG_PRINT(data.air_temp);
    DEBUG_PRINT(F("°C, Humidity: "));
    DEBUG_PRINT(data.air_humidity);
    DEBUG_PRINTLN(F("%"));
  } else {
    DEBUG_PRINTLN(F("  [WARN] Air sensor data invalid"));
  }
  
  // 물탱크 센서
  data.water_temp = readWaterTemp();
  data.water_ph = readPH();
  data.water_tds = readTDS();
  data.water_ec = readEC();
  data.water_valid = isValidTemperature(data.water_temp) && 
                     isValidFloat(data.water_ph) && 
                     isValidFloat(data.water_tds) && 
                     isValidFloat(data.water_ec);
  
  if (data.water_valid) {
    DEBUG_PRINT(F("  Water Temp: "));
    DEBUG_PRINT(data.water_temp);
    DEBUG_PRINT(F("°C, pH: "));
    DEBUG_PRINT(data.water_ph);
    DEBUG_PRINT(F(", TDS: "));
    DEBUG_PRINT(data.water_tds);
    DEBUG_PRINT(F(" ppm, EC: "));
    DEBUG_PRINT(data.water_ec);
    DEBUG_PRINTLN(F(" mS/cm"));
  } else {
    DEBUG_PRINTLN(F("  [WARN] Water sensor data invalid"));
  }
  
  // 토양 센서 (Modbus RTU)
  for (uint8_t i = 0; i < TANK_COUNT; i++) {
    uint8_t slaveId = SOIL_SENSOR_START_ID + i;
    
    bool success = readSoilSensor(
      slaveId,
      &data.soil_temp[i],
      &data.soil_moisture[i],
      &data.soil_ec[i],
      &data.soil_ph[i]
    );
    
    data.soil_valid[i] = success;
    
    if (success) {
      DEBUG_PRINT(F("  Tank "));
      DEBUG_PRINT(i + 1);
      DEBUG_PRINT(F(" - Temp: "));
      DEBUG_PRINT(data.soil_temp[i]);
      DEBUG_PRINT(F("°C, Moisture: "));
      DEBUG_PRINT(data.soil_moisture[i]);
      DEBUG_PRINT(F("%, EC: "));
      DEBUG_PRINT(data.soil_ec[i]);
      DEBUG_PRINT(F(" μS/cm, pH: "));
      DEBUG_PRINTLN(data.soil_ph[i]);
    } else {
      DEBUG_PRINT(F("  [WARN] Tank "));
      DEBUG_PRINT(i + 1);
      DEBUG_PRINTLN(F(" soil sensor read failed"));
    }
    
    // Modbus 통신 간 딜레이
    delay(100);
  }
  
  return data;
}

// ============================================
// 개별 센서 읽기 함수
// ============================================

float readAirTemp() {
  float temp = sht31.readTemperature();
  return isnan(temp) ? -999.0 : temp;
}

float readAirHumidity() {
  float hum = sht31.readHumidity();
  return isnan(hum) ? -999.0 : hum;
}

float readWaterTemp() {
  dallas.requestTemperatures();
  float temp = dallas.getTempCByIndex(0);
  return (temp == DEVICE_DISCONNECTED_C) ? -999.0 : temp;
}

float readPH() {
  int rawValue = analogRead(PH_SENSOR_PIN);
  // pH 센서 변환 공식 (센서 스펙에 맞게 수정)
  float voltage = rawValue * (5.0 / 1023.0);
  float ph = 3.5 * voltage + PH_CALIBRATION_OFFSET;
  return (ph >= 0 && ph <= 14) ? ph : -999.0;
}

float readTDS() {
  int rawValue = analogRead(TDS_SENSOR_PIN);
  // TDS 센서 변환 공식
  float voltage = rawValue * (5.0 / 1023.0);
  float tds = (133.42 * voltage * voltage * voltage - 
               255.86 * voltage * voltage + 
               857.39 * voltage) * TDS_CALIBRATION_FACTOR;
  return (tds >= 0 && tds <= 1000) ? tds : -999.0;
}

float readEC() {
  int rawValue = analogRead(EC_SENSOR_PIN);
  // EC 센서 변환 공식 (센서 스펙에 맞게 수정)
  float voltage = rawValue * (5.0 / 1023.0);
  float ec = voltage * 2.0 * EC_CALIBRATION_FACTOR;  // 예시 공식
  return (ec >= 0 && ec <= 20) ? ec : -999.0;
}

// ============================================
// Modbus RTU 토양 센서 읽기
// ============================================
bool readSoilSensor(uint8_t slaveId, float* temp, float* moisture, float* ec, float* ph) {
  // SEN0604 센서 레지스터 주소 (예시 - 실제 스펙에 맞게 수정)
  const uint16_t REGISTER_START = 0x0000;
  const uint8_t NUM_REGISTERS = 4;
  
  // Modbus Read Holding Registers 요청
  if (!ModbusRTUClient.requestFrom(slaveId, HOLDING_REGISTERS, REGISTER_START, NUM_REGISTERS)) {
    DEBUG_PRINT(F("    [ERROR] Modbus read failed for Slave "));
    DEBUG_PRINTLN(slaveId);
    return false;
  }
  
  // 데이터 읽기
  if (ModbusRTUClient.available() >= NUM_REGISTERS) {
    uint16_t reg0 = ModbusRTUClient.read();  // Temperature (x10)
    uint16_t reg1 = ModbusRTUClient.read();  // Moisture (x10)
    uint16_t reg2 = ModbusRTUClient.read();  // EC (x100)
    uint16_t reg3 = ModbusRTUClient.read();  // pH (x10)
    
    *temp = reg0 / 10.0;
    *moisture = reg1 / 10.0;
    *ec = reg2 / 100.0;
    *ph = reg3 / 10.0;
    
    return true;
  }
  
  return false;
}

// ============================================
// 수위 센서 읽기 및 발행
// ============================================
void readAndPublishWaterLevels() {
  for (uint8_t i = 1; i <= TANK_COUNT; i++) {
    int level = readWaterLevel(i);
    
    // MQTT 발행은 mqtt_handler.cpp에서 처리
    // 여기서는 시리얼 출력만
    DEBUG_PRINT(F("  Tank "));
    DEBUG_PRINT(i);
    DEBUG_PRINT(F(" water level: "));
    DEBUG_PRINT(level);
    DEBUG_PRINTLN(F("%"));
  }
}

int readWaterLevel(uint8_t tankNum) {
  // 간단한 예시: Tank 1만 A3 핀 사용
  // 실제로는 멀티플렉서 또는 추가 아날로그 핀 필요
  if (tankNum == 1) {
    int rawValue = analogRead(WATER_LEVEL_TANK01);
    // 0~1023을 0~100%로 변환
    int level = map(rawValue, 0, 1023, 0, 100);
    return constrain(level, 0, 100);
  }
  
  // 나머지 탱크는 임시로 랜덤값 반환 (실제 구현 필요)
  return random(30, 80);
}

// ============================================
// 센서 값 유효성 검사
// ============================================
bool isValidFloat(float value) {
  return !isnan(value) && value != -999.0;
}

bool isValidTemperature(float temp) {
  return isValidFloat(temp) && temp >= -40.0 && temp <= 125.0;
}
