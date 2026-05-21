// =====================================================
// FILE: src/ButtonManager/ButtonManager.h
// =====================================================

#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "Config/Config.h"

#if ENABLE_BUTTON == 1

enum class ButtonEvent {
    NONE,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    LONG_PRESS
};

class ButtonManager {
private:
    uint8_t _pin;
    bool _lastStableState;
    bool _lastReading;
    unsigned long _lastDebounceTime;
    unsigned long _pressStartTime;
    
    bool _waitingForDoubleClick;
    unsigned long _lastClickTime;
    ButtonEvent _pendingEvent;
    
    bool _modeChangeRequested;
    bool _buttonPressed;
    
    void updateButtonState();
    ButtonEvent detectEvent();
    void resetButtonState();
    void detectStartupMode();
    
public:
    ButtonManager(uint8_t pin = BUTTON_PIN);
    void begin();
    void update();
    
    bool isModeChangeRequested();
    void confirmModeChange();
    bool isPressed();
    void printMode();
};

#endif // ENABLE_BUTTON

#endif // BUTTON_MANAGER_H