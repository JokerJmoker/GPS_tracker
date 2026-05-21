// =====================================================
// FILE: src/main.cpp
// =====================================================

#include <Arduino.h>
#include "Config/Config.h"

// =====================================================
// MODULES
// =====================================================

#include "GPS/GPS.h"
#include "GSM/GSM.h"
#include "MPU/MPU.h"

// =====================================================
// FSM
// =====================================================

#include "GPS/GPS_FSM.h"
#include "GSM/GSM_FSM.h"
#include "MPU/MPU_FSM.h"

// =====================================================
// MODE MANAGER
// =====================================================

#include "ModeManager/SystemModes.h"
#include "ModeManager/TrackerController.h"

// =====================================================
// BUTTON MANAGER
// =====================================================

#if ENABLE_BUTTON == 1
    #include "ButtonManager/ButtonManager.h"
#endif

// =====================================================
// CREATE MODULE OBJECTS
// =====================================================

GPS gps(3, 4);
GSM gsm(5, 6);
MPU mpu;

// =====================================================
// CREATE FSM (POINTERS)
// =====================================================

GPS_FSM* gpsFSM = nullptr;
GSM_FSM* gsmFSM = nullptr;
MPU_FSM* mpuFSM = nullptr;

// =====================================================
// TRACKER CONTROLLER (POINTER)
// =====================================================

TrackerController* trackerController = nullptr;

// =====================================================
// BUTTON MANAGER OBJECT
// =====================================================

#if ENABLE_BUTTON == 1
    ButtonManager buttonManager(BUTTON_PIN);
#endif

// =====================================================
// FUNCTION TO INIT SYSTEM FOR CURRENT MODE
// =====================================================

void initSystemForMode(OperationMode mode) {
    Serial.println(F("[MAIN] Initializing system for current mode..."));
    
    // Очищаем предыдущие объекты
    if (gpsFSM != nullptr) {
        gpsFSM->disable();
        delete gpsFSM;
        gpsFSM = nullptr;
    }
    
    if (gsmFSM != nullptr) {
        gsmFSM->disable();
        delete gsmFSM;
        gsmFSM = nullptr;
    }
    
    if (mpuFSM != nullptr) {
        delete mpuFSM;
        mpuFSM = nullptr;
    }
    
    if (trackerController != nullptr) {
        delete trackerController;
        trackerController = nullptr;
    }
    
    // Создаём базовые объекты
    gpsFSM = new GPS_FSM(&gps);
    gsmFSM = new GSM_FSM(&gsm);
    mpuFSM = new MPU_FSM(&mpu);
    
    // Инициализируем в зависимости от режима
    switch (mode) {
        case OperationMode::DEBUG_MODE:
            Serial.println(F("[MAIN] DEBUG mode active"));
            #ifdef TEST_GPS
                gpsFSM->begin();
                gpsFSM->enable();
                #if GPS_MOCK_MODE == 2
                    gpsFSM->setState(GPSState::MOCK_PARSE);
                #else
                    gpsFSM->setState(GPSState::REAL_FIX);
                #endif
            #endif
            #ifdef TEST_MPU6050
                mpuFSM->begin();
            #endif
            break;
            
        case OperationMode::TRACKER_MODE:
            Serial.println(F("[MAIN] TRACKER mode active"));
            trackerController = new TrackerController(gpsFSM, gsmFSM, mpuFSM);
            trackerController->begin();
            break;
            
        case OperationMode::SLEEP_MODE:
            Serial.println(F("[MAIN] SLEEP mode active"));
            trackerController = new TrackerController(gpsFSM, gsmFSM, mpuFSM);
            trackerController->begin();
            break;
    }
}

// =====================================================
// SETUP
// =====================================================

void setup() {
    // LED индикация старта
    pinMode(LED_BUILTIN, OUTPUT);
    for(int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
    
    // Serial инициализация
    Serial.begin(9600);
    unsigned long start = millis();
    while (!Serial && millis() - start < 2000);
    
    Serial.println();
    Serial.println(F("===================================="));
    Serial.println(F("SYSTEM START"));
    Serial.println(F("===================================="));
    
    // Инициализация SystemModes
    SystemModes::begin();
    
    // Инициализация кнопки
    #if ENABLE_BUTTON == 1
        buttonManager.begin();
        Serial.println(F("[MAIN] Button manager enabled"));
        Serial.println(F("  - Single click: TRACKER mode"));
        Serial.println(F("  - Double click: SLEEP mode"));
        Serial.println(F("  - Long press (2s): DEBUG mode"));
    #endif
    
    // Инициализация системы под текущий режим
    initSystemForMode(SystemModes::getCurrentMode());
    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    // Обработка кнопки
    #if ENABLE_BUTTON == 1
        buttonManager.update();
        
        if (buttonManager.isModeChangeRequested()) {
            Serial.println(F("\n[MAIN] Mode change detected, reinitializing..."));
            initSystemForMode(SystemModes::getCurrentMode());
            buttonManager.confirmModeChange();
        }
        
        // Индикация нажатия
        static bool lastPressedState = false;
        if (buttonManager.isPressed() && !lastPressedState) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else if (!buttonManager.isPressed() && lastPressedState) {
            digitalWrite(LED_BUILTIN, LOW);
        }
        lastPressedState = buttonManager.isPressed();
    #endif
    
    // Основная логика по текущему режиму
    OperationMode currentMode = SystemModes::getCurrentMode();
    
    switch (currentMode) {
        case OperationMode::DEBUG_MODE:
            #ifdef TEST_GPS
                if (gpsFSM != nullptr) {
                    gpsFSM->update();
                    if (gpsFSM->isURLReady()) {
                        Serial.println();
                        Serial.println(F("===================================="));
                        Serial.println(F("[MAIN] GPS URL READY"));
                        Serial.print(F("[URL] "));
                        Serial.println(gpsFSM->getURL());
                        Serial.println(F("===================================="));
                        gpsFSM->reset();
                    }
                }
            #endif
            #ifdef TEST_MPU6050
                if (mpuFSM != nullptr) {
                    mpuFSM->update();
                    if (mpuFSM->isAwaken()) {
                        Serial.println(F("\n[SYSTEM] MOVEMENT DETECTED"));
                        mpuFSM->setState(MPUState::DISABLED);
                    }
                }
            #endif
            delay(10);
            break;
            
        case OperationMode::TRACKER_MODE:
            if (trackerController != nullptr) {
                trackerController->update();
            }
            delay(50);
            break;
            
        case OperationMode::SLEEP_MODE:
            if (trackerController != nullptr) {
                trackerController->update();
            }
            delay(50);
            
            static unsigned long lastBlink = 0;
            if (millis() - lastBlink > 5000) {
                lastBlink = millis();
                digitalWrite(LED_BUILTIN, HIGH);
                delay(50);
                digitalWrite(LED_BUILTIN, LOW);
            }
            break;
    }
}