#include "NEO.h"

// Конструктор
NEO::NEO(int rxPin, int txPin) {
  _rxPin = rxPin;
  _txPin = txPin;
  _gpsSerial = new SoftwareSerial(rxPin, txPin);
  _hasNewData = false;
  _buffer = "";
  _lineBuffer = "";  // НОВОЕ: буфер для готовой строки
  _enabled = true;
}

// Инициализация
void NEO::begin(long baudrate) {
  if (_enabled) {
    _gpsSerial->begin(baudrate);
    delay(100);
    // Очищаем мусор после запуска
    while (_gpsSerial->available()) {
      _gpsSerial->read();
    }
    _buffer = "";
    _lineBuffer = "";
    _hasNewData = false;
  }
}

// Включить модуль
void NEO::enable() {
  _enabled = true;
  _gpsSerial->begin(9600);
  delay(100);
  while (_gpsSerial->available()) {
    _gpsSerial->read();
  }
}

// Выключить модуль
void NEO::disable() {
  _enabled = false;
  _gpsSerial->end();
  _buffer = "";
  _lineBuffer = "";
  _hasNewData = false;
}

// Проверка, включён ли модуль
bool NEO::isEnabled() {
  return _enabled;
}

// Установить состояние включения
void NEO::setEnabled(bool enabled) {
  if (enabled && !_enabled) {
    enable();
  } else if (!enabled && _enabled) {
    disable();
  }
}

// Проверка наличия данных
bool NEO::available() {
  if (!_enabled) return false;
  return _gpsSerial->available();
}

// ============= ИСПРАВЛЕННЫЙ МЕТОД update() =============
void NEO::update() {
  if (!_enabled) return;
  
  while (_gpsSerial->available()) {
    char c = _gpsSerial->read();
    
    if (c == '\n') {
      // Нашли конец строки
      _lineBuffer = _buffer;     // Сохраняем строку
      _buffer = "";              // Очищаем для следующей
      _hasNewData = true;
    } 
    else if (c != '\r') {
      // Добавляем символ в текущую строку
      _buffer += c;
      
      // Защита от переполнения (максимум 255 символов на строку)
      if (_buffer.length() > 255) {
        _buffer = "";
      }
    }
  }
}

// ============= ИСПРАВЛЕННЫЙ МЕТОД getRawData() =============
String NEO::getRawData() {
  if (!_enabled) return "";
  
  if (_hasNewData) {
    _hasNewData = false;
    
    // Проверяем, что строка валидная (начинается с $)
    if (_lineBuffer.length() > 0 && _lineBuffer[0] == '$') {
      return _lineBuffer;
    }
  }
  return "";
}

// Получить указатель на SoftwareSerial
SoftwareSerial* NEO::getSerial() {
  return _gpsSerial;
}