/*
 * Wasabi SmartFarm - pH 센서 칼리브레이션 도구
 * 
 * 설명: SEN0161 pH 센서를 3점 칼리브레이션하는 도구
 *       pH 4.00, pH 7.00, pH 10.00 표준 용액 사용
 * 
 * 작성자: 서준원
 * 버전: v1.0.0
 * 날짜: 2025-12-27
 * 
 * 사용 방법:
 * 1. Arduino Uno R4 WiFi에 업로드
 * 2. 시리얼 모니터 열기 (115200 baud)
 * 3. 메뉴 지시에 따라 칼리브레이션 진행
 * 4. 완료 후 출력된 보정값을 config.h에 적용
 */

#include <EEPROM.h>

// ============================================
// 핀 설정
// ============================================
#define PH_SENSOR_PIN A0             // pH 센서 핀

// ============================================
// 센서 설정
// ============================================
#define PH_VOLTAGE_REF 5.0           // 기준 전압 (5V)
#define PH_ADC_RESOLUTION 1024       // ADC 해상도 (10비트)
#define SAMPLE_COUNT 30              // 샘플링 횟수 (안정적인 측정)
#define SAMPLE_DELAY 100             // 샘플 간 딜레이 (ms)

// ============================================
// 칼리브레이션 표준 용액
// ============================================
#define PH_STANDARD_4   4.00         // pH 4.00 표준 용액
#define PH_STANDARD_7   7.00         // pH 7.00 표준 용액 (중성)
#define PH_STANDARD_10  10.00        // pH 10.00 표준 용액

// ============================================
// EEPROM 주소
// ============================================
#define EEPROM_MAGIC_ADDR 0          // 매직 넘버 주소
#define EEPROM_OFFSET_ADDR 4         // Offset 주소 (float, 4바이트)
#define EEPROM_SLOPE_ADDR 8          // Slope 주소 (float, 4바이트)
#define EEPROM_MAGIC_NUMBER 0x50484341  // "PHCA" (pH Calibration)

// ============================================
// 칼리브레이션 데이터 구조체
// ============================================
struct CalibrationData {
  float voltage_ph4;     // pH 4.00 용액의 전압
  float voltage_ph7;     // pH 7.00 용액의 전압
  float voltage_ph10;    // pH 10.00 용액의 전압
  float offset;          // 오프셋 (중성점 보정)
  float slope;           // 기울기 (감도)
  bool isValid;          // 데이터 유효성
};

CalibrationData calibData;

// ============================================
// 전역 변수
// ============================================
bool calibrationComplete = false;

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // 시리얼 포트 연결 대기
  }
  
  delay(1000);
  
  // 헤더 출력
  printHeader();
  
  // 핀 초기화
  pinMode(PH_SENSOR_PIN, INPUT);
  
  // 저장된 칼리브레이션 데이터 불러오기
  loadCalibration();
  
  // 메인 메뉴 표시
  printMainMenu();
}

// ============================================
// Loop
// ============================================
void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    // 남은 버퍼 비우기
    while (Serial.available() > 0) {
      Serial.read();
    }
    
    handleCommand(command);
  }
}

// ============================================
// 헤더 출력
// ============================================
void printHeader() {
  Serial.println(F("\n"));
  Serial.println(F("===================================================="));
  Serial.println(F("   Wasabi SmartFarm - pH Sensor Calibration Tool   "));
  Serial.println(F("===================================================="));
  Serial.println(F("  버전: v1.0.0"));
  Serial.println(F("  센서: SEN0161 pH Sensor"));
  Serial.println(F("  방식: 3-Point Calibration (pH 4.00, 7.00, 10.00)"));
  Serial.println(F("===================================================="));
  Serial.println();
}

// ============================================
// 메인 메뉴 출력
// ============================================
void printMainMenu() {
  Serial.println(F("\n===== 메인 메뉴 ====="));
  Serial.println(F("1. 실시간 pH 측정 (현재 보정값 적용)"));
  Serial.println(F("2. 3점 칼리브레이션 시작"));
  Serial.println(F("3. 저장된 칼리브레이션 데이터 보기"));
  Serial.println(F("4. 칼리브레이션 데이터 초기화"));
  Serial.println(F("5. 원시 전압 측정 (보정 없음)"));
  Serial.println(F("====================="));
  Serial.println(F("명령어를 입력하세요 (1-5): "));
}

// ============================================
// 명령어 처리
// ============================================
void handleCommand(char command) {
  Serial.println();
  
  switch (command) {
    case '1':
      realtimeMeasurement();
      break;
      
    case '2':
      startCalibration();
      break;
      
    case '3':
      showCalibrationData();
      break;
      
    case '4':
      resetCalibration();
      break;
      
    case '5':
      rawVoltageMeasurement();
      break;
      
    default:
      Serial.println(F("[ERROR] 잘못된 명령어입니다."));
      break;
  }
  
  printMainMenu();
}

