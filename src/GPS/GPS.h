// =====================================================
// FILE: src/GPS/GPS.h (ОПТИМИЗИРОВАННЫЙ)
// =====================================================

#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class GPS {
  private:
    SoftwareSerial* _gpsSerial;
    int _rxPin;
    int _txPin;
    
    // ОПТИМИЗАЦИЯ: объединяем два буфера в один с разделением
    char _buffer[256];      // Единый буфер (используется как _buffer и _lineBuffer)
    uint8_t _bufferIndex;
    uint8_t _lineStart;     // Индекс начала готовой строки
    uint8_t _lineEnd;       // Индекс конца готовой строки
    
    uint8_t _flags;         // Бит 0: _hasNewData, Бит 1: _enabled
    
    // Константы
    static const uint8_t BUFFER_SIZE = 128;  // Уменьшаем до 128 байт!
    
  public:
    GPS(int rxPin, int txPin);
    ~GPS();  // Добавляем деструктор
    
    void begin(long baudrate = 9600);
    void enable();
    void disable();
    bool isEnabled() const { return (_flags & 0x02) != 0; }
    void setEnabled(bool enabled);
    bool available() const;
    void update();
    
    const char* getRawData();
    void printRawData(HardwareSerial& serial = Serial);
    
    SoftwareSerial* getSerial() { return _gpsSerial; }
};

#endif