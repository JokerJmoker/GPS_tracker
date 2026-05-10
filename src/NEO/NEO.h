#ifndef NEO_H
#define NEO_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class NEO {
  private:
    SoftwareSerial* _gpsSerial;  // Указатель на объект программного UART для связи с GPS
    int _rxPin;                   // Пин для приёма данных (RX) от GPS модуля
    int _txPin;                   // Пин для передачи данных (TX) в GPS модуль
    String _buffer;               // Буфер для накопления строки NMEA до символа \n
    bool _hasNewData;             // Флаг наличия новой полной строки данных
    bool _enabled;                // Флаг включения/выключения модуля (логическое, не физическое)
    String _lineBuffer;   // НОВОЕ: буфер для готовой строки
    
  public:
    // Конструктор - сохраняет пины и создаёт объект SoftwareSerial
    NEO(int rxPin, int txPin);
    
    // Инициализация - запускает UART с заданной скоростью (по умолчанию 9600)
    void begin(long baudrate = 9600);
    
    // Включение модуля - запускает UART и устанавливает флаг _enabled
    void enable();
    
    // Выключение модуля - закрывает UART порт и очищает буфер
    void disable();
    
    // Проверка, включён ли модуль (возвращает состояние флага _enabled)
    bool isEnabled();
    
    // Установка состояния включения/выключения
    void setEnabled(bool enabled);
    
    // Проверка наличия данных в буфере UART
    bool available();
    
    // Обновление данных - чтение из UART и накопление строк (вызывать в loop)
    void update();
    
    // Получение сырых данных - возвращает одну полную строку NMEA
    String getRawData();
    
    // Получить указатель на SoftwareSerial для низкоуровневого доступа
    SoftwareSerial* getSerial();
};

#endif