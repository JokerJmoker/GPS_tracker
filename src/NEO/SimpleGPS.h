#ifndef SIMPLE_GPS_H
#define SIMPLE_GPS_H

#include <Arduino.h>

class SimpleGPS {
  private:
    // Данные
    double _latitude;
    double _longitude;
    float _altitude;
    float _speed;
    int _satellites;
    float _hdop;
    String _time;
    String _date;
    bool _hasFix;
    
    // Парсинг GGA строки
    void parseGGA(const String& nmea);
    
    // Парсинг RMC строки
    void parseRMC(const String& nmea);
    
    // Конвертация NMEA формата (ddmm.mmmm) в десятичные градусы
    double convertToDecimal(const String& coord, const String& direction);
    
    // Извлечение поля из NMEA строки
    String getField(const String& nmea, int fieldIndex);
    
  public:
    SimpleGPS();
    
    // Основной метод парсинга
    bool parse(const String& nmeaData);
    
    // Проверка наличия валидного сигнала
    bool hasValidFix();
    
    // Геттеры
    double getLatitude();
    double getLongitude();
    float getAltitude();
    float getSpeed();      // км/ч
    float getSpeedKnots(); // узлы
    int getSatellites();
    float getHdop();
    String getTime();
    String getDate();
    
    // Форматированный вывод
    String getFormattedData();
    String getCSVData();
    
    // Сброс данных
    void reset();
};

#endif