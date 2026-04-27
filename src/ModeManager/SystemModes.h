#ifndef SYSTEM_MODES_H
#define SYSTEM_MODES_H

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>

enum class OperationMode {
  DEBUG_MODE,      // Режим отладки - всё работает постоянно
  TRACKER_MODE,    // Режим трекера - циклическая работа GPS->SIM
  SLEEP_MODE       // Режим сна - пробуждение от MPU6050
};

class SystemModes {
  private:
    static OperationMode _currentMode;
    static bool _modeSwitchRequested;
    static unsigned long _gpsStartTime;
    static unsigned long _simStartTime;
    static bool _gpsCompleted;
    static bool _simCompleted;
    static bool _oneCycleComplete;
    static bool _gpsDataReceived;
    static bool _simCommandsCompleted;  // <- переименовано с _simCommandsDone
    static bool _simCommandsDone;       // <- ДОБАВИТЬ эту строку!
    
  public:
    // Установка режима из кода
    static void setMode(OperationMode mode);
    static OperationMode getCurrentMode();
    
    // Управление режимом трекера
    static void resetTrackerCycle();
    static void updateTrackerState(bool gpsHasData, bool simCommandsDone);
    static bool shouldGPSBeActive();
    static bool shouldSIMBeActive();
    static bool isTrackerCycleComplete();
    
    // Управление сном
    static void goToSleep();
    static void wakeUp();
    
    // Визуальная индикация
    static void indicateModeChange();
    
    // Инициализация
    static void begin();
    
    // Добавлен метод forceReset
    static void forceReset();
};

#endif