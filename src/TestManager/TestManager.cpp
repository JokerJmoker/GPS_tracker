#include "TestManager.h"

TestManager::TestManager(NEO* gps, SIM* sim800l, MPU* mpu6050) {
  _gps = gps;           // Сохраняем указатели на внешние объекты
  _sim800l = sim800l;
  _mpu6050 = mpu6050;
  
  _lastStatus = 0;      // Время последнего вывода статуса GPS
  _lastPrint = 0;       // Время последней отправки AT команды
  _lastVoltageCheck = 0;// Время последней проверки напряжения
  _lastMpuPrint = 0;    // Время последнего вывода MPU
  _cmdIndex = 0;        // Индекс текущей команды для SIM
  
  //initCommands();       // Заполняем массив AT команд
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
    static unsigned long startTime = 0;
    static bool firstRun = true;
    
    if (firstRun) {
      startTime = millis();
      firstRun = false;
      Serial.println("[GPS] Starting...");
    }
    
    _gps->update();
    
    // Прямой вывод - самый эффективный
    _gps->printRawData(Serial);
  #endif
}

void TestManager::processSIM() {

#ifdef TEST_SIM800L

    static unsigned long lastCommand = 0;

    static unsigned long waitStart = 0;

    static int step = 0;

    static bool waitingResponse = false;

    const unsigned long COMMAND_DELAY = 2000;

    const unsigned long RESPONSE_TIMEOUT = 30000;

    _sim800l->update();

    // =====================================================
    // FULL RESPONSE
    // =====================================================

    String fullResponse = _sim800l->getFullResponse();

    if (fullResponse.length() > 0) {

        Serial.println("[SIM] Full response:");
        Serial.println(fullResponse);

        // Ответ получен
        if (waitingResponse) {

            waitingResponse = false;

            lastCommand = millis();

            // Проверяем ERROR
            if (fullResponse.indexOf("ERROR") >= 0) {

                Serial.println("[SIM] COMMAND FAILED");
            }
            else {

                Serial.println("[SIM] COMMAND OK");
            }

            step++;
        }
    }

    // =====================================================
    // OPTIONAL LINE DEBUG
    // =====================================================

    String line;

    while ((line = _sim800l->getData()).length() > 0) {

        Serial.print("[SIM] Line: ");
        Serial.println(line);
    }

    // =====================================================
    // RESPONSE TIMEOUT
    // =====================================================

    if (waitingResponse &&
        millis() - waitStart >= RESPONSE_TIMEOUT) {

        Serial.println("[SIM] RESPONSE TIMEOUT");

        waitingResponse = false;

        lastCommand = millis();

        // Можно:
        // 1. повторить команду
        // 2. пропустить
        // 3. рестартнуть SIM

        Serial.print("[SIM] SKIPPING COMMAND: ");
        
        const char* failedCommands[] = {
            "AT+CFUN=1",
            "AT",
            "AT+CPIN?",
            "AT+CCID",
            "AT+CREG=1",
            "AT+CREG?",
            "AT+COPS=0",
            "AT+COPS?",
            "AT+CSQ",
            "AT+CBC"
        };

        Serial.println(failedCommands[step]);

        step++;
    }

    // =====================================================
    // SEND COMMANDS
    // =====================================================

    if (!waitingResponse &&
        millis() - lastCommand >= COMMAND_DELAY) {

        const char* commands[] = {

            "AT+CFUN=1",
            "AT",
            "AT+CPIN?",
            "AT+CCID",
            "AT+CREG=1",
            "AT+CREG?",
            "AT+COPS=0",
            "AT+COPS?",
            "AT+CSQ",
            "AT+CBATCHK=1",
            "AT+CBC"
        };

        const int COMMAND_COUNT =
            sizeof(commands) / sizeof(commands[0]);

        if (step < COMMAND_COUNT) {

            Serial.print("[SIM] Sending: ");
            Serial.println(commands[step]);

            _sim800l->sendCommand(commands[step]);

            waitingResponse = true;

            waitStart = millis();
        }
        else {

            Serial.println("[SIM] TEST COMPLETE");

            step = 0;

            delay(5000);
        }
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