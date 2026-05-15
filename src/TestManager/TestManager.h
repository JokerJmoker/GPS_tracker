#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
#include "NEO/GPSParser.h"
#include "Config.h"

class TestManager {
  private:

    // =========================
    // MODULES
    // =========================
    NEO* _gps;
    SIM* _sim800l;
    MPU* _mpu6050;

    // =========================
    // TIMERS
    // =========================
    unsigned long _lastStatus;
    unsigned long _lastPrint;
    unsigned long _lastVoltageCheck;
    unsigned long _lastMpuPrint;

    // =========================
    // SIM TEST STATE MACHINE
    // =========================
    int _cmdIndex;
    const char* _commands[10];

    // =========================
    // PIPELINE (GPS → SIM)
    // =========================
    char _gpsUrl[160];      // готовая ссылка
    bool _gpsHasFix;        // есть валидные координаты
    bool _pipelineBusy;     // GPS → SIM lock

    // =========================
    // PRIVATE METHODS
    // =========================
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