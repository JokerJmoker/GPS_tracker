#ifndef TRACKER_CONTROLLER_H
#define TRACKER_CONTROLLER_H

#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
#include "SystemModes.h"

class TrackerController {
  private:
    NEO* _gps;
    SIM* _sim;
    MPU* _mpu;
    
    // Отслеживание состояния
    bool _gpsDataReady;
    bool _simCommandsDone;
    unsigned long _lastCommandTime;
    int _commandIndex;
    
    // Для режима сна
    bool _mpuMovementDetected;
    
    void sendNextSimCommand();
    void resetSimCommandSequence();
    
  public:
    TrackerController(NEO* gps, SIM* sim, MPU* mpu);
    
    void begin();
    void update();
    void resetCycle();  
    
    // Проверка состояния
    bool isGPSDataReady();
    bool isSIMCommandsDone();
    bool isMPUMovementDetected();
    
    // Принудительное управление
    void forceGPSRead();
    void forceSIMCommands();
};

#endif