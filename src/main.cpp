// =====================================================
// FILE: src/main.cpp
// =====================================================

#include <Arduino.h>

#include "Config.h"

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

// =====================================================
// TRACKER CONTROLLER
// =====================================================

#include "ModeManager/TrackerController.h"

// =====================================================
// CREATE MODULE OBJECTS
// =====================================================

// -------------------------------------
// GPS (RX = 3, TX = 4)
// -------------------------------------

GPS gps(3, 4);

// -------------------------------------
// GSM (RX = 5, TX = 6)
// -------------------------------------

GSM gsm(5, 6);

// -------------------------------------
// MPU6050
// -------------------------------------

MPU mpu;

// =====================================================
// CREATE FSM (POINTERS FOR DYNAMIC ALLOCATION)
// =====================================================

GPS_FSM* gpsFSM = nullptr;
GSM_FSM* gsmFSM = nullptr;
MPU_FSM* mpuFSM = nullptr;

// =====================================================
// TRACKER CONTROLLER (POINTER)
// =====================================================

#if defined(MODE_TRACKER) || defined(MODE_SLEEP)
    TrackerController* trackerController = nullptr;
#endif

// =====================================================
// SETUP
// =====================================================

void setup()
{
    // =====================================
    // LED INDICATOR (ALWAYS WORKS)
    // =====================================
    
    pinMode(LED_BUILTIN, OUTPUT);
    
    // 5 вспышек = начало загрузки
    for(int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
    
    // =====================================
    // SERIAL
    // =====================================

    Serial.begin(9600);
    
    // Таймаут для Serial (чтобы не зависать)
    unsigned long start = millis();
    while (!Serial && millis() - start < 2000);
    
    Serial.println();
    Serial.println(F("===================================="));
    Serial.println(F("SYSTEM START"));
    Serial.println(F("===================================="));
    
    // =====================================
    // SYSTEM MODES INIT
    // =====================================

    SystemModes::begin();
    
    // =====================================
    // DYNAMIC ALLOCATION BASED ON MODE
    // =====================================

#ifdef MODE_DEBUG
    
    Serial.println(F("[SYSTEM] MODE_DEBUG"));
    
    // Allocate FSM objects for DEBUG mode
    gpsFSM = new GPS_FSM(&gps);
    gsmFSM = new GSM_FSM(&gsm);
    mpuFSM = new MPU_FSM(&mpu);
    
    // =====================================
    // GPS TEST
    // =====================================
    
    #ifdef TEST_GPS
    
        Serial.println(F("[DEBUG] TEST_GPS"));
        
        gpsFSM->begin();
        gpsFSM->enable();
        
        #if GPS_MOCK_MODE == 2
            gpsFSM->setState(GPSState::MOCK_PARSE);
        #elif GPS_MOCK_MODE == 1
            gpsFSM->setState(GPSState::REAL_FIX);
        #else
            gpsFSM->setState(GPSState::CONTINUOUS);
        #endif
    
    #endif
    
    // =====================================
    // MPU6050 TEST
    // =====================================
    
    #ifdef TEST_MPU6050
    
        Serial.println(F("[DEBUG] TEST_MPU6050"));
        mpuFSM->begin();
    
    #endif
    
    Serial.println();
    Serial.println(F("[SYSTEM] READY"));
    Serial.println();

#endif // MODE_DEBUG

    // =====================================
    // MODE: TRACKER
    // =====================================

#ifdef MODE_TRACKER
    
    Serial.println(F("[SYSTEM] MODE_TRACKER"));
    
    // Allocate FSM objects for TRACKER mode
    gpsFSM = new GPS_FSM(&gps);
    gsmFSM = new GSM_FSM(&gsm);
    mpuFSM = new MPU_FSM(&mpu);
    
    // Create and initialize Tracker Controller
    trackerController = new TrackerController(gpsFSM, gsmFSM, mpuFSM);
    trackerController->begin();
    
    Serial.println();
    Serial.println(F("[SYSTEM] READY_TRACKER"));
    Serial.println();

#endif // MODE_TRACKER

    // =====================================
    // MODE: SLEEP
    // =====================================

#ifdef MODE_SLEEP
    Serial.println(F("[SYSTEM] MODE_SLEEP"));
    
    // СОЗДАЕМ ОБЪЕКТЫ
    gpsFSM = new GPS_FSM(&gps);
    if (gpsFSM == nullptr) {
        Serial.println(F("[ERROR] Failed to create gpsFSM"));
        while(1);
    }
    
    gsmFSM = new GSM_FSM(&gsm);
    mpuFSM = new MPU_FSM(&mpu);
    
    trackerController = new TrackerController(gpsFSM, gsmFSM, mpuFSM);
    if (trackerController == nullptr) {
        Serial.println(F("[ERROR] Failed to create trackerController"));
        while(1);
    }
    
    trackerController->begin();
    
    Serial.println(F("[SYSTEM] READY_SLEEP"));
    Serial.flush();
#endif


    // Final ready blink
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
#ifdef MODE_DEBUG
    
    if (gpsFSM == nullptr) {
        delay(10);
        return;
    }
    
    // =====================================
    // GPS TEST
    // =====================================
    
    #ifdef TEST_GPS
    
        gpsFSM->update();
        
        if (gpsFSM->isURLReady())
        {
            Serial.println();
            Serial.println(F("===================================="));
            Serial.println(F("[MAIN] GPS URL READY"));
            Serial.println(F("===================================="));
            
            Serial.print(F("[URL] "));
            Serial.println(gpsFSM->getURL());
            
            Serial.println(F("===================================="));
            Serial.println();
            
            gpsFSM->reset();
        }
    
    #endif
    
    // =====================================
    // MPU6050 TEST
    // =====================================
    
    #ifdef TEST_MPU6050
    
        if (mpuFSM != nullptr) {
            mpuFSM->update();
            
            if (mpuFSM->isAwaken())
            {
                Serial.println();
                Serial.println(F("===================================="));
                Serial.println(F("[SYSTEM] MOVEMENT DETECTED"));
                Serial.println(F("===================================="));
                Serial.println();
                
                // emulate external handling
                mpuFSM->setState(MPUState::DISABLED);
            }
        }
    
    #endif
    
    delay(10);

#endif // MODE_DEBUG

    // =====================================
    // MODE_TRACKER
    // =====================================

#ifdef MODE_TRACKER
    
    if (trackerController != nullptr) {
        trackerController->update();
    }
    
    delay(50);

#endif // MODE_TRACKER

    // =====================================
    // MODE_SLEEP
    // =====================================

#ifdef MODE_SLEEP
    
    if (trackerController != nullptr) {
        trackerController->update();
    }
    
    delay(50);
    
    // Quick blink every 5 seconds to show it's alive (optional)
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 5000) {
        lastBlink = millis();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(50);
        digitalWrite(LED_BUILTIN, LOW);
    }

#endif // MODE_SLEEP
}