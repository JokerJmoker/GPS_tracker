#include "NEO.h"

// Конструктор
NEO::NEO(int rxPin, int txPin) {
  _rxPin = rxPin;
  _txPin = txPin;
  _gpsSerial = new SoftwareSerial(rxPin, txPin);
  _hasNewData = false;
  _buffer = "";
  _enabled = true;           // По умолчанию включён
}

// Инициализация
void NEO::begin(long baudrate) {
  if (_enabled) {
    _gpsSerial->begin(baudrate);
  }
}

// Включить модуль
void NEO::enable() {
  _enabled = true;
  _gpsSerial->begin(9600);
}

// Выключить модуль (освобождает пины)
void NEO::disable() {
  _enabled = false;
  _gpsSerial->end();         // Закрываем порт
  _buffer = "";
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

// Обновление данных (накопление строк)
void NEO::update() {
  if (!_enabled) return;
  
  while (_gpsSerial->available()) {
    char c = _gpsSerial->read();
    if (c == '\n') {
      _hasNewData = true;
    } else if (c != '\r') {
      _buffer += c;
    }
  }
}

// Получение сырых данных
String NEO::getRawData() {
  if (!_enabled) return "";
  
  if (_hasNewData) {
    _hasNewData = false;
    String temp = _buffer;
    _buffer = "";
    return temp;
  }
  return "";
}

// Получить указатель на SoftwareSerial
SoftwareSerial* NEO::getSerial() {
  return _gpsSerial;
}