// ============================================
// 1. 실시간 pH 측정
// ============================================
void realtimeMeasurement() {
  Serial.println(F("\n===== 실시간 pH 측정 ====="));
  Serial.println(F("현재 저장된 보정값으로 pH를 측정합니다."));
  Serial.println(F("'q'를 입력하면 종료합니다.\n"));
  
  if (!calibData.isValid) {
    Serial.println(F("[WARNING] 칼리브레이션 데이터가 없습니다."));
    Serial.println(F("[WARNING] 기본값(Offset=0, Slope=3.5)으로 측정합니다.\n"));
  }
  
  while (true) {
    // 센서 읽기
    float voltage = readSensorVoltage();
    float ph = voltageToPH(voltage);
    
    // 결과 출력
    Serial.print(F("전압: "));
    Serial.print(voltage, 3);
    Serial.print(F(" V  →  pH: "));
    Serial.println(ph, 2);
    
    // 종료 체크
    if (Serial.available() > 0) {
      char ch = Serial.read();
      while (Serial.available() > 0) Serial.read();  // 버퍼 비우기
      
      if (ch == 'q' || ch == 'Q') {
        Serial.println(F("\n[INFO] 실시간 측정을 종료합니다.\n"));
        break;
      }
    }
    
    delay(1000);
  }
}

// ============================================
// 2. 3점 칼리브레이션 시작
// ============================================
void startCalibration() {
  Serial.println(F("\n===== 3점 칼리브레이션 시작 ====="));
  Serial.println(F("준비물: pH 4.00, 7.00, 10.00 표준 용액"));
  Serial.println(F("\n[주의사항]"));
  Serial.println(F("1. 센서를 깨끗이 증류수로 세척하세요."));
  Serial.println(F("2. 센서를 종이 타월로 물기를 제거하세요."));
  Serial.println(F("3. 표준 용액에 센서를 충분히 담그세요."));
  Serial.println(F("4. 각 측정 시 약 1-2분 안정화 시간이 필요합니다."));
  Serial.println(F("==================================\n"));
  
  // Step 1: pH 7.00 (중성점)
  Serial.println(F("Step 1/3: pH 7.00 표준 용액 측정"));
  Serial.println(F("센서를 pH 7.00 용액에 담그고 'c'를 입력하세요."));
  waitForConfirm();
  
  calibData.voltage_ph7 = measureVoltageStable();
  Serial.print(F("[측정 완료] pH 7.00 전압: "));
  Serial.print(calibData.voltage_ph7, 3);
  Serial.println(F(" V\n"));
  
  // Step 2: pH 4.00 (산성)
  Serial.println(F("Step 2/3: pH 4.00 표준 용액 측정"));
  Serial.println(F("센서를 깨끗이 세척 후 pH 4.00 용액에 담그고 'c'를 입력하세요."));
  waitForConfirm();
  
  calibData.voltage_ph4 = measureVoltageStable();
  Serial.print(F("[측정 완료] pH 4.00 전압: "));
  Serial.print(calibData.voltage_ph4, 3);
  Serial.println(F(" V\n"));
  
  // Step 3: pH 10.00 (염기성)
  Serial.println(F("Step 3/3: pH 10.00 표준 용액 측정"));
  Serial.println(F("센서를 깨끗이 세척 후 pH 10.00 용액에 담그고 'c'를 입력하세요."));
  waitForConfirm();
  
  calibData.voltage_ph10 = measureVoltageStable();
  Serial.print(F("[측정 완료] pH 10.00 전압: "));
  Serial.print(calibData.voltage_ph10, 3);
  Serial.println(F(" V\n"));
  
  // 칼리브레이션 계산
  calculateCalibration();
  
  // 결과 출력
  printCalibrationResults();
  
  // EEPROM에 저장 확인
  Serial.println(F("\n칼리브레이션 데이터를 저장하시겠습니까? (y/n): "));
  char confirm = waitForYesNo();
  
  if (confirm == 'y' || confirm == 'Y') {
    saveCalibration();
    Serial.println(F("[SUCCESS] 칼리브레이션 데이터가 저장되었습니다!"));
    calibrationComplete = true;
  } else {
    Serial.println(F("[INFO] 칼리브레이션 데이터가 저장되지 않았습니다."));
  }
}

// ============================================
// 3. 저장된 칼리브레이션 데이터 보기
// ============================================
void showCalibrationData() {
  Serial.println(F("\n===== 저장된 칼리브레이션 데이터 ====="));
  
  if (!calibData.isValid) {
    Serial.println(F("[INFO] 저장된 칼리브레이션 데이터가 없습니다."));
    Serial.println(F("[INFO] 먼저 칼리브레이션을 진행하세요."));
    return;
  }
  
  printCalibrationResults();
}

