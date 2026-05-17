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

// =====================================================
// MODE MANAGER
// =====================================================

#include "ModeManager/SystemModes.h"

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

GSM gsmModule(5, 6);

// -------------------------------------
// MPU6050
// -------------------------------------

MPU mpu6050;

// =====================================================
// CREATE FSM
// =====================================================

GPS_FSM gpsFSM(&gps);

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

    SystemModes::setMode(OperationMode::DEBUG_MODE);

    Serial.println(F("[SYSTEM] MODE_DEBUG"));

    gpsFSM.begin();

    gpsFSM.enable();

    // =====================================
    // GPS FSM MODE SELECTION
    // =====================================

    #if GPS_MOCK_MODE == 2

        gpsFSM.setState(GPSState::MOCK_PARSE);

    #elif GPS_MOCK_MODE == 1

        gpsFSM.setState(GPSState::REAL_FIX);

    #else

        gpsFSM.setState(GPSState::CONTINUOUS);

    #endif

#endif

    // =====================================
    // MODE: TRACKER
    // =====================================

#ifdef MODE_TRACKER

    SystemModes::setMode(OperationMode::TRACKER_MODE);

    Serial.println(F("[SYSTEM] MODE_TRACKER"));

    gpsFSM.begin();
    gpsFSM.enable();

    // =====================================
    // GPS FSM MODE SELECTION (TRACKER)
    // =====================================

    #if GPS_MOCK_MODE == 2

        // MOCK PARSE MODE
        Serial.println(F("[TRACKER] GPS MOCK PARSE MODE"));
        gpsFSM.setState(GPSState::MOCK_PARSE);

    #else

        // DEFAULT SAFETY FALLBACK
        Serial.println(F("[TRACKER] GPS REAL FIX MODE"));
        gpsFSM.setState(GPSState::REAL_FIX);

    #endif

    // =====================================
    // OTHER MODULES
    // =====================================

    gsmModule.disable();
    mpu6050.disable();

#endif

    // =====================================
    // MODE: SLEEP
    // =====================================

#ifdef MODE_SLEEP

    SystemModes::setMode(OperationMode::SLEEP_MODE);

    Serial.println(F("[SYSTEM] MODE_SLEEP"));

    gpsFSM.disable();

    gsmModule.disable();

    mpu6050.begin();

#endif

    // =====================================
    // READY
    // =====================================

    Serial.println();
    Serial.println(F("[SYSTEM] READY"));
    Serial.println();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    // =====================================
    // MODE_DEBUG
    // =====================================

#ifdef MODE_DEBUG

    gpsFSM.update();

    // =====================================
    // URL READY
    // =====================================

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

        // после mock режима не повторяем цикл
        gpsFSM.reset();
    }

    delay(10);

#endif

    // =====================================
    // MODE_TRACKER
    // =====================================

#ifdef MODE_TRACKER

    // =====================================
    // GPS ACTIVE
    // =====================================

    if (SystemModes::shouldGPSBeActive())
    {
        if (!gpsFSM.isEnabled())
        {
            gpsFSM.enable();

            gsmModule.disable();

            Serial.println(F("[TRACKER] GPS ACTIVE"));
        }

        gpsFSM.update();
    }

    // =====================================
    // FIX READY
    // =====================================

    if (gpsFSM.hasFix())
    {
        Serial.println();
        Serial.println(F("[TRACKER] GPS FIX RECEIVED"));

        Serial.print(F("[TRACKER] URL: "));
        Serial.println(gpsFSM.getURL());

        // =================================
        // DISABLE GPS
        // =================================

        gpsFSM.disable();

        // =================================
        // ENABLE GSM
        // =================================

        gsmModule.begin(9600);

        Serial.println(F("[TRACKER] GSM ACTIVE"));

        // =================================
        // MOCK GSM SEND
        // =================================

        Serial.println(F("[GSM] AT"));
        Serial.println(F("OK"));

        Serial.println(F("[GSM] AT+CMGF=1"));
        Serial.println(F("OK"));

        Serial.println(F("[GSM] SMS SENT"));

        Serial.println(gpsFSM.getURL());

        // =================================
        // RESET FSM
        // =================================

        gpsFSM.reset();

        // =================================
        // BACK TO GPS
        // =================================

        gsmModule.disable();

        gpsFSM.enable();

        Serial.println(F("[TRACKER] RETURN TO GPS"));
    }

    delay(50);

#endif

    // =====================================
    // MODE_SLEEP
    // =====================================

#ifdef MODE_SLEEP

    // =====================================
    // MPU ACTIVE
    // =====================================

    mpu6050.update();

    // =====================================
    // MOVEMENT DETECTED
    // =====================================

    if (mpu6050.available())
    {
        Serial.println();
        Serial.println(F("[SLEEP] WAKE UP"));
        Serial.println();

        // =================================
        // ENABLE GPS
        // =================================

        gpsFSM.enable();

        gpsFSM.setState(GPSState::REAL_FIX);

        // =================================
        // WAIT FIX
        // =================================

        while (!gpsFSM.hasFix())
        {
            gpsFSM.update();
            delay(10);
        }

        // =================================
        // SEND VIA GSM
        // =================================

        gsmModule.begin(9600);

        Serial.println(F("[GSM] SMS SENT"));

        Serial.println(gpsFSM.getURL());

        // =================================
        // DISABLE MODULES
        // =================================

        gsmModule.disable();

        gpsFSM.disable();

        gpsFSM.reset();

        // =================================
        // RETURN SLEEP
        // =================================

        Serial.println(F("[SLEEP] RETURN TO SLEEP"));
    }

    delay(100);

#endif
}