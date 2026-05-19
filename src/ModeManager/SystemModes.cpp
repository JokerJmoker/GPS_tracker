// =====================================================
// FILE: src/ModeManager/SystemModes.cpp
// =====================================================

#include "SystemModes.h"
#include "Config.h"  // ВАЖНО: config.h, а не config.h (регистр)

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
  // Убираем pinMode и мигание - они уже есть в setup()
  // pinMode(LED_BUILTIN, OUTPUT);  // ← УБРАТЬ!
  // indicateModeChange();           // ← ВРЕМЕННО УБРАТЬ!
  
  // Просто выводим информацию без лишних задержек
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
  Serial.flush();  // Принудительный вывод
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
  for (int i = 0; i < 3; i++) {  // Уменьшил с 5 до 3 миганий
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);  // Уменьшил задержку
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
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
  Serial.flush();
}

void SystemModes::forceReset() {
  _gpsCompleted = false;
  _simCompleted = false;
  _oneCycleComplete = false;
  _gpsDataReceived = false;
  _simCommandsCompleted = false;
  _gpsStartTime = 0;
  _simStartTime = 0;
}

void SystemModes::resetTrackerCycle() {
  _gpsCompleted = false;
  _simCompleted = false;
  _oneCycleComplete = false;
  _gpsDataReceived = false;
  _simCommandsCompleted = false;
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
    return true;
  }
  
  if (_currentMode == OperationMode::TRACKER_MODE) {
    return !_gpsCompleted;
  }
  
  if (_currentMode == OperationMode::SLEEP_MODE) {
    return (!_gpsCompleted && !_oneCycleComplete);
  }
  
  return false;
}

bool SystemModes::shouldSIMBeActive() {
  if (_currentMode == OperationMode::DEBUG_MODE) {
    return true;
  }
  
  if (_currentMode == OperationMode::TRACKER_MODE) {
    return (_gpsCompleted && !_simCompleted);
  }
  
  if (_currentMode == OperationMode::SLEEP_MODE) {
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

// ВРЕМЕННО: заглушка для goToSleep, чтобы не занимала память
void SystemModes::goToSleep() {
  // Заглушка - не используем режим сна через SystemModes
  Serial.println("[Sleep] WARNING: goToSleep() is disabled (stub)");
}

void SystemModes::wakeUp() {
  // Заглушка
}