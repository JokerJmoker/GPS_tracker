// =====================================================
// FILE: src/ModeManager/TrackerController.cpp
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
    Serial.println(F("[TRACKER] BEGIN"));

    // reset flags
    _gpsLatch = false;
    _gsmLatch = false;

#ifdef MODE_SLEEP

    Serial.println(F("[TRACKER] MODE_SLEEP INIT"));

    _gps->disable();
    _gsm->disable();

    _mpu->begin();
    _mpu->setState(MPUState::LISTENING);

    _state = TrackerState::SLEEP_LISTENING;

    Serial.println(F("[TRACKER] ENTER SLEEP LISTENING"));

    return;
#endif

#ifdef MODE_TRACKER

    Serial.println(F("[TRACKER] MODE_TRACKER INIT"));

    _mpu->setState(MPUState::DISABLED);

    _gps->reset();
    _gps->enable();

    #if GPS_MOCK_MODE == 2
        _gps->setState(GPSState::MOCK_PARSE);
    #else
        _gps->setState(GPSState::REAL_FIX);
    #endif

    #if GPS_MOCK_MODE == 2
        _gsm->setMode(true);
    #else
        _gsm->setMode(false);
    #endif

    _state = TrackerState::GPS_ACTIVE;

    Serial.println(F("[TRACKER] GPS ACTIVE"));

#endif
}

// =====================================================
// UPDATE
// =====================================================

void TrackerController::update()
{
    switch (_state)
    {
        case TrackerState::IDLE:
            Serial.println(F("[TRACKER] IDLE -> AUTO START"));

#ifdef MODE_SLEEP
            _state = TrackerState::SLEEP_LISTENING;
#else
            _state = TrackerState::GPS_ACTIVE;
#endif
            break;

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
            Serial.println(F("[TRACKER] UNKNOWN STATE -> RESET"));
            resetCycle();
            break;
    }
}

// =====================================================
// RESET CYCLE
// =====================================================

void TrackerController::resetCycle()
{
    Serial.println(F("[TRACKER] RESET CYCLE"));

    _gps->disable();
    _gsm->disable();

    _gps->reset();
    _gsm->reset();

    _gpsLatch = false;
    _gsmLatch = false;

#ifdef MODE_SLEEP

    Serial.println(F("[TRACKER] BACK TO SLEEP LISTENING"));

    _mpu->setState(MPUState::LISTENING);

    _state = TrackerState::SLEEP_LISTENING;

#else

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

    if (!_gpsLatch)
    {
        _gpsLatch = true;
        _gpsStartTime = millis();
        Serial.println(F("[TRACKER] GPS ACTIVE"));
    }

    if (_gps->hasFix())
    {
        if (_gsmLatch) return;

        Serial.println(F("[TRACKER] GPS FIX RECEIVED"));

        _gps->disable();

        _gsm->enable();
        _gsm->setURL(_gps->getURL());

        _gsmLatch = true;

        _state = TrackerState::GSM_ACTIVE;
        _gsmStartTime = millis();

        return;
    }

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

    if (_state == TrackerState::GSM_ACTIVE)
    {
        Serial.println(F("[TRACKER] GSM SEND START"));
        _state = TrackerState::GSM_WAIT_RESPONSE;
    }

    if (_gsm->isDone())
    {
        Serial.println(F("[TRACKER] GSM DONE"));
        _gsm->disable();
        resetCycle();
        return;
    }

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
        return;

    Serial.println(F("[TRACKER] NEW CYCLE START"));

    _gps->reset();
    _gsm->reset();

    _gps->enable();

#if GPS_MOCK_MODE == 2
    _gsm->setMode(true);
    _gps->setState(GPSState::MOCK_PARSE);
#else
    _gsm->setMode(false);
    _gps->setState(GPSState::REAL_FIX);
#endif

    _gpsLatch = false;
    _gsmLatch = false;

    _state = TrackerState::GPS_ACTIVE;
}

// =====================================================
// SLEEP LISTENING
// =====================================================

#ifdef MODE_SLEEP
void TrackerController::processSleepListening()
{
    if (_mpu == nullptr)
        return;

    _mpu->update();

    if (_mpu->isAwaken())
    {
        Serial.println(F("[TRACKER] MPU AWAKEN"));

        _mpu->setState(MPUState::DISABLED);

        _gps->reset();
        _gps->enable();

#if GPS_MOCK_MODE == 2
        _gps->setState(GPSState::MOCK_PARSE);
#else
        _gps->setState(GPSState::REAL_FIX);
#endif

        _gpsLatch = false;
        _gsmLatch = false;

        _state = TrackerState::GPS_ACTIVE;

        Serial.println(F("[TRACKER] WAKE -> GPS START"));
    }
}
#endif