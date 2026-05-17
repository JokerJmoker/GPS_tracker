#include "TrackerFSM.h"

#include "../NEO/GPSParser.h"

// ============================================
// CONSTRUCTOR
// ============================================

TrackerFSM::TrackerFSM(
    NEO* gps,
    SIM* sim,
    MPU* mpu
)
{
    _gps = gps;
    _sim = sim;
    _mpu = mpu;

    _state = TrackerState::BOOT;

    _stateStartTime = 0;

    memset(_gpsRawBuffer, 0, sizeof(_gpsRawBuffer));
    memset(_urlBuffer, 0, sizeof(_urlBuffer));

    _latitude = 0.0f;
    _longitude = 0.0f;
}

// ============================================
// BEGIN
// ============================================

void TrackerFSM::begin()
{
    changeState(TrackerState::BOOT);
}

// ============================================
// UPDATE
// ============================================

void TrackerFSM::update()
{
    switch (_state)
    {
        case TrackerState::BOOT:
            handleBoot();
            break;

        case TrackerState::GPS_POWER_ON:
            handleGPSPowerOn();
            break;

        case TrackerState::GPS_WAIT_DATA:
            handleGPSWaitData();
            break;

        case TrackerState::GPS_PARSE:
            handleGPSParse();
            break;

        case TrackerState::GPS_POWER_OFF:
            handleGPSPowerOff();
            break;

        case TrackerState::GSM_POWER_ON:
            handleGSMPowerOn();
            break;

        case TrackerState::GSM_INIT:
            handleGSMInit();
            break;

        case TrackerState::GSM_SEND_SMS:
            handleGSMSendSMS();
            break;

        case TrackerState::GSM_WAIT_SMS_RESULT:
            handleGSMWaitResult();
            break;

        case TrackerState::GSM_POWER_OFF:
            handleGSMPowerOff();
            break;

        case TrackerState::SLEEP:
            handleSleep();
            break;

        case TrackerState::ERROR_STATE:
            handleError();
            break;
    }
}

// ============================================
// STATE CHANGE
// ============================================

void TrackerFSM::changeState(TrackerState newState)
{
    _state = newState;

    _stateStartTime = millis();

    printState(newState);
}

// ============================================
// PRINT STATE
// ============================================

void TrackerFSM::printState(TrackerState state)
{
    Serial.print("[FSM] STATE -> ");

    switch(state)
    {
        case TrackerState::BOOT:
            Serial.println("BOOT");
            break;

        case TrackerState::GPS_POWER_ON:
            Serial.println("GPS_POWER_ON");
            break;

        case TrackerState::GPS_WAIT_DATA:
            Serial.println("GPS_WAIT_DATA");
            break;

        case TrackerState::GPS_PARSE:
            Serial.println("GPS_PARSE");
            break;

        case TrackerState::GPS_POWER_OFF:
            Serial.println("GPS_POWER_OFF");
            break;

        case TrackerState::GSM_POWER_ON:
            Serial.println("GSM_POWER_ON");
            break;

        case TrackerState::GSM_INIT:
            Serial.println("GSM_INIT");
            break;

        case TrackerState::GSM_SEND_SMS:
            Serial.println("GSM_SEND_SMS");
            break;

        case TrackerState::GSM_WAIT_SMS_RESULT:
            Serial.println("GSM_WAIT_SMS_RESULT");
            break;

        case TrackerState::GSM_POWER_OFF:
            Serial.println("GSM_POWER_OFF");
            break;

        case TrackerState::SLEEP:
            Serial.println("SLEEP");
            break;

        case TrackerState::ERROR_STATE:
            Serial.println("ERROR_STATE");
            break;
    }
}

// ============================================
// BOOT
// ============================================

void TrackerFSM::handleBoot()
{
    _gps->disable();
    _sim->disable();

    delay(500);

    changeState(TrackerState::GPS_POWER_ON);
}

// ============================================
// GPS POWER ON
// ============================================

void TrackerFSM::handleGPSPowerOn()
{
    _sim->disable();

    if (!_gps->isEnabled())
    {
        _gps->enable();
    }

    changeState(TrackerState::GPS_WAIT_DATA);
}

// ============================================
// GPS WAIT DATA
// ============================================

void TrackerFSM::handleGPSWaitData()
{
    _gps->update();

    const char* data = _gps->getRawData();

    if (data == nullptr)
    {
        return;
    }

    strncpy(
        _gpsRawBuffer,
        data,
        sizeof(_gpsRawBuffer) - 1
    );

    changeState(TrackerState::GPS_PARSE);
}

// ============================================
// GPS PARSE
// ============================================

void TrackerFSM::handleGPSParse()
{
    bool ok =
        parseRMC(
            _gpsRawBuffer,
            _latitude,
            _longitude
        );

    if (!ok)
    {
        changeState(TrackerState::GPS_WAIT_DATA);
        return;
    }

    buildYandexURL(
        _latitude,
        _longitude,
        _urlBuffer,
        sizeof(_urlBuffer)
    );

    Serial.println("[GPS] FIX OK");

    Serial.print("[GPS] URL: ");
    Serial.println(_urlBuffer);

    changeState(TrackerState::GPS_POWER_OFF);
}

// ============================================
// GPS POWER OFF
// ============================================

void TrackerFSM::handleGPSPowerOff()
{
    _gps->disable();

    changeState(TrackerState::GSM_POWER_ON);
}

// ============================================
// GSM POWER ON
// ============================================

void TrackerFSM::handleGSMPowerOn()
{
    if (!_sim->isEnabled())
    {
        _sim->enable();
    }

    changeState(TrackerState::GSM_INIT);
}

// ============================================
// GSM INIT
// ============================================

void TrackerFSM::handleGSMInit()
{
    Serial.println("[SIM] INIT");

    changeState(TrackerState::GSM_SEND_SMS);
}

// ============================================
// GSM SEND SMS
// ============================================

void TrackerFSM::handleGSMSendSMS()
{
    Serial.println("[SIM] SEND SMS");

    bool result =
        _sim->sendSMS(
            "+79999999999",
            _urlBuffer
        );

    if (result)
    {
        changeState(
            TrackerState::GSM_WAIT_SMS_RESULT
        );
    }
    else
    {
        changeState(
            TrackerState::ERROR_STATE
        );
    }
}

// ============================================
// GSM WAIT RESULT
// ============================================

void TrackerFSM::handleGSMWaitResult()
{
    Serial.println("[SIM] SMS COMPLETE");

    changeState(TrackerState::GSM_POWER_OFF);
}

// ============================================
// GSM POWER OFF
// ============================================

void TrackerFSM::handleGSMPowerOff()
{
    _sim->disable();

    delay(3000);

    changeState(TrackerState::GPS_POWER_ON);
}

// ============================================
// SLEEP
// ============================================

void TrackerFSM::handleSleep()
{
}

// ============================================
// ERROR
// ============================================

void TrackerFSM::handleError()
{
    Serial.println("[FSM] ERROR");

    delay(3000);

    changeState(TrackerState::GPS_POWER_ON);
}