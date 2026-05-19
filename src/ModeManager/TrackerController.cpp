// =====================================================
// FILE: FILE: src/ModeManager/TrackerController.cpp
// =====================================================

#include "TrackerController.h"
#include "Config.h"
// =====================================================
// CONSTRUCTOR
// =====================================================

TrackerController::TrackerController(
    GPS_FSM* gps,
    GSM_FSM* gsm,
    MPU_FSM* mpu
)
{
    _gps = gps;
    _gsm = gsm;
    _mpu = mpu;

    _state = TrackerState::IDLE;

    _gpsLatch = false;
    _gsmLatch = false;
}

// =====================================================
// BEGIN
// =====================================================

void TrackerController::begin()
{
    #ifdef MODE_TRACKER

        Serial.println(F("[TRACKER] MODE_TRACKER INIT"));

        // MPU unused in tracker mode
        _mpu->setState(MPUState::DISABLED);

        // reset GPS FSM
        _gps->reset();

        // enable GPS
        _gps->enable();

        // configure GPS mode
        #if GPS_MOCK_MODE == 2
            _gps->setState(GPSState::MOCK_PARSE);
        #elif GPS_MOCK_MODE == 3
            _gps->setState(GPSState::REAL_FIX);
        #else
            _gps->setState(GPSState::REAL_FIX);
        #endif

        // configure GSM mode
        #if GPS_MOCK_MODE == 2
            _gsm->setMode(true);
        #else
            _gsm->setMode(false);
        #endif

        _state = TrackerState::GPS_ACTIVE;

        Serial.println(F("[TRACKER] GPS ACTIVE"));

    #endif

    // ========== ДОБАВИТЬ ЭТОТ БЛОК ==========
    #ifdef MODE_SLEEP
        _state = TrackerState::SLEEP_LISTENING;
        Serial.println(F("[TRACKER] FORCE SLEEP STATE"));
    #endif
    // =======================================
}


// =====================================================
// MAIN UPDATE
// =====================================================

void TrackerController::update()
{
    switch (_state)
    {
        // УДАЛИТЕ или ЗАКОММЕНТИРУЙТЕ этот case:
        // case TrackerState::IDLE:
        //     Serial.println(F("[TRACKER] IDLE -> START"));
        //     _state = TrackerState::GPS_ACTIVE;
        //     break;

        case TrackerState::GPS_ACTIVE:
        case TrackerState::GPS_WAIT_FIX:
            processGPS();
            break;

        case TrackerState::GSM_ACTIVE:
        case TrackerState::GSM_WAIT_RESPONSE:
            processGSM();
            break;

        case TrackerState::WAIT_NEXT_CYCLE:
            processCooldown();
            break;

        #ifdef MODE_SLEEP
        case TrackerState::SLEEP_LISTENING:
            processSleepListening();
            break;
        #endif

        default:
            Serial.println(F("[TRACKER] UNKNOWN STATE"));
            resetCycle();
            break;
    }
}
// =====================================================
// RESET CYCLE (SAFE ENTRY POINT)
// =====================================================

void TrackerController::resetCycle()
{
    Serial.println(F("[TRACKER] RESET CYCLE"));

    // =====================================
    // DISABLE MODULES
    // =====================================

    _gps->disable();
    _gsm->disable();

    // =====================================
    // RESET FSM
    // =====================================

    _gps->reset();
    _gsm->reset();

    // =====================================
    // RESET FLAGS
    // =====================================

    _gpsLatch = false;
    _gsmLatch = false;

    // =====================================
    // MODE_SLEEP
    // =====================================

    #ifdef MODE_SLEEP

        Serial.println(F("[TRACKER] RETURN TO MPU LISTENING"));

        _mpu->setState(MPUState::LISTENING);

        _state = TrackerState::SLEEP_LISTENING;

    #else

        // =====================================
        // MODE_TRACKER
        // =====================================

        _state = TrackerState::WAIT_NEXT_CYCLE;

        _cycleCounter++;
        _cycleCooldownStart = millis();

    #endif
}

// =====================================================
// GPS PROCESS
// =====================================================

