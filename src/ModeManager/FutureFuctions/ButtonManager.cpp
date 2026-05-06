#include "ButtonManager.h"

ButtonManager::ButtonManager(uint8_t pin) {
  _pin = pin;
  _currentMode = MODE_DEBUG;  // Стартуем в режиме отладки
  _lastPressTime = 0;
  _lastReleaseTime = 0;
  _longPressStart = 0;
  _pressCount = 0;
  _isPressing = false;
  _longPressHandled = false;
  _debounceDelay = 50;
}

void ButtonManager::begin() {
  pinMode(_pin, INPUT_PULLUP);
}

ButtonPress ButtonManager::detectPress() {
  bool currentState = digitalRead(_pin);
  unsigned long now = millis();
  
  // Нажатие (кнопка замкнута на GND, поэтому LOW)
  if (!currentState && !_isPressing) {
    _isPressing = true;
    _longPressStart = now;
    _longPressHandled = false;
  }
  
  // Отпускание
  if (currentState && _isPressing) {
    _isPressing = false;
    
    // Проверяем длинное нажатие (3 секунды)
    if (!_longPressHandled && (now - _longPressStart) >= 3000) {
      _pressCount = 0;
      return PRESS_LONG;
    }
    
    // Обычное нажатие
    if ((now - _lastPressTime) < 500) { // Двойное/тройное в течение 500ms
      _pressCount++;
    } else {
      _pressCount = 1;
    }
    
    _lastPressTime = now;
    _lastReleaseTime = now;
    
    if (_pressCount == 1) return PRESS_SINGLE;
    if (_pressCount == 2) return PRESS_DOUBLE;
    if (_pressCount >= 3) return PRESS_TRIPLE;
  }
  
  // Сброс счетчика если прошло больше 500ms без нажатий
  if (!_isPressing && _pressCount > 0 && (now - _lastPressTime) > 500) {
    _pressCount = 0;
  }
  
  // Длинное нажатие (еще не обработано)
  if (_isPressing && !_longPressHandled && (now - _longPressStart) >= 3000) {
    _longPressHandled = true;
    _pressCount = 0;
    return PRESS_LONG;
  }
  
  return PRESS_NONE;
}

void ButtonManager::update() {
  static TrackerMode lastMode = _currentMode;
  ButtonPress press = detectPress();
  
  switch(press) {
    case PRESS_SINGLE:
      // Одиночное: переключение между DEBUG и TRACKING
      if (_currentMode == MODE_DEBUG) {
        _currentMode = MODE_TRACKING;
      } else if (_currentMode == MODE_TRACKING) {
        _currentMode = MODE_DEBUG;
      }
      indicateMode(_currentMode);
      break;
      
    case PRESS_DOUBLE:
      // Двойное: переключение на SLEEP
      _currentMode = MODE_SLEEP;
      indicateMode(_currentMode);
      break;
      
    case PRESS_TRIPLE:
      // Тройное: вход в тестовый режим (оставляем DEBUG)
      _currentMode = MODE_DEBUG;
      Serial.println("[BUTTON] Triple press - Entering TEST/DEBUG mode");
      indicateMode(_currentMode);
      break;
      
    case PRESS_LONG:
      // Длинное: вход в тестовый режим (DEBUG)
      _currentMode = MODE_DEBUG;
      Serial.println("[BUTTON] Long press - Entering TEST/DEBUG mode");
      indicateMode(_currentMode);
      break;
      
    default:
      break;
  }
  
  if (lastMode != _currentMode) {
    lastMode = _currentMode;
    Serial.print("[BUTTON] Mode changed to: ");
    Serial.println(_currentMode);
  }
}

TrackerMode ButtonManager::getMode() {
  return _currentMode;
}

bool ButtonManager::isModeChanged() {
  static TrackerMode lastMode = _currentMode;
  if (lastMode != _currentMode) {
    lastMode = _currentMode;
    return true;
  }
  return false;
}

void ButtonManager::indicateMode(TrackerMode mode) {
  // Визуальная индикация через встроенный LED (pin 13)
  pinMode(LED_BUILTIN, OUTPUT);
  
  switch(mode) {
    case MODE_DEBUG:
      // 1 вспышка
      for(int i = 0; i < 1; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
      }
      Serial.println("[BUTTON] Mode indicator: 1 flash = DEBUG mode");
      break;
      
    case MODE_TRACKING:
      // 2 вспышки
      for(int i = 0; i < 2; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
      }
      Serial.println("[BUTTON] Mode indicator: 2 flashes = TRACKING mode");
      break;
      
    case MODE_SLEEP:
      // 3 вспышки
      for(int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
      }
      Serial.println("[BUTTON] Mode indicator: 3 flashes = SLEEP mode");
      break;
  }
}