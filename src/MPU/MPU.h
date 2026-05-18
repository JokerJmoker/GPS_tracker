// =====================================================
// FILE: src/MPU/MPU.h
// =====================================================

#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <Wire.h>

class MPU {
  private:
    bool _enabled;           // Флаг включения/выключения датчика
    bool _initialized;       // Флаг успешной инициализации

    int16_t _ax, _ay, _az;   // Сырые значения акселерометра по осям X, Y, Z
    unsigned long _lastRead; // Время последнего чтения данных (в миллисекундах)

    void writeRegister(uint8_t reg, uint8_t data); // Запись байта в регистр MPU
    void readRawData();      // Чтение сырых данных акселерометра через I2C

  public:
    MPU();                   // Конструктор - инициализация переменных

    void begin();            // Запуск I2C и пробуждение MPU6050
    
    void enable();           // Включение датчика (вызов begin)
    void disable();          // Выключение датчика (сброс флагов)
    bool isEnabled();        // Возвращает состояние флага _enabled
    void setEnabled(bool enabled); // Установка состояния включения/выключения

    void update();           // Вызов чтения данных (не чаще 1 раза в 500 мс)
    bool available();        // Проверка, инициализирован ли датчик

    String getData();        // Возвращает строку с данными акселерометра
};

#endif
