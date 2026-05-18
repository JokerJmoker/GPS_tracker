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

// =====================================================
// GPS ACTIVE PHASE
// =====================================================

if (SystemModes::shouldGPSBeActive())
{
    if (!gpsFSM.isEnabled())
    {
        gpsFSM.enable();
        gsmFSM.disable();

        Serial.println(F("[TRACKER] GPS ACTIVE"));
    }

    gpsFSM.update();
}

// =====================================================
// GPS FIX → START GSM FSM
// =====================================================

if (gpsFSM.hasFix())
{
    Serial.println();
    Serial.println(F("[TRACKER] GPS FIX RECEIVED"));

    Serial.print(F("[TRACKER] URL: "));
    Serial.println(gpsFSM.getURL());

    // =============================
    // STOP GPS
    // =============================

    gpsFSM.disable();

    // =============================
    // START GSM FSM (ONLY CONTROL POINT)
    // =============================

    gsmFSM.begin();
    gsmFSM.setURL(gpsFSM.getURL());

    // MOCK MODE SELECTION
    #if GPS_MOCK_MODE == 2
        gsmFSM.setMode(true);   // MOCK SEND
        Serial.println(F("[TRACKER] GSM MOCK MODE"));
    #else
        gsmFSM.setMode(false);  // REAL SEND
        Serial.println(F("[TRACKER] GSM REAL MODE"));
    #endif

    gsmFSM.enable();

    Serial.println(F("[TRACKER] GSM FSM STARTED"));

    gpsFSM.reset();
}

// =====================================================
// GSM FSM UPDATE
// =====================================================

if (gsmFSM.isEnabled())
{
    gsmFSM.update();

    if (gsmFSM.isDone())
    {
        Serial.println();
        Serial.println(F("[TRACKER] GSM COMPLETE"));

        gsmFSM.disable();

        gpsFSM.enable();

        Serial.println(F("[TRACKER] RETURN TO GPS"));
    }
}

delay(50);

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

// =====================================================
// GPS ACTIVE PHASE
// =====================================================

if (SystemModes::shouldGPSBeActive())
{
    if (!gpsFSM.isEnabled())
    {
        gpsFSM.enable();
        gsm.disable();

        Serial.println(F("[TRACKER] GPS ACTIVE"));
    }

    gpsFSM.update();
}

// =====================================================
// GPS FIX RECEIVED → SWITCH TO GSM FSM
// =====================================================

if (gpsFSM.hasFix())
{
    Serial.println();
    Serial.println(F("[TRACKER] GPS FIX RECEIVED"));

    Serial.print(F("[TRACKER] URL: "));
    Serial.println(gpsFSM.getURL());

    // =====================================
    // STOP GPS
    // =====================================

    gpsFSM.disable();

    // =====================================
    // START GSM FSM (IMPORTANT CHANGE)
    // =====================================

    gsmFSM.begin();
    gsmFSM.setURL(gpsFSM.getURL());
    gsmFSM.setMode(true); // MOCK MODE (или false если real)

    gsmFSM.enable();

    Serial.println(F("[TRACKER] GSM FSM STARTED"));

    // =====================================
    // RESET GPS LAYER (prepare next cycle)
    // =====================================

    gpsFSM.reset();

    Serial.println(F("[TRACKER] SWITCHED TO GSM FSM"));
}

// =====================================================
// GSM FSM PROCESSING (NEW CONTROL LAYER)
// =====================================================

if (gsmFSM.isEnabled())
{
    gsmFSM.update();

    // =====================================
    // GSM DONE → END CYCLE
    // =====================================

    if (gsmFSM.isDone())
    {
        Serial.println();
        Serial.println(F("[TRACKER] GSM SEND COMPLETE"));

        // =====================================
        // STOP GSM
        // =====================================

        gsmFSM.disable();

        // =====================================
        // RETURN TO GPS MODE
        // =====================================

        gpsFSM.enable();

        Serial.println(F("[TRACKER] RETURN TO GPS"));
    }
}

// =====================================================
// SAFETY DELAY
// =====================================================

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