void TrackerController::processGPS()
{
    _gps->update();

    // -------------------------
    // ENTER STATE ONCE
    // -------------------------
    if (!_gpsLatch)
    {
        _gpsLatch = true;

        _gpsStartTime = millis();

        Serial.println(F("[TRACKER] GPS ACTIVE"));
    }

    // -------------------------
    // FIX RECEIVED (EDGE EVENT)
    // -------------------------
    if (_gps->hasFix())
    {
        if (_gsmLatch) return; // защита от повторного входа

        Serial.println(F("[TRACKER] GPS FIX RECEIVED"));

        _gps->disable();

        _gsm->enable();
        _gsm->setURL(_gps->getURL());

        _gsmLatch = true;
        _state = TrackerState::GSM_ACTIVE;
        _gsmStartTime = millis();

        return;
    }

    // -------------------------
    // TIMEOUT
    // -------------------------
    if (millis() - _gpsStartTime > GPS_TIMEOUT)
    {
        Serial.println(F("[TRACKER] GPS TIMEOUT"));

        resetCycle();
    }
}

// =====================================================
// GSM PROCESS
// =====================================================

void TrackerController::processGSM()
{
    _gsm->update();

    // -------------------------
    // ENTER STATE ONCE
    // -------------------------
    if (_state == TrackerState::GSM_ACTIVE)
    {
        Serial.println(F("[TRACKER] GSM SEND START"));

        _state = TrackerState::GSM_WAIT_RESPONSE;
    }

    // -------------------------
    // DONE
    // -------------------------
    if (_gsm->isDone())
    {
        Serial.println(F("[TRACKER] GSM DONE"));

        _gsm->disable();

        resetCycle();
        return;
    }

    // -------------------------
    // TIMEOUT
    // -------------------------
    if (millis() - _gsmStartTime > GSM_TIMEOUT)
    {
        Serial.println(F("[TRACKER] GSM TIMEOUT"));

        resetCycle();
    }
}

// =====================================================
// COOLDOWN
// =====================================================

void TrackerController::processCooldown()
{
    if (millis() - _cycleCooldownStart < CYCLE_DELAY)
    {
        return;
    }

    Serial.println(F("[TRACKER] NEW CYCLE START"));

    // =====================================
    // RESET FSM
    // =====================================

    _gps->reset();


    //Reset and reconfigure GSM
    _gsm->reset();
    
    #if GPS_MOCK_MODE == 2
        _gsm->setMode(true);  // Mock mode
    #else
        _gsm->setMode(false); // Real mode
    #endif

    // =====================================
    // ENABLE GPS
    // =====================================

    _gps->enable();

    // =====================================
    // RESTORE GPS MODE
    // =====================================

    #if GPS_MOCK_MODE == 2

        _gps->setState(GPSState::MOCK_PARSE);

    #elif GPS_MOCK_MODE == 3

        _gps->setState(GPSState::REAL_FIX);

    #else

        _gps->setState(GPSState::REAL_FIX);

    #endif

    // =====================================
    // RESET FLAGS
    // =====================================

    _gpsLatch = false;
    _gsmLatch = false;

    // =====================================
    // NEXT STATE
    // =====================================

    _state = TrackerState::GPS_ACTIVE;
}

// =====================================================
// TIMER
// =====================================================

unsigned long TrackerController::getCycleDelay() const
{
#ifdef MODE_DEBUG

    // DEBUG MODE BEHAVIOR
    #if GPS_MOCK_MODE == 2
        return 5000;   // 5s fast loop spam mode
    #elif GPS_MOCK_MODE == 1
        return 0;      // instant loop (continuous)
    #else
        return 3000;
    #endif

#elif defined(MODE_TRACKER)

    // TRACKER MODE BEHAVIOR
    #if GPS_MOCK_MODE == 2
        return 10000;  // 10s cycle
    #elif GPS_MOCK_MODE == 3
        return 15000;  // 15s cycle
    #else
        return 10000;
    #endif

#else

    return 10000;

#endif
}


// =====================================================
// SLEEP LISTENING
// =====================================================

void TrackerController::processSleepListening()
{
    if (_mpu == nullptr)
    {
        return;
    }

    _mpu->update();

    // =====================================
    // MOVEMENT DETECTED
    // =====================================

    if (_mpu->isAwaken())
    {
        Serial.println(F("[TRACKER] MPU AWAKEN"));

        // disable MPU during GPS/GSM cycle
        _mpu->setState(MPUState::DISABLED);

        // reset GPS
        _gps->reset();

        // enable GPS
        _gps->enable();

        // configure GPS state
        #if GPS_MOCK_MODE == 2

            _gps->setState(GPSState::MOCK_PARSE);

        #else

            _gps->setState(GPSState::REAL_FIX);

        #endif

        _gpsLatch = false;
        _gsmLatch = false;

        _state = TrackerState::GPS_ACTIVE;

        Serial.println(F("[TRACKER] GPS STARTED"));
    }
}