// ============================================
// 4. 칼리브레이션 데이터 초기화
// ============================================
void resetCalibration() {
  Serial.println(F("\n===== 칼리브레이션 데이터 초기화 ====="));
  Serial.println(F("정말 초기화하시겠습니까? (y/n): "));
  
  char confirm = waitForYesNo();
  
  if (confirm == 'y' || confirm == 'Y') {
    // EEPROM 초기화
    uint32_t magic = 0;
    EEPROM.put(EEPROM_MAGIC_ADDR, magic);
    
    // 메모리 데이터 초기화
    calibData.voltage_ph4 = 0;
    calibData.voltage_ph7 = 0;
    calibData.voltage_ph10 = 0;
    calibData.offset = 0;
    calibData.slope = 3.5;  // 기본값
    calibData.isValid = false;
    
    Serial.println(F("[SUCCESS] 칼리브레이션 데이터가 초기화되었습니다."));
  } else {
    Serial.println(F("[INFO] 초기화가 취소되었습니다."));
  }
}

// ============================================
// 5. 원시 전압 측정
// ============================================
void rawVoltageMeasurement() {
  Serial.println(F("\n===== 원시 전압 측정 ====="));
  Serial.println(F("보정 없이 센서의 원시 전압을 측정합니다."));
  Serial.println(F("'q'를 입력하면 종료합니다.\n"));
  
  while (true) {
    // 센서 읽기
    float voltage = readSensorVoltage();
    
    // 결과 출력
    Serial.print(F("원시 전압: "));
    Serial.print(voltage, 3);
    Serial.println(F(" V"));
    
    // 종료 체크
    if (Serial.available() > 0) {
      char ch = Serial.read();
      while (Serial.available() > 0) Serial.read();
      
      if (ch == 'q' || ch == 'Q') {
        Serial.println(F("\n[INFO] 원시 전압 측정을 종료합니다.\n"));
        break;
      }
    }
    
    delay(1000);
  }
}

// ============================================
// 센서 전압 읽기 (평균)
// ============================================
float readSensorVoltage() {
  long sum = 0;
  
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += analogRead(PH_SENSOR_PIN);
    delay(SAMPLE_DELAY);
  }
  
  float avgValue = (float)sum / SAMPLE_COUNT;
  float voltage = avgValue * (PH_VOLTAGE_REF / PH_ADC_RESOLUTION);
  
  return voltage;
}

// ============================================
// 안정된 전압 측정 (진행 표시 포함)
// ============================================
float measureVoltageStable() {
  Serial.println(F("[측정 중] 센서 안정화 중..."));
  
  float sum = 0;
  int validSamples = 0;
  
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int rawValue = analogRead(PH_SENSOR_PIN);
    float voltage = rawValue * (PH_VOLTAGE_REF / PH_ADC_RESOLUTION);
    
    sum += voltage;
    validSamples++;
    
    // 진행 표시 (5샘플마다)
    if ((i + 1) % 5 == 0) {
      Serial.print(F("."));
    }
    
    delay(SAMPLE_DELAY);
  }
  
  Serial.println();
  
  float avgVoltage = sum / validSamples;
  return avgVoltage;
}

// ============================================
// 칼리브레이션 계산
// ============================================
void calculateCalibration() {
  Serial.println(F("\n[CALC] 칼리브레이션 계수 계산 중..."));
  
  // Offset 계산 (중성점 보정)
  // pH 7.00에서 전압이 2.5V가 되어야 이상적
  calibData.offset = 7.0 - (7.0 - (calibData.voltage_ph7 - 2.5) / 0.18);
  
  // Slope 계산 (감도)
  // 이론적으로 pH 1 변화당 약 0.18V (=3.5 / 18.0)
  // 실제 측정값으로 기울기 보정
  
  // pH 4~7 구간 기울기
  float slope_4_7 = (PH_STANDARD_7 - PH_STANDARD_4) / 
                    (calibData.voltage_ph7 - calibData.voltage_ph4);
  
  // pH 7~10 구간 기울기
  float slope_7_10 = (PH_STANDARD_10 - PH_STANDARD_7) / 
                     (calibData.voltage_ph10 - calibData.voltage_ph7);
  
  // 평균 기울기
  float avgSlope = (slope_4_7 + slope_7_10) / 2.0;
  
  // Slope를 3.5 기준으로 변환 (config.h 호환)
  calibData.slope = 3.5 / (avgSlope * 18.0);
  
  calibData.isValid = true;
  
  Serial.println(F("[CALC] 계산 완료!"));
}

