#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

// Режимы работы трекера
enum TrackerMode {
  MODE_DEBUG = 0,      // Режим отладки - всё работает постоянно
  MODE_TRACKING = 1,   // Режим трекинга - GPS и SIM по очереди
  MODE_SLEEP = 2       // Режим сна - пробуждение по MPU
};

// Типы нажатий
enum ButtonPress {
  PRESS_NONE = 0,
  PRESS_SINGLE = 1,
  PRESS_DOUBLE = 2,
  PRESS_TRIPLE = 3,
  PRESS_LONG = 4
};

class ButtonManager {
  private:
    uint8_t _pin;
    TrackerMode _currentMode;
    
    // Для обработки нажатий
    unsigned long _lastPressTime;
    unsigned long _lastReleaseTime;
    unsigned long _longPressStart;
    int _pressCount;
    bool _isPressing;
    bool _longPressHandled;
    
    // Для дебаунса
    unsigned long _debounceDelay;
    
    ButtonPress detectPress();
    
  public:
    ButtonManager(uint8_t pin);
    
    void begin();
    void update();
    TrackerMode getMode();
    bool isModeChanged();
    
    // Визуальная индикация
    void indicateMode(TrackerMode mode);
};

#endif