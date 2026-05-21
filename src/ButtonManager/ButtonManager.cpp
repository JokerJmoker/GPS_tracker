// =====================================================
// FILE: src/ButtonManager/ButtonManager.cpp
// =====================================================

#include "ButtonManager.h"
#include "ModeManager/SystemModes.h"

#if ENABLE_BUTTON == 1

ButtonManager::ButtonManager(uint8_t pin) {
    _pin = pin;
    _lastStableState = HIGH;
    _lastReading = HIGH;
    _lastDebounceTime = 0;
    _pressStartTime = 0;
    _waitingForDoubleClick = false;
    _lastClickTime = 0;
    _pendingEvent = ButtonEvent::NONE;
    _modeChangeRequested = false;
    _buttonPressed = false;
}

void ButtonManager::begin() {
    pinMode(_pin, INPUT_PULLUP);
    Serial.println(F("[BUTTON] Manager initialized"));
    Serial.print(F("[BUTTON] Pin: "));
    Serial.println(_pin);
    
    detectStartupMode();
    printMode();
}

void ButtonManager::detectStartupMode() {
    Serial.println(F("[BUTTON] Checking startup button state..."));
    delay(100);
    
    bool buttonPressedAtStart = (digitalRead(_pin) == LOW);
    
    if (buttonPressedAtStart) {
        Serial.println(F("[BUTTON] Button pressed at startup - waiting for release"));
        unsigned long startWait = millis();
        while (digitalRead(_pin) == LOW && (millis() - startWait) < 3000) {
            delay(10);
        }
    } else {
        Serial.println(F("[BUTTON] No button press at startup - using DEFAULT_MODE"));
    }
}

void ButtonManager::updateButtonState() {
    bool currentReading = digitalRead(_pin);
    
    if (currentReading != _lastReading) {
        _lastDebounceTime = millis();
        _lastReading = currentReading;
    }
    
    if ((millis() - _lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        _lastStableState = currentReading;
    }
    
    _buttonPressed = (_lastStableState == LOW);
}

ButtonEvent ButtonManager::detectEvent() {
    if (_lastStableState == LOW && _lastReading == HIGH) {
        _pressStartTime = 0;
        
        unsigned long pressDuration = millis() - _lastDebounceTime;
        
        if (pressDuration >= LONG_PRESS_MS) {
            resetButtonState();
            return ButtonEvent::LONG_PRESS;
        } else {
            if (_waitingForDoubleClick) {
                unsigned long timeBetweenClicks = millis() - _lastClickTime;
                if (timeBetweenClicks <= DOUBLE_CLICK_MAX_MS) {
                    _waitingForDoubleClick = false;
                    _lastClickTime = 0;
                    resetButtonState();
                    return ButtonEvent::DOUBLE_CLICK;
                }
            }
            
            if (!_waitingForDoubleClick) {
                _waitingForDoubleClick = true;
                _lastClickTime = millis();
                _pendingEvent = ButtonEvent::SINGLE_CLICK;
                return ButtonEvent::NONE;
            }
        }
    }
    
    if (_waitingForDoubleClick && (millis() - _lastClickTime) > DOUBLE_CLICK_MAX_MS) {
        _waitingForDoubleClick = false;
        if (_pendingEvent == ButtonEvent::SINGLE_CLICK) {
            _pendingEvent = ButtonEvent::NONE;
            return ButtonEvent::SINGLE_CLICK;
        }
    }
    
    return ButtonEvent::NONE;
}

void ButtonManager::resetButtonState() {
    _waitingForDoubleClick = false;
    _pendingEvent = ButtonEvent::NONE;
    _lastClickTime = 0;
    _pressStartTime = 0;
}

void ButtonManager::update() {
    updateButtonState();
    
    if (_lastStableState == LOW && _pressStartTime == 0) {
        _pressStartTime = millis();
    }
    
    if (_pressStartTime > 0 && (millis() - _pressStartTime) >= LONG_PRESS_MS) {
        Serial.println(F("[BUTTON] Long press detected - DEBUG MODE"));
        SystemModes::setMode(OperationMode::DEBUG_MODE);
        _modeChangeRequested = true;
        resetButtonState();
        return;
    }
    
    ButtonEvent event = detectEvent();
    
    switch (event) {
        case ButtonEvent::SINGLE_CLICK:
            Serial.println(F("[BUTTON] Single click detected - TRACKER MODE"));
            SystemModes::setMode(OperationMode::TRACKER_MODE);
            _modeChangeRequested = true;
            break;
            
        case ButtonEvent::DOUBLE_CLICK:
            Serial.println(F("[BUTTON] Double click detected - SLEEP MODE"));
            SystemModes::setMode(OperationMode::SLEEP_MODE);
            _modeChangeRequested = true;
            break;
            
        default:
            break;
    }
}

bool ButtonManager::isModeChangeRequested() {
    return _modeChangeRequested;
}

void ButtonManager::confirmModeChange() {
    _modeChangeRequested = false;
}

bool ButtonManager::isPressed() {
    return _buttonPressed;
}

void ButtonManager::printMode() {
    Serial.print(F("[BUTTON] Current mode: "));
    switch (SystemModes::getCurrentMode()) {
        case OperationMode::DEBUG_MODE:
            Serial.println(F("DEBUG"));
            break;
        case OperationMode::TRACKER_MODE:
            Serial.println(F("TRACKER"));
            break;
        case OperationMode::SLEEP_MODE:
            Serial.println(F("SLEEP"));
            break;
    }
}

#endif // ENABLE_BUTTON