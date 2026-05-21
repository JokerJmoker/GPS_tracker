// =====================================================
// FILE: src/ModeManager/SystemModes.cpp
// =====================================================

#include "SystemModes.h"
#include "Config/Config.h"

// Инициализация режима на основе макросов из config.h
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

void SystemModes::setMode(OperationMode mode) {
  if (_currentMode != mode) {
    _currentMode = mode;
    _modeSwitchRequested = true;
    indicateModeChange();
    
    if (mode == OperationMode::TRACKER_MODE) {
      resetTrackerCycle();
    }
  }
}

OperationMode SystemModes::getCurrentMode() {
  return _currentMode;
}

void SystemModes::indicateModeChange() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
  
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
  
  if (!_gpsCompleted && gpsHasData) {
    _gpsDataReceived = true;
    _gpsCompleted = true;
    Serial.println("[Tracker] GPS data received - switching to SIM");
  }
  
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

void SystemModes::goToSleep() {
  Serial.println("[Sleep] WARNING: goToSleep() is disabled (stub)");
}

void SystemModes::wakeUp() {
  // Заглушка
}