#pragma once

#include <Arduino.h>

#include "../NEO/NEO.h"
#include "../SIM/SIM.h"
#include "../MPU/MPU.h"

// ============================================
// FSM STATES
// ============================================

enum class TrackerState
{
    BOOT,

    GPS_POWER_ON,
    GPS_WAIT_DATA,
    GPS_PARSE,
    GPS_POWER_OFF,

    GSM_POWER_ON,
    GSM_INIT,
    GSM_SEND_SMS,
    GSM_WAIT_SMS_RESULT,
    GSM_POWER_OFF,

    SLEEP,

    ERROR_STATE
};

// ============================================
// TRACKER FSM
// ============================================

class TrackerFSM
{
private:

    TrackerState _state;

    unsigned long _stateStartTime;

    NEO* _gps;
    SIM* _sim;
    MPU* _mpu;

    // GPS DATA
    char _gpsRawBuffer[128];

    float _latitude;
    float _longitude;

    // READY URL
    char _urlBuffer[128];

public:

    TrackerFSM(
        NEO* gps,
        SIM* sim,
        MPU* mpu
    );

    void begin();

    void update();

private:

    void changeState(TrackerState newState);

    void handleBoot();

    void handleGPSPowerOn();
    void handleGPSWaitData();
    void handleGPSParse();
    void handleGPSPowerOff();

    void handleGSMPowerOn();
    void handleGSMInit();
    void handleGSMSendSMS();
    void handleGSMWaitResult();
    void handleGSMPowerOff();

    void handleSleep();

    void handleError();

    void printState(TrackerState state);
};