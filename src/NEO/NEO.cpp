#include "NEO.h"

NEO::NEO(int rxPin, int txPin) {
  _rxPin = rxPin;
  _txPin = txPin;
  _gpsSerial = new SoftwareSerial(rxPin, txPin);
  _hasNewData = false;
  _bufferIndex = 0;
  _lineIndex = 0;
  _enabled = true;
  
  memset(_buffer, 0, sizeof(_buffer));
  memset(_lineBuffer, 0, sizeof(_lineBuffer));
}

void NEO::begin(long baudrate) {
  if (_enabled) {
    _gpsSerial->begin(baudrate);
    delay(100);
    while (_gpsSerial->available()) {
      _gpsSerial->read();
    }
    _bufferIndex = 0;
    _lineIndex = 0;
    _hasNewData = false;
    memset(_buffer, 0, sizeof(_buffer));
    memset(_lineBuffer, 0, sizeof(_lineBuffer));
  }
}

void NEO::enable() {
  _enabled = true;
  _gpsSerial->begin(9600);
  delay(100);
  while (_gpsSerial->available()) {
    _gpsSerial->read();
  }
}

void NEO::disable() {
  _enabled = false;
  _gpsSerial->end();
  _bufferIndex = 0;
  _lineIndex = 0;
  _hasNewData = false;
}

bool NEO::isEnabled() {
  return _enabled;
}

void NEO::setEnabled(bool enabled) {
  if (enabled && !_enabled) {
    enable();
  } else if (!enabled && _enabled) {
    disable();
  }
}

bool NEO::available() {
  if (!_enabled) return false;
  return _gpsSerial->available();
}

void NEO::update() {
  if (!_enabled) return;
  
  while (_gpsSerial->available()) {
    char c = _gpsSerial->read();
    
    if (c == '\n') {
      // Копируем буфер в lineBuffer
      _lineIndex = _bufferIndex;
      memcpy(_lineBuffer, _buffer, _bufferIndex);
      _lineBuffer[_lineIndex] = '\0';  // null-terminator
      
      _bufferIndex = 0;
      memset(_buffer, 0, sizeof(_buffer));
      _hasNewData = true;
    } 
    else if (c != '\r' && _bufferIndex < 254) {
      _buffer[_bufferIndex++] = c;
    }
    else if (_bufferIndex >= 254) {
      // Переполнение - сбрасываем буфер
      _bufferIndex = 0;
      memset(_buffer, 0, sizeof(_buffer));
    }
  }
}

// Возвращает указатель на внутренний буфер (НЕ копирует)
const char* NEO::getRawData() {
  if (!_enabled || !_hasNewData) return nullptr;
  
  if (_lineIndex > 0 && _lineBuffer[0] == '$') {
    _hasNewData = false;
    return _lineBuffer;
  }
  
  _hasNewData = false;
  return nullptr;
}

// Копирует данные в предоставленный буфер (безопасно)
void NEO::getRawDataAsString(char* output, size_t maxLen) {
  if (!output || maxLen == 0) return;
  
  const char* data = getRawData();
  if (data) {
    strncpy(output, data, maxLen - 1);
    output[maxLen - 1] = '\0';
  } else {
    output[0] = '\0';
  }
}

// Прямой вывод в Serial (экономит RAM полностью)
void NEO::printRawData(HardwareSerial& serial) {
  if (!_enabled || !_hasNewData) return;
  
  if (_lineIndex > 0 && _lineBuffer[0] == '$') {
    // Выводим символ за символом
    for (uint8_t i = 0; i < _lineIndex; i++) {
      serial.write(_lineBuffer[i]);
    }
    serial.write('\n');  // Добавляем перевод строки
    _hasNewData = false;
  }
}

SoftwareSerial* NEO::getSerial() {
  return _gpsSerial;
}