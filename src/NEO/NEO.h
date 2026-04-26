#ifndef NEO_H
#define NEO_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class NEO {
  private:
    SoftwareSerial* _gpsSerial;
    int _rxPin;
    int _txPin;
    String _buffer;
    bool _hasNewData;
    bool _enabled;           // Флаг включения/выключения модуля
    
  public:
    // Конструктор
    NEO(int rxPin, int txPin);
    
    // Инициализация
    void begin(long baudrate = 9600);
    
    // Включение/выключение модуля
    void enable();
    void disable();
    bool isEnabled();
    void setEnabled(bool enabled);
    
    // Проверка, есть ли данные от GPS
    bool available();
    
    // Обновление данных (накопление строк)
    void update();
    
    // Получение сырых данных (одна строка NMEA)
    String getRawData();
    
    // Получить указатель на SoftwareSerial
    SoftwareSerial* getSerial();
};

#endif