// SystemModes.cpp
#include "SystemModes.h"
#include "config.h"  // Добавьте эту строку!

// Теперь инициализируем режим на основе макросов из config.h
OperationMode SystemModes::_currentMode = 
    #if defined(MODE_TRACKER)
        OperationMode::TRACKER_MODE
    #elif defined(MODE_SLEEP)
        OperationMode::SLEEP_MODE
    #else
        OperationMode::DEBUG_MODE
    #endif
;

bool SystemModes::_modeSwitchRequested = false;
unsigned long SystemModes::_gpsStartTime = 0;
unsigned long SystemModes::_simStartTime = 0;
bool SystemModes::_gpsCompleted = false;
bool SystemModes::_simCompleted = false;
bool SystemModes::_oneCycleComplete = false;
bool SystemModes::_gpsDataReceived = false;
bool SystemModes::_simCommandsCompleted = false;

void SystemModes::begin() {
  pinMode(LED_BUILTIN, OUTPUT);
  indicateModeChange();  // Теперь покажет правильный режим
}

void SystemModes::setMode(OperationMode mode) {
  if (_currentMode != mode) {
    _currentMode = mode;
    _modeSwitchRequested = true;
    indicateModeChange();
    
    // Сброс состояния при смене режима
    if (mode == OperationMode::TRACKER_MODE) {
      resetTrackerCycle();
    }
  }
}

OperationMode SystemModes::getCurrentMode() {
  return _currentMode;
}

void SystemModes::indicateModeChange() {
  // Визуальная индикация режима (мигание светодиодом)
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  
  // Дополнительная индикация в Serial
  Serial.println("==========================================");
  switch (_currentMode) {
    case OperationMode::DEBUG_MODE:
      Serial.println(">>> MODE: DEBUG (All modules active) <<<");
      break;
    case OperationMode::TRACKER_MODE:
      Serial.println(">>> MODE: TRACKER (Cyclic GPS->SIM) <<<");
      break;
    case OperationMode::SLEEP_MODE:
      Serial.println(">>> MODE: SLEEP (Wake on MPU movement) <<<");
      break;
  }
  Serial.println("==========================================");
}

void SystemModes::forceReset() {
  _gpsCompleted = false;
  _simCompleted = false;
  _oneCycleComplete = false;
  _gpsDataReceived = false;
  _simCommandsCompleted = false;  // Исправлено
  _gpsStartTime = 0;
  _simStartTime = 0;
}

void SystemModes::resetTrackerCycle() {
  _gpsCompleted = false;
  _simCompleted = false;
  _oneCycleComplete = false;
  _gpsDataReceived = false;
  _simCommandsCompleted = false;  // Исправлено
  _gpsStartTime = 0;
  _simStartTime = 0;
  
  Serial.println("[Tracker] Cycle reset - starting with GPS");
}

void SystemModes::updateTrackerState(bool gpsHasData, bool simCommandsDone) {
  if (_currentMode != OperationMode::TRACKER_MODE && 
      _currentMode != OperationMode::SLEEP_MODE) {
    return;
  }
  
  // Обновляем состояние GPS
  if (!_gpsCompleted && gpsHasData) {
    _gpsDataReceived = true;
    _gpsCompleted = true;
    Serial.println("[Tracker] GPS data received - switching to SIM");
  }
  
  // Обновляем состояние SIM
  if (!_simCompleted && simCommandsDone) {
    _simCommandsCompleted = true;
    _simCompleted = true;
    _oneCycleComplete = true;
    Serial.println("[Tracker] SIM commands completed - cycle finished");
  }
}

bool SystemModes::shouldGPSBeActive() {
  if (_currentMode == OperationMode::DEBUG_MODE) {
    return true;  // В режиме отладки GPS всегда активен
  }
  
  if (_currentMode == OperationMode::TRACKER_MODE) {
    // GPS активен, пока не получит данные
    return !_gpsCompleted;
  }
  
  if (_currentMode == OperationMode::SLEEP_MODE) {
    // В режиме сна GPS активен только до получения данных (один цикл)
    return (!_gpsCompleted && !_oneCycleComplete);
  }
  
  return false;
}

bool SystemModes::shouldSIMBeActive() {
  if (_currentMode == OperationMode::DEBUG_MODE) {
    return true;  // В режиме отладки SIM всегда активен
  }
  
  if (_currentMode == OperationMode::TRACKER_MODE) {
    // SIM активен, только после получения данных GPS
    return (_gpsCompleted && !_simCompleted);
  }
  
  if (_currentMode == OperationMode::SLEEP_MODE) {
    // В режиме сна SIM активен только один цикл после GPS
    return (_gpsCompleted && !_simCompleted && !_oneCycleComplete);
  }
  
  return false;
}

bool SystemModes::isTrackerCycleComplete() {
  if (_currentMode == OperationMode::TRACKER_MODE) {
    return _oneCycleComplete;
  }
  
  if (_currentMode == OperationMode::SLEEP_MODE) {
    return _oneCycleComplete;
  }
  
  return false;
}

void SystemModes::goToSleep() {
  if (_currentMode != OperationMode::SLEEP_MODE) {
    return;  // Только в режиме сна можно спать
  }
  
  Serial.println("[Sleep] Entering deep sleep...");
  Serial.flush();
  delay(100);
  
  // Настройка прерывания на MPU6050 (INT пин)
  // Предполагаем, что MPU подключен к пину 2 (прерывание 0)
  attachInterrupt(digitalPinToInterrupt(2), wakeUp, RISING);
  
  // Отключаем ненужные модули
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  
  // Отключаем ADC и таймеры для экономии
  ADCSRA &= ~(1 << ADEN);  // Выключаем ADC
  power_all_disable();      // Отключаем все периферийные модули
  
  sleep_mode();  // Уход в сон
  
  // Код после пробуждения
  sleep_disable();
  ADCSRA |= (1 << ADEN);   // Включаем ADC обратно
  power_all_enable();       // Включаем периферию
  
  detachInterrupt(digitalPinToInterrupt(2));
  
  Serial.println("[Sleep] Woken up by MPU interrupt!");
  resetTrackerCycle();  // Сбрасываем цикл для нового измерения
}

void SystemModes::wakeUp() {
  // Функция-обработчик прерывания
  // Минимальная работа - просто выход из сна
}