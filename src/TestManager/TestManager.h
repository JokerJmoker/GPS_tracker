#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"

// ============================================
// НАСТРОЙКА ТЕСТИРУЕМЫХ МОДУЛЕЙ
// ============================================
#define TEST_GPS          // Раскомментировать для тестирования GPS модуля
//#define TEST_SIM800L        // Раскомментировать для тестирования SIM800L модуля
//#define TEST_MPU6050       // Раскомментировать для тестирования MPU6050 датчика
//#define TEST_EEPROM        // Раскомментировать для тестирования EEPROM (в разработке)
// ============================================

class TestManager {
  private:
    // ===== Указатели на модули =====
    NEO* _gps;              // Указатель на объект GPS модуля
    SIM* _sim800l;          // Указатель на объект SIM800L модуля
    MPU* _mpu6050;          // Указатель на объект MPU6050 датчика
    
    // ===== Временные метки (для неблокирующих задержек) =====
    unsigned long _lastStatus;        // Время последнего вывода статуса GPS
    unsigned long _lastPrint;         // Время последней отправки AT команды SIM
    unsigned long _lastVoltageCheck;  // Время последней проверки напряжения SIM
    unsigned long _lastMpuPrint;      // Время последнего вывода данных MPU
    
    // ===== Для SIM команд =====
    int _cmdIndex;                    // Текущий индекс в массиве команд (0-9)
    const char* _commands[10];        // Массив из 10 AT команд для тестирования SIM
    
    // ===== Приватные методы =====
    void initCommands();      // Заполнение массива AT команд
    void printHeader();       // Вывод красивой шапки в Serial
    void initModules();       // Инициализация/деинициализация модулей согласно #define
    void updateModules();     // Вызов update() для всех активных модулей
    void processGPS();        // Логика чтения и вывода данных GPS
    void processSIM();        // Логика отправки AT команд и вывода ответов SIM
    void processMPU();        // Логика чтения и вывода данных MPU6050
    
  public:
    // Конструктор - принимает указатели на уже созданные объекты модулей
    TestManager(NEO* gps, SIM* sim800l, MPU* mpu6050);
    
    // Инициализация - выводит заголовок и запускает выбранные модули
    void begin();
    
    // Основной цикл - вызывать в loop() для обработки всех модулей
    void update();
};

#endif