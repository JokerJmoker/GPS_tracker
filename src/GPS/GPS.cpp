// =====================================================
// FILE: src/GPS/GPS.cpp (ОПТИМИЗИРОВАННЫЙ)
// =====================================================

#include "GPS.h"

GPS::GPS(int rxPin, int txPin) {
    _rxPin = rxPin;
    _txPin = txPin;
    _gpsSerial = new SoftwareSerial(rxPin, txPin);
    _bufferIndex = 0;
    _lineStart = 0;
    _lineEnd = 0;
    _flags = 0x02;  // enabled = true (бит 1)
    memset(_buffer, 0, BUFFER_SIZE);
}

GPS::~GPS() {
    delete _gpsSerial;
}

void GPS::begin(long baudrate) {
    if (!(_flags & 0x02)) return;
    
    _gpsSerial->begin(baudrate);
    delay(100);
    
    // Очистка буфера
    while (_gpsSerial->available()) _gpsSerial->read();
    _bufferIndex = 0;
    _lineStart = 0;
    _lineEnd = 0;
    _flags &= ~0x01;  // clear _hasNewData
    memset(_buffer, 0, BUFFER_SIZE);
}

void GPS::enable() {
    _flags |= 0x02;  // enabled = true
    _gpsSerial->begin(9600);
    delay(100);
    while (_gpsSerial->available()) _gpsSerial->read();
}

void GPS::disable() {
    _flags &= ~0x02;  // enabled = false
    _gpsSerial->end();
    _bufferIndex = 0;
    _lineStart = 0;
    _lineEnd = 0;
    _flags &= ~0x01;  // clear _hasNewData
}

void GPS::setEnabled(bool enabled) {
    if (enabled && !(_flags & 0x02)) enable();
    else if (!enabled && (_flags & 0x02)) disable();
}

bool GPS::available() const {
    if (!(_flags & 0x02)) return false;
    return _gpsSerial->available() > 0;
}

void GPS::update() {
    if (!(_flags & 0x02)) return;
    
    while (_gpsSerial->available()) {
        char c = _gpsSerial->read();
        
        if (c == '\n') {
            // Сохраняем позицию готовой строки
            _lineStart = 0;
            _lineEnd = _bufferIndex;
            _flags |= 0x01;  // _hasNewData = true
            _bufferIndex = 0;
        } 
        else if (c != '\r' && _bufferIndex < BUFFER_SIZE - 1) {
            _buffer[_bufferIndex++] = c;
        }
        else if (_bufferIndex >= BUFFER_SIZE - 1) {
            // Переполнение - сброс
            _bufferIndex = 0;
            _flags &= ~0x01;
        }
    }
}

const char* GPS::getRawData() {
    if (!(_flags & 0x02) || !(_flags & 0x01)) return nullptr;
    
    // Проверяем, что строка начинается с '$'
    if (_lineEnd > 0 && _buffer[0] == '$') {
        _flags &= ~0x01;  // clear _hasNewData
        _buffer[_lineEnd] = '\0';
        return _buffer;
    }
    
    _flags &= ~0x01;
    return nullptr;
}

void GPS::printRawData(HardwareSerial& serial) {
    if (!(_flags & 0x02) || !(_flags & 0x01)) return;
    
    if (_lineEnd > 0 && _buffer[0] == '$') {
        for (uint8_t i = 0; i < _lineEnd; i++) {
            serial.write(_buffer[i]);
        }
        serial.write('\n');
        _flags &= ~0x01;
    }
}