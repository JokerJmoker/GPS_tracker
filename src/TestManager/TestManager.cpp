#include "TestManager.h"

TestManager::TestManager(NEO* gps, SIM* sim800l, MPU* mpu6050) {
  _gps = gps;
  _sim800l = sim800l;
  _mpu6050 = mpu6050;
  
  _lastStatus = 0;
  _lastPrint = 0;
  _lastVoltageCheck = 0;
  _lastMpuPrint = 0;
  _cmdIndex = 0;
  
  initCommands();
}

void TestManager::initCommands() {
  _commands[0] = "AT+CFUN=1";      // 1. ВКЛЮЧИТЬ радио
  _commands[1] = "AT";             // 2. Проверка связи
  _commands[2] = "AT+CPIN?";       // 3. Проверка SIM
  _commands[3] = "AT+CCID";        // 4. ID SIM-карты
  _commands[4] = "AT+CREG=1";      // 5. Включить регистрацию
  _commands[5] = "AT+CREG?";       // 6. Статус регистрации
  _commands[6] = "AT+COPS=0";      // 7. Автопоиск оператора
  _commands[7] = "AT+COPS?";       // 8. Какой оператор найден
  _commands[8] = "AT+CSQ";         // 9. Уровень сигнала
  _commands[9] = "AT+CBC";         // 10. Напряжение
}

void TestManager::printHeader() {
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════╗");
  Serial.println("║           MODULE TEST BENCH                      ║");
  Serial.println("║   Select which module to test in main.cpp       ║");
  Serial.println("╚══════════════════════════════════════════════════╝");
  Serial.println();
}

void TestManager::initModules() {
  #ifdef TEST_GPS
    Serial.println("GPS module ENABLED");
    _gps->begin(9600);
  #else
    _gps->disable();
  #endif
  
  #ifdef TEST_SIM800L
    Serial.println("SIM800L module ENABLED");
    _sim800l->begin(9600);
  #else
    _sim800l->disable();
  #endif
  
  #ifdef TEST_MPU6050
    Serial.println("MPU6050 module ENABLED");
    _mpu6050->begin();
  #else
    _mpu6050->disable();
  #endif
  
  #ifdef TEST_EEPROM
    Serial.println("EEPROM module ENABLED (coming soon)");
  #endif
  
  Serial.println();
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println();
}

void TestManager::updateModules() {
  #ifdef TEST_GPS
    _gps->update();
  #endif
  
  #ifdef TEST_SIM800L
    _sim800l->update();
  #endif
  
  #ifdef TEST_MPU6050
    _mpu6050->update();
  #endif
}

void TestManager::processGPS() {
  #ifdef TEST_GPS
    String data = _gps->getRawData();
    if (data.length() > 0) {
      Serial.println(data);
    }
    
    if (millis() - _lastStatus >= 5000) {
      _lastStatus = millis();
      if (!_gps->available()) {
        Serial.println("[GPS] Waiting for signal...");
        Serial.println();
      }
    }
  #endif
}

void TestManager::processSIM() {
  #ifdef TEST_SIM800L
    // Основное тестирование (каждые 3 секунды)
    if (millis() - _lastPrint >= 3000) {
      _lastPrint = millis();
      Serial.print("[SIM] Sending: ");
      Serial.println(_commands[_cmdIndex]);
      _sim800l->sendCommand(_commands[_cmdIndex]);
      
      _cmdIndex++;
      if (_cmdIndex >= 10) _cmdIndex = 0;
    }
    
    // Проверка напряжения каждые 30 секунд
    if (millis() - _lastVoltageCheck >= 30000) {
      _lastVoltageCheck = millis();
      Serial.println("[SIM] Checking battery voltage...");
      _sim800l->sendCommand("AT+CBC");
    }
    
    // Получаем и выводим ответы
    String simData = _sim800l->getData();
    if (simData.length() > 0) {
      Serial.print("[SIM] Response: ");
      Serial.println(simData);
    }
  #endif
}

void TestManager::processMPU() {
  #ifdef TEST_MPU6050
    if (millis() - _lastMpuPrint >= 1000) {
      _lastMpuPrint = millis();
      
      if (_mpu6050->available()) {
        Serial.print("[MPU] ");
        Serial.println(_mpu6050->getData());
      } else {
        Serial.println("[MPU] Not initialized");
      }
    }
  #endif
}

void TestManager::begin() {
  printHeader();
  initModules();
}

void TestManager::update() {
  updateModules();
  processGPS();
  processSIM();
  processMPU();
  
  #ifdef TEST_EEPROM
    // Код для EEPROM
  #endif
}