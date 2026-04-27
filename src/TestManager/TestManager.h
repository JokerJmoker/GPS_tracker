#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"

// ============================================
// НАСТРОЙКА ТЕСТИРУЕМЫХ МОДУЛЕЙ
// ============================================
#define TEST_GPS
//#define TEST_SIM800L
//#define TEST_MPU6050
//#define TEST_EEPROM
// ============================================

class TestManager {
  private:
    // Указатели на модули
    NEO* _gps;
    SIM* _sim800l;
    MPU* _mpu6050;
    
    // Временные метки
    unsigned long _lastStatus;
    unsigned long _lastPrint;
    unsigned long _lastVoltageCheck;
    unsigned long _lastMpuPrint;
    
    // Для SIM команд
    int _cmdIndex;
    const char* _commands[10];
    
    // Приватные методы
    void initCommands();
    void printHeader();
    void initModules();
    void updateModules();
    void processGPS();
    void processSIM();
    void processMPU();
    
  public:
    TestManager(NEO* gps, SIM* sim800l, MPU* mpu6050);
    
    void begin();
    void update();
};

#endif