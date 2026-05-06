#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"

class PowerManager {
  private:
    NEO* _gps;
    SIM* _sim;
    MPU* _mpu;
    
    // Состояния цикла в режиме TRACKING
    enum TrackingState {
      STATE_GPS_ACTIVE = 0,
      STATE_SIM_ACTIVE = 1
    };
    
    TrackingState _trackingState;
    bool _gpsDataReady;      // Флаг: GPS получил данные
    bool _simWorkDone;       // Флаг: SIM выполнил все команды
    bool _trackingCycleActive;
    int _simCommandIndex;
    
    // Для сна и пробуждения
    volatile bool _wakeByMPU;
    bool _mpuDetectedMotion;
    bool _oneCycleComplete;   // Флаг: один цикл завершен (для SLEEP режима)
    
  public:
    PowerManager(NEO* gps, SIM* sim, MPU* mpu);
    
    void begin();
    void update();
    void resetTrackingCycle();
    void processTrackingMode();
    void processSleepMode();
    
    // Обработка MPU прерывания
    static void wakeUpISR();
    bool isWakeByMPU();
    void clearWakeFlag();
    
    void setGPSDataReady(bool ready);
    void setSIMWorkDone(bool done);
    bool isOneCycleComplete();
};

#endif