// ============================================
// 칼리브레이션 결과 출력
// ============================================
void printCalibrationResults() {
  Serial.println(F("\n========================================"));
  Serial.println(F("     칼리브레이션 결과"));
  Serial.println(F("========================================"));
  Serial.println(F("\n[측정 전압]"));
  Serial.print(F("  pH 4.00:  "));
  Serial.print(calibData.voltage_ph4, 3);
  Serial.println(F(" V"));
  Serial.print(F("  pH 7.00:  "));
  Serial.print(calibData.voltage_ph7, 3);
  Serial.println(F(" V"));
  Serial.print(F("  pH 10.00: "));
  Serial.print(calibData.voltage_ph10, 3);
  Serial.println(F(" V"));
  
  Serial.println(F("\n[계산된 보정 계수]"));
  Serial.print(F("  Offset: "));
  Serial.println(calibData.offset, 4);
  Serial.print(F("  Slope:  "));
  Serial.println(calibData.slope, 4);
  
  Serial.println(F("\n[config.h에 적용할 값]"));
  Serial.println(F("  #define PH_CALIBRATION_OFFSET ") + String(calibData.offset, 4));
  Serial.println(F("  #define PH_CALIBRATION_SLOPE  ") + String(calibData.slope, 4));
  
  // 검증 테스트
  Serial.println(F("\n[검증 테스트]"));
  Serial.print(F("  pH 4.00:  측정 pH = "));
  Serial.println(voltageToPH(calibData.voltage_ph4), 2);
  Serial.print(F("  pH 7.00:  측정 pH = "));
  Serial.println(voltageToPH(calibData.voltage_ph7), 2);
  Serial.print(F("  pH 10.00: 측정 pH = "));
  Serial.println(voltageToPH(calibData.voltage_ph10), 2);
  
  Serial.println(F("========================================\n"));
}

// ============================================
// 전압을 pH로 변환
// ============================================
float voltageToPH(float voltage) {
  if (!calibData.isValid) {
    // 기본값 사용
    return 7.0 - ((voltage - 2.5) / 0.18);
  }
  
  // 보정값 적용
  float ph = 7.0 + calibData.offset - ((voltage - 2.5) / (calibData.slope / 18.0));
  return ph;
}

// ============================================
// EEPROM에 칼리브레이션 데이터 저장
// ============================================
void saveCalibration() {
  Serial.println(F("\n[SAVE] EEPROM에 저장 중..."));
  
  // 매직 넘버 저장
  uint32_t magic = EEPROM_MAGIC_NUMBER;
  EEPROM.put(EEPROM_MAGIC_ADDR, magic);
  
  // Offset 저장
  EEPROM.put(EEPROM_OFFSET_ADDR, calibData.offset);
  
  // Slope 저장
  EEPROM.put(EEPROM_SLOPE_ADDR, calibData.slope);
  
  Serial.println(F("[SAVE] 저장 완료!"));
}

// ============================================
// EEPROM에서 칼리브레이션 데이터 불러오기
// ============================================
void loadCalibration() {
  Serial.println(F("\n[LOAD] EEPROM에서 불러오는 중..."));
  
  // 매직 넘버 확인
  uint32_t magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);
  
  if (magic != EEPROM_MAGIC_NUMBER) {
    Serial.println(F("[LOAD] 저장된 칼리브레이션 데이터가 없습니다."));
    calibData.isValid = false;
    calibData.offset = 0;
    calibData.slope = 3.5;  // 기본값
    return;
  }
  
  // Offset 불러오기
  EEPROM.get(EEPROM_OFFSET_ADDR, calibData.offset);
  
  // Slope 불러오기
  EEPROM.get(EEPROM_SLOPE_ADDR, calibData.slope);
  
  calibData.isValid = true;
  
  Serial.println(F("[LOAD] 불러오기 완료!"));
  Serial.print(F("[LOAD] Offset: "));
  Serial.println(calibData.offset, 4);
  Serial.print(F("[LOAD] Slope:  "));
  Serial.println(calibData.slope, 4);
}

// ============================================
// 사용자 확인 대기 ('c' 입력)
// ============================================
void waitForConfirm() {
  while (true) {
    if (Serial.available() > 0) {
      char ch = Serial.read();
      while (Serial.available() > 0) Serial.read();  // 버퍼 비우기
      
      if (ch == 'c' || ch == 'C') {
        break;
      }
    }
    delay(100);
  }
}

// ============================================
// Yes/No 입력 대기
// ============================================
char waitForYesNo() {
  while (true) {
    if (Serial.available() > 0) {
      char ch = Serial.read();
      while (Serial.available() > 0) Serial.read();  // 버퍼 비우기
      
      if (ch == 'y' || ch == 'Y' || ch == 'n' || ch == 'N') {
        return ch;
      }
    }
    delay(100);
  }
}
