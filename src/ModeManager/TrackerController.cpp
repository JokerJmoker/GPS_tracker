#include "TrackerController.h"

TrackerController::TrackerController(NEO* gps, SIM* sim, MPU* mpu) {
  _gps = gps;
  _sim = sim;
  _mpu = mpu;
  
  _gpsDataReady = false;
  _simCommandsDone = false;
  _lastCommandTime = 0;
  _commandIndex = 0;
  _mpuMovementDetected = false;
}

void TrackerController::begin() {
  resetSimCommandSequence();
}

void TrackerController::resetSimCommandSequence() {
  _commandIndex = 0;
  _simCommandsDone = false;
  _lastCommandTime = 0;
}

void TrackerController::sendNextSimCommand() {
  // Команды для SIM800L
  const char* commands[] = {
    "AT+CFUN=1",      // Включить радио
    "AT",             // Проверка связи
    "AT+CPIN?",       // Проверка SIM
    "AT+CCID",        // ID SIM-карты
    "AT+CREG=1",      // Включить регистрацию
    "AT+CREG?",       // Статус регистрации
    "AT+COPS=0",      // Автопоиск оператора
    "AT+COPS?",       // Какой оператор найден
    "AT+CSQ",         // Уровень сигнала
    "AT+CBC"          // Напряжение
  };
  const int numCommands = 10;
  
  if (_commandIndex < numCommands) {
    _sim->sendCommand(commands[_commandIndex]);
    Serial.print("[SIM Command] ");
    Serial.println(commands[_commandIndex]);
    _commandIndex++;
    _lastCommandTime = millis();
  } else {
    // Все команды выполнены
    _simCommandsDone = true;
    Serial.println("[SIM] All commands completed");
  }
}

void TrackerController::update() {
  // Обновляем GPS
  _gps->update();
  
  // Проверяем, получили ли данные GPS
  String gpsData = _gps->getRawData();
  if (gpsData.length() > 0 && !_gpsDataReady) {
    _gpsDataReady = true;
    Serial.println("[GPS] Data received!");
    Serial.println(gpsData);
  }
  
  // Обновляем SIM если он активен
  if (SystemModes::shouldSIMBeActive()) {
    _sim->update();
    
    // Получаем ответы от SIM
    String simData = _sim->getData();
    if (simData.length() > 0) {
      Serial.print("[SIM Response] ");
      Serial.println(simData);
    }
    
    // Отправляем следующую команду (раз в 3 секунды)
    if (!_simCommandsDone && millis() - _lastCommandTime >= 3000) {
      sendNextSimCommand();
    }
  }
  
  // Обновляем состояние трекера
  SystemModes::updateTrackerState(_gpsDataReady, _simCommandsDone);
  
  // В режиме сна проверяем MPU
  if (SystemModes::getCurrentMode() == OperationMode::SLEEP_MODE) {
    _mpu->update();
    // Проверка движения по акселерометру
    if (_mpu->available()) {
      String mpuData = _mpu->getData();
      // Упрощенная проверка - если есть данные, считаем движение
      if (mpuData.length() > 0) {
        _mpuMovementDetected = true;
        SystemModes::wakeUp();
      }
    }
  }
}

bool TrackerController::isGPSDataReady() {
  return _gpsDataReady;
}

bool TrackerController::isSIMCommandsDone() {
  return _simCommandsDone;
}

bool TrackerController::isMPUMovementDetected() {
  return _mpuMovementDetected;
}

void TrackerController::forceGPSRead() {
  _gpsDataReady = false;
  // Принудительное чтение GPS
  _gps->update();
  _gps->getRawData();  // Триггер чтения
}

void TrackerController::forceSIMCommands() {
  resetSimCommandSequence();
  _simCommandsDone = false;
}

void TrackerController::resetCycle() {
  _gpsDataReady = false;
  _simCommandsDone = false;
  _commandIndex = 0;
  _lastCommandTime = 0;
  Serial.println("[TrackerController] Internal state reset");
}