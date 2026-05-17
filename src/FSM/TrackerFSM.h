#pragma once

#include <Arduino.h>

#include "../Config.h"
#include "../NEO/NEO.h"
#include "../SIM/SIM.h"
#include "../MPU/MPU.h"

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

enum class RunMode
{
    DEBUG,
    TRACKER,
    SLEEP
};

class TrackerFSM
{
public:

    TrackerFSM(NEO* gps, SIM* sim, MPU* mpu);

    void begin();
    void update();

private:

    void changeState(TrackerState newState);
    void printState(TrackerState state);

    void loadConfig();

    // handlers
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

private:

    NEO* _gps;
    SIM* _sim;
    MPU* _mpu;

    TrackerState _state;
    RunMode _mode;

    uint32_t _stateStartTime;

    // runtime config from defines
    bool _gpsEnabled;
    bool _simEnabled;
    bool _mpuEnabled;

    bool _gpsMockMode;
    bool _pipelineMode;

    // buffers
    char _gpsRawBuffer[128];
    char _urlBuffer[160];

    float _latitude;
    float _longitude;
};