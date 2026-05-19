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
// TRACKER CONTROLLER (NEW)
// =====================================================

#include "ModeManager/TrackerController.h"

// =====================================================
// CREATE MODULE OBJECTS
// =====================================================

// -------------------------------------
// GPS
// RX = 3
// TX = 4
// -------------------------------------

GPS gps(3, 4);

// -------------------------------------
// GSM
// RX = 5
// TX = 6
// -------------------------------------

GSM gsm(5, 6);

// -------------------------------------
// MPU6050
// -------------------------------------

MPU mpu;

// =====================================================
// CREATE FSM
// =====================================================

GPS_FSM gpsFSM(&gps);
GSM_FSM gsmFSM(&gsm);
MPU_FSM mpuFSM(&mpu);

// =====================================================
// CREATE TRACKER CONTROLLER (for MODE_TRACKER)
// =====================================================

#if defined(MODE_TRACKER) || defined(MODE_SLEEP)

    TrackerController trackerController(
        &gpsFSM,
        &gsmFSM,
        &mpuFSM
    );

#endif

// =====================================================
// SETUP
// =====================================================

void setup()
{
    // =====================================
    // SERIAL
    // =====================================

    Serial.begin(9600);

    while (!Serial);

    Serial.println();
    Serial.println(F("===================================="));
    Serial.println(F("SYSTEM START"));
    Serial.println(F("===================================="));

    // =====================================
    // SYSTEM MODES INIT
    // =====================================

    SystemModes::begin();

    // =====================================
    // MODE: DEBUG
    // =====================================

#ifdef MODE_DEBUG

    Serial.println(F("[SYSTEM] MODE_DEBUG"));

    // =====================================
    // GPS TEST
    // =====================================

    #ifdef TEST_GPS

        Serial.println(F("[DEBUG] TEST_GPS"));

        gpsFSM.begin();
        gpsFSM.enable();

        #if GPS_MOCK_MODE == 2
            gpsFSM.setState(GPSState::MOCK_PARSE);
        #elif GPS_MOCK_MODE == 1
            gpsFSM.setState(GPSState::REAL_FIX);
        #else
            gpsFSM.setState(GPSState::CONTINUOUS);
        #endif

    #endif

    // =====================================
    // MPU6050 TEST
    // =====================================

    #ifdef TEST_MPU6050

        Serial.println(F("[DEBUG] TEST_MPU6050"));

        mpuFSM.begin();

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

    // Initialize GPS FSM
    //gpsFSM.begin();
    
    // Initialize Tracker Controller (not GSM FSM directly!)
    trackerController.begin();

    Serial.println();
    Serial.println(F("[SYSTEM] READY"));
    Serial.println();

#endif // MODE_TRACKER

    // =====================================
    // MODE: SLEEP
    // =====================================

#ifdef MODE_SLEEP

    Serial.println(F("[SYSTEM] MODE_SLEEP"));

    trackerController.begin();

    Serial.println();
    Serial.println(F("[SYSTEM] READY_SLEEP"));
    Serial.println();

#endif

}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    // =====================================
    // MODE_DEBUG
    // =====================================

// =====================================
// MODE_DEBUG
// =====================================

#ifdef MODE_DEBUG

    // =====================================
    // GPS TEST
    // =====================================

    #ifdef TEST_GPS

        gpsFSM.update();

        if (gpsFSM.isURLReady())
        {
            Serial.println();
            Serial.println(F("===================================="));
            Serial.println(F("[MAIN] GPS URL READY"));
            Serial.println(F("===================================="));

            Serial.print(F("[URL] "));
            Serial.println(gpsFSM.getURL());

            Serial.println(F("===================================="));
            Serial.println();

            gpsFSM.reset();
        }

    #endif

    // =====================================
    // MPU6050 TEST
    // =====================================

    #ifdef TEST_MPU6050

        mpuFSM.update();

        if (mpuFSM.isAwaken())
        {
            Serial.println();
            Serial.println(F("===================================="));
            Serial.println(F("[SYSTEM] MOVEMENT DETECTED"));
            Serial.println(F("===================================="));
            Serial.println();

            // emulate external handling
            mpuFSM.setState(MPUState::DISABLED);
        }

    #endif

    delay(10);

#endif // MODE_DEBUGG

    // =====================================
    // MODE_TRACKER
    // =====================================

#ifdef MODE_TRACKER

    // Just call the tracker controller - it handles everything!
    trackerController.update();
    
    delay(50);

#endif // MODE_TRACKER

    // =====================================
    // MODE_SLEEP
    // =====================================

#ifdef MODE_SLEEP

    trackerController.update();

    delay(50);

#endif
}