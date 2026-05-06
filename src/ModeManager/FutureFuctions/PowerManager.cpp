#include "PowerManager.h"

volatile bool wakeFlag = false;

PowerManager::PowerManager(NEO* gps, SIM* sim, MPU* mpu) {
  _gps = gps;
  _sim = sim;
  _mpu = mpu;
  
  _trackingState = STATE_GPS_ACTIVE;
  _gpsDataReady = false;
  _simWorkDone = false;
  _trackingCycleActive = false;
  _simCommandIndex = 0;
  _wakeByMPU = false;
  _mpuDetectedMotion = false;
  _oneCycleComplete = false;
}

void PowerManager::begin() {
  // Настройка MPU для прерывания (если поддерживается)
  // TODO: Настроить пин прерывания MPU
  pinMode(2, INPUT_PULLUP); // Пример: прерывание на пине 2
}

void PowerManager::wakeUpISR() {
  wakeFlag = true;
}

bool PowerManager::isWakeByMPU() {
  return wakeFlag;
}

void PowerManager::clearWakeFlag() {
  wakeFlag = false;
}

void PowerManager::setGPSDataReady(bool ready) {
  _gpsDataReady = ready;
}

void PowerManager::setSIMWorkDone(bool done) {
  _simWorkDone = done;
}

bool PowerManager::isOneCycleComplete() {
  return _oneCycleComplete;
}

void PowerManager::resetTrackingCycle() {
  _trackingState = STATE_GPS_ACTIVE;
  _gpsDataReady = false;
  _simWorkDone = false;
  _trackingCycleActive = true;
  _simCommandIndex = 0;
  _oneCycleComplete = false;
}

void PowerManager::processTrackingMode() {
  if (!_trackingCycleActive) {
    resetTrackingCycle();
  }
  
  switch(_trackingState) {
    case STATE_GPS_ACTIVE:
      // GPS активен, SIM спит
      _sim->disable();
      _gps->enable();
      
      Serial.println("[POWER] GPS active, SIM sleeping...");
      
      // Даем время GPS получить данные
      static unsigned long gpsStartTime = millis();
      if (millis() - gpsStartTime >= 5000) { // 5 секунд на получение данных
        _gpsDataReady = true;
      }
      
      // Проверяем, получили ли данные
      if (_gps->available()) {
        String data = _gps->getRawData();
        if (data.length() > 0) {
          _gpsDataReady = true;
          Serial.println("[POWER] GPS data received!");
        }
      }
      
      if (_gpsDataReady) {
        _trackingState = STATE_SIM_ACTIVE;
        Serial.println("[POWER] Switching to SIM...");
      }
      break;
      
    case STATE_SIM_ACTIVE:
      // SIM активен, GPS спит
      _gps->disable();
      _sim->enable();
      
      if (!_simWorkDone) {
        // Массив команд для SIM
        const char* commands[] = {
          "AT+CFUN=1", "AT", "AT+CPIN?", "AT+CCID",
          "AT+CREG=1", "AT+CREG?", "AT+COPS=0",
          "AT+COPS?", "AT+CSQ", "AT+CBC"
        };
        int numCommands = 10;
        
        // Отправляем команды по очереди
        static unsigned long lastSent = 0;
        if (millis() - lastSent >= 2000) {
          if (_simCommandIndex < numCommands) {
            Serial.print("[POWER] SIM send: ");
            Serial.println(commands[_simCommandIndex]);
            _sim->sendCommand(commands[_simCommandIndex]);
            lastSent = millis();
            _simCommandIndex++;
          } else {
            _simWorkDone = true;
          }
        }
        
        // Получаем ответы
        String response = _sim->getData();
        if (response.length() > 0) {
          Serial.print("[POWER] SIM response: ");
          Serial.println(response);
        }
      }
      
      if (_simWorkDone) {
        // Цикл завершен
        _trackingCycleActive = false;
        _oneCycleComplete = true;
        Serial.println("[POWER] Tracking cycle complete!");
      }
      break;
  }
}

void PowerManager::processSleepMode() {
  // Выключаем все модули
  _gps->disable();
  _sim->disable();
  
  // Настраиваем MPU для пробуждения по движению
  Serial.println("[POWER] Entering SLEEP mode...");
  Serial.println("[POWER] Waiting for MPU motion detection...");
  
  // Настройка прерывания MPU (пример для MPU6050 с библиотекой I2Cdev)
  // attachInterrupt(digitalPinToInterrupt(2), wakeUpISR, RISING);
  
  // Усыпляем Arduino
  // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // sleep_enable();
  // sleep_mode();
  
  // Эмуляция сна (в реальном проекте - реальный сон)
  delay(1000); // Имитация сна
  
  // При пробуждении (в ISR)
  if (isWakeByMPU()) {
    clearWakeFlag();
    Serial.println("[POWER] Wake up by MPU motion!");
    
    // Запускаем один цикл трекинга
    resetTrackingCycle();
    
    // Выполняем один цикл
    while (!_oneCycleComplete) {
      processTrackingMode();
    }
    
    // Снова засыпаем после одного цикла
    _oneCycleComplete = false;
    Serial.println("[POWER] One cycle complete, going back to sleep...");
    // Снова усыпляем (рекурсивно)
    processSleepMode();
  }
}

void PowerManager::update() {
  // Этот метод будет вызываться в loop() и обрабатывать текущий режим
  // Реальная логика будет в main.cpp или TestManager
}