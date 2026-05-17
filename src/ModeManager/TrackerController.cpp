// =====================================================
// FILE: src/ModeManager/TrackerController.cpp
// =====================================================

#include "TrackerController.h"

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
}

// =====================================================
// BEGIN
// =====================================================

void TrackerController::begin()
{
    Serial.println(F("[TRACKER] BEGIN"));
}

// =====================================================
// UPDATE
// =====================================================

void TrackerController::update()
{
    switch (_state)
    {
        case TrackerState::GPS_ACTIVE:
            processGPS();
            break;

        case TrackerState::GSM_ACTIVE:
            processGSM();
            break;

        default:
            break;
    }
}

// =====================================================
// PROCESS GPS
// =====================================================

void TrackerController::processGPS()
{
    _gps->update();

    if (_gps->hasFix())
    {
        Serial.println(F("[TRACKER] GPS FIX OK"));

        _gps->disable();

        _gsm->enable();

        _gsm->setURL(_gps->getURL());

        _state = TrackerState::GSM_ACTIVE;
    }
}

// =====================================================
// PROCESS GSM
// =====================================================

void TrackerController::processGSM()
{
    _gsm->update();

    if (_gsm->isDone())
    {
        Serial.println(F("[TRACKER] GSM DONE"));

        _gsm->disable();

        _gps->reset();

        _gps->enable();

        _state = TrackerState::GPS_ACTIVE;
    }
}

// =====================================================
// SET STATE
// =====================================================

void TrackerController::setState(TrackerState state)
{
    _state = state;

    Serial.print(F("[TRACKER] STATE -> "));

    switch (_state)
    {
        case TrackerState::IDLE:
            Serial.println(F("IDLE"));
            break;

        case TrackerState::GPS_ACTIVE:
            Serial.println(F("GPS_ACTIVE"));
            break;

        case TrackerState::GSM_ACTIVE:
            Serial.println(F("GSM_ACTIVE"));
            break;

        case TrackerState::SLEEP:
            Serial.println(F("SLEEP"));
            break;

        default:
            Serial.println(F("OTHER"));
            break;
    }
}

// =====================================================
// GET STATE
// =====================================================

TrackerState TrackerController::getState()
{
    return _state;
}