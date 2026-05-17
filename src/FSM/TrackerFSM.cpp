#include "TrackerFSM.h"
#include "../NEO/GPSParser.h"

// =====================================================
// CONSTRUCTOR
// =====================================================
TrackerFSM::TrackerFSM(NEO* gps, SIM* sim, MPU* mpu)
    : _gps(gps)
    , _sim(sim)
    , _mpu(mpu)
    , _state(TrackerState::BOOT)
    , _mode(RunMode::DEBUG)
    , _stateStartTime(0)
    , _gpsEnabled(false)
    , _simEnabled(false)
    , _mpuEnabled(false)
    , _gpsMockMode(false)
    , _pipelineMode(false)
    , _latitude(0.0f)
    , _longitude(0.0f)
{
    memset(_gpsRawBuffer, 0, sizeof(_gpsRawBuffer));
    memset(_urlBuffer, 0, sizeof(_urlBuffer));
}

// =====================================================
// PUBLIC METHODS
// =====================================================

void TrackerFSM::begin()
{
    loadConfig();

    Serial.println();
    Serial.println(F("=========================================="));

    switch (_mode)
    {
        case RunMode::DEBUG:
            Serial.println(F(">>> MODE: DEBUG (All modules active) <<<"));
            break;
        case RunMode::TRACKER:
            Serial.println(F(">>> MODE: TRACKER (GPS → GSM cycle) <<<"));
            break;
        case RunMode::SLEEP:
            Serial.println(F(">>> MODE: SLEEP (Wake on motion) <<<"));
            break;
    }

    Serial.println(F("=========================================="));
    Serial.println();

    // Initial hardware state
    if (_gpsEnabled && _gps)
    {
        if (_mode == RunMode::DEBUG || _state == TrackerState::GPS_POWER_ON)
        {
            _gps->begin(9600);
        }
        else
        {
            _gps->disable();
        }
    }

    if (_simEnabled && _sim)
    {
        if (_mode == RunMode::DEBUG)
        {
            _sim->begin(9600);
        }
        else
        {
            _sim->disable();
        }
    }

    if (_mpuEnabled && _mpu)
    {
        if (_mode == RunMode::DEBUG || _mode == RunMode::SLEEP)
        {
            _mpu->begin();
        }
        else
        {
            _mpu->disable();
        }
    }

    changeState(TrackerState::BOOT);
}

void TrackerFSM::update()
{
    if (!_gps || !_sim || !_mpu)
    {
        changeState(TrackerState::ERROR_STATE);
        handleError();
        return;
    }

    // Always update hardware buffers (non-blocking reads)
    if (_gpsEnabled && _gps->isEnabled())
    {
        _gps->update();
    }

    if (_simEnabled && _sim->isEnabled())
    {
        _sim->update();
    }

    if (_mpuEnabled && _mpu->isEnabled())
    {
        _mpu->update();
    }

    // State machine
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

// =====================================================
// STATE MANAGEMENT
// =====================================================

void TrackerFSM::changeState(TrackerState newState)
{
    if (_state == newState)
    {
        return;
    }

    _state = newState;
    _stateStartTime = millis();

    printState(newState);
}

void TrackerFSM::printState(TrackerState state)
{
    Serial.print(F("[FSM] State: "));

    switch (state)
    {
        case TrackerState::BOOT:
            Serial.println(F("BOOT"));
            break;
        case TrackerState::GPS_POWER_ON:
            Serial.println(F("GPS_POWER_ON"));
            break;
        case TrackerState::GPS_WAIT_DATA:
            Serial.println(F("GPS_WAIT_DATA"));
            break;
        case TrackerState::GPS_PARSE:
            Serial.println(F("GPS_PARSE"));
            break;
        case TrackerState::GPS_POWER_OFF:
            Serial.println(F("GPS_POWER_OFF"));
            break;
        case TrackerState::GSM_POWER_ON:
            Serial.println(F("GSM_POWER_ON"));
            break;
        case TrackerState::GSM_INIT:
            Serial.println(F("GSM_INIT"));
            break;
        case TrackerState::GSM_SEND_SMS:
            Serial.println(F("GSM_SEND_SMS"));
            break;
        case TrackerState::GSM_WAIT_SMS_RESULT:
            Serial.println(F("GSM_WAIT_SMS_RESULT"));
            break;
        case TrackerState::GSM_POWER_OFF:
            Serial.println(F("GSM_POWER_OFF"));
            break;
        case TrackerState::SLEEP:
            Serial.println(F("SLEEP"));
            break;
        case TrackerState::ERROR_STATE:
            Serial.println(F("ERROR"));
            break;
        default:
            Serial.println(F("UNKNOWN"));
            break;
    }
}

// =====================================================
// CONFIGURATION
// =====================================================

void TrackerFSM::loadConfig()
{
    // Read runtime configuration from Config.h defines

#ifdef MODE_DEBUG
    _mode = RunMode::DEBUG;
#elif defined(MODE_TRACKER)
    _mode = RunMode::TRACKER;
#elif defined(MODE_SLEEP)
    _mode = RunMode::SLEEP;
#else
    _mode = RunMode::DEBUG;  // Default
#endif

#ifdef TEST_GPS
    _gpsEnabled = true;
#else
    _gpsEnabled = false;
#endif

#ifdef TEST_SIM800L
    _simEnabled = true;
#else
    _simEnabled = false;
#endif

#ifdef TEST_MPU6050
    _mpuEnabled = true;
#else
    _mpuEnabled = false;
#endif

#ifdef GPS_MOCK_MODE
    _gpsMockMode = (GPS_MOCK_MODE == 1);
#else
    _gpsMockMode = false;
#endif

#ifdef TEST_PIPELINE_MODE
    _pipelineMode = (TEST_PIPELINE_MODE == 1);
#else
    _pipelineMode = false;
#endif

    // Module status output
    Serial.println(F("[CFG] === Configuration === "));
    Serial.print(F("[CFG] Mode: "));
    switch (_mode)
    {
        case RunMode::DEBUG:   Serial.println(F("DEBUG")); break;
        case RunMode::TRACKER: Serial.println(F("TRACKER")); break;
        case RunMode::SLEEP:   Serial.println(F("SLEEP")); break;
    }

    Serial.print(F("[CFG] GPS: "));
    Serial.println(_gpsEnabled ? F("ENABLED") : F("DISABLED"));

    Serial.print(F("[CFG] SIM800L: "));
    Serial.println(_simEnabled ? F("ENABLED") : F("DISABLED"));

    Serial.print(F("[CFG] MPU6050: "));
    Serial.println(_mpuEnabled ? F("ENABLED") : F("DISABLED"));

    Serial.print(F("[CFG] GPS Mock Mode: "));
    Serial.println(_gpsMockMode ? F("ON") : F("OFF"));

    Serial.print(F("[CFG] Pipeline Mode: "));
    Serial.println(_pipelineMode ? F("ON") : F("OFF"));

    Serial.println(F("[CFG] ======================"));
    Serial.println();
}

// =====================================================
// STATE HANDLERS
// =====================================================

void TrackerFSM::handleBoot()
{
    // Boot delay to let hardware stabilize
    const uint32_t BOOT_DELAY = 2000;

    if (millis() - _stateStartTime >= BOOT_DELAY)
    {
        if (_mode == RunMode::DEBUG)
        {
            // DEBUG mode: all modules active, but we still follow FSM if pipeline mode is on
            if (_pipelineMode && _gpsEnabled && _simEnabled)
            {
                changeState(TrackerState::GPS_POWER_ON);
            }
            else
            {
                // In DEBUG without pipeline, just stay in BOOT and let update() handle raw output
                // Actually we just stay here and do nothing - modules are already active
            }
        }
        else if (_mode == RunMode::TRACKER)
        {
            // TRACKER mode: start GPS cycle
            if (_gpsEnabled)
            {
                changeState(TrackerState::GPS_POWER_ON);
            }
            else if (_simEnabled)
            {
                changeState(TrackerState::GSM_POWER_ON);
            }
            else
            {
                changeState(TrackerState::ERROR_STATE);
            }
        }
        else if (_mode == RunMode::SLEEP)
        {
            // SLEEP mode: go to sleep, wake on MPU interrupt
            changeState(TrackerState::SLEEP);
        }
    }
}

void TrackerFSM::handleGPSPowerOn()
{
    const uint32_t POWER_ON_DELAY = 500;  // ms for GPS to stabilize

    if (!_gpsEnabled)
    {
        changeState(TrackerState::ERROR_STATE);
        return;
    }

    // Enable GPS if not already enabled
    if (!_gps->isEnabled())
    {
        _gps->enable();
        _gps->begin(9600);
        Serial.println(F("[GPS] Power ON"));
    }

    if (millis() - _stateStartTime >= POWER_ON_DELAY)
    {
        changeState(TrackerState::GPS_WAIT_DATA);
    }
}

void TrackerFSM::handleGPSWaitData()
{
    const uint32_t GPS_TIMEOUT = 30000;  // 30 seconds max wait for fix
    const uint32_t GPS_READ_INTERVAL = 1000;

    static uint32_t lastRead = 0;
    static bool waitingForFix = true;

    if (waitingForFix && (millis() - _stateStartTime >= GPS_TIMEOUT))
    {
        Serial.println(F("[GPS] Timeout - no fix"));
        waitingForFix = true;
        changeState(TrackerState::GPS_POWER_OFF);
        return;
    }

    if (millis() - lastRead >= GPS_READ_INTERVAL)
    {
        lastRead = millis();

        const char* rawData = _gps->getRawData();

        if (rawData && rawData[0] == '$')
        {
            // Check if it's an RMC sentence
            if (strncmp(rawData, "$GNRMC", 6) == 0 ||
                strncmp(rawData, "$GPRMC", 6) == 0)
            {
                strncpy(_gpsRawBuffer, rawData, sizeof(_gpsRawBuffer) - 1);
                _gpsRawBuffer[sizeof(_gpsRawBuffer) - 1] = '\0';
                waitingForFix = true;
                changeState(TrackerState::GPS_PARSE);
            }
        }
    }
}

void TrackerFSM::handleGPSParse()
{
    bool parseResult = false;

    // MOCK mode override
    if (_gpsMockMode)
    {
        static uint32_t lastMock = 0;
        if (millis() - lastMock >= 2000)
        {
            lastMock = millis();
            const char* mockData = "$GNRMC,123519,A,5545.9469,N,3741.1349,E,0.13,309.62,150526,,,A*7C";
            strncpy(_gpsRawBuffer, mockData, sizeof(_gpsRawBuffer) - 1);
            parseResult = parseRMC(_gpsRawBuffer, _latitude, _longitude);
        }
        else
        {
            return;
        }
    }
    else
    {
        parseResult = parseRMC(_gpsRawBuffer, _latitude, _longitude);
    }

    if (parseResult)
    {
        buildYandexURL(_latitude, _longitude, _urlBuffer, sizeof(_urlBuffer));

        Serial.println(F("[GPS] FIX OK"));
        Serial.print(F("  LAT: "));
        Serial.println(_latitude, 6);
        Serial.print(F("  LON: "));
        Serial.println(_longitude, 6);
        Serial.print(F("  URL: "));
        Serial.println(_urlBuffer);

        if (_simEnabled)
        {
            changeState(TrackerState::GPS_POWER_OFF);
        }
        else
        {
            // No SIM - just restart GPS cycle
            _latitude = 0;
            _longitude = 0;
            memset(_urlBuffer, 0, sizeof(_urlBuffer));
            changeState(TrackerState::GPS_POWER_ON);
        }
    }
    else
    {
        Serial.println(F("[GPS] Parse failed - no fix"));
        changeState(TrackerState::GPS_WAIT_DATA);
    }
}

void TrackerFSM::handleGPSPowerOff()
{
    const uint32_t POWER_OFF_DELAY = 100;

    if (_gps->isEnabled())
    {
        _gps->disable();
        Serial.println(F("[GPS] Power OFF"));
    }

    if (millis() - _stateStartTime >= POWER_OFF_DELAY)
    {
        if (_simEnabled)
        {
            changeState(TrackerState::GSM_POWER_ON);
        }
        else
        {
            // No GSM, restart GPS cycle
            _latitude = 0;
            _longitude = 0;
            memset(_urlBuffer, 0, sizeof(_urlBuffer));
            changeState(TrackerState::GPS_POWER_ON);
        }
    }
}

// =====================================================
// GSM STATE HANDLERS
// =====================================================

void TrackerFSM::handleGSMPowerOn()
{
    const uint32_t POWER_ON_DELAY = 1000;  // GSM needs more time

    if (!_simEnabled)
    {
        changeState(TrackerState::ERROR_STATE);
        return;
    }

    if (!_sim->isEnabled())
    {
        _sim->enable();
        _sim->begin(9600);
        Serial.println(F("[GSM] Power ON"));
    }

    if (millis() - _stateStartTime >= POWER_ON_DELAY)
    {
        changeState(TrackerState::GSM_INIT);
    }
}

void TrackerFSM::handleGSMInit()
{
    static int initStep = 0;
    static unsigned long lastCommand = 0;
    static bool waitingResponse = false;

    const unsigned long COMMAND_DELAY = 2000;
    const unsigned long RESPONSE_TIMEOUT = 30000;

    const char* initCommands[] = {
        "AT",
        "AT+CPIN?",
        "AT+CREG=1",
        "AT+CREG?",
        "AT+CSQ"
    };
    const int COMMAND_COUNT = sizeof(initCommands) / sizeof(initCommands[0]);

    // Check for responses
    String fullResponse = _sim->getFullResponse();

    if (fullResponse.length() > 0 && waitingResponse)
    {
        waitingResponse = false;
        lastCommand = millis();

        if (fullResponse.indexOf("ERROR") >= 0)
        {
            Serial.println(F("[GSM] Command failed"));
        }
        else
        {
            Serial.println(F("[GSM] Command OK"));
        }

        initStep++;
    }

    // Timeout handling
    if (waitingResponse && (millis() - _stateStartTime >= RESPONSE_TIMEOUT))
    {
        Serial.println(F("[GSM] Init timeout"));
        waitingResponse = false;
        initStep++;
    }

    // Send next command
    if (!waitingResponse && (millis() - lastCommand >= COMMAND_DELAY))
    {
        if (initStep < COMMAND_COUNT)
        {
            Serial.print(F("[GSM] Sending: "));
            Serial.println(initCommands[initStep]);

            _sim->sendCommand(initCommands[initStep]);
            waitingResponse = true;
        }
        else
        {
            // Init complete
            Serial.println(F("[GSM] Init complete"));
            initStep = 0;
            waitingResponse = false;
            changeState(TrackerState::GSM_SEND_SMS);
        }
    }
}

void TrackerFSM::handleGSMSendSMS()
{
    static bool smsSent = false;
    static unsigned long sendStartTime = 0;
    
    if (!smsSent)
    {
        sendStartTime = millis();
        Serial.println(F("[GSM] Sending SMS..."));
        
        // Используем встроенный метод sendSMS
        bool result = _sim->sendSMS("+79991234567", _urlBuffer, 30000);
        
        if (result)
        {
            Serial.println(F("[GSM] SMS sent successfully"));
            smsSent = true;
        }
        else
        {
            Serial.println(F("[GSM] SMS send failed"));
            smsSent = true;  // Переходим к выключению питания
        }
    }
    
    // Даем время на завершение всех операций
    if (smsSent && (millis() - sendStartTime >= 2000))
    {
        smsSent = false;
        changeState(TrackerState::GSM_POWER_OFF);
    }
}

void TrackerFSM::handleGSMWaitResult()
{
    // Этот метод можно вообще убрать, так как sendSMS уже ждет результат
    // Просто переходим к выключению питания
    changeState(TrackerState::GSM_POWER_OFF);
}

void TrackerFSM::handleGSMPowerOff()
{
    const uint32_t POWER_OFF_DELAY = 500;

    if (_sim->isEnabled())
    {
        _sim->disable();
        Serial.println(F("[GSM] Power OFF"));
    }

    if (millis() - _stateStartTime >= POWER_OFF_DELAY)
    {
        // Clear buffers for next cycle
        _latitude = 0;
        _longitude = 0;
        memset(_urlBuffer, 0, sizeof(_urlBuffer));

        if (_mode == RunMode::TRACKER)
        {
            // Start new cycle with GPS
            changeState(TrackerState::GPS_POWER_ON);
        }
        else if (_mode == RunMode::SLEEP)
        {
            changeState(TrackerState::SLEEP);
        }
        else if (_mode == RunMode::DEBUG)
        {
            if (_pipelineMode)
            {
                changeState(TrackerState::GPS_POWER_ON);
            }
        }
    }
}

// =====================================================
// SLEEP AND ERROR HANDLERS
// =====================================================

void TrackerFSM::handleSleep()
{
#ifdef MODE_SLEEP
    static bool wakingUp = false;

    if (!wakingUp)
    {
        Serial.println(F("[SLEEP] Entering deep sleep..."));
        Serial.println(F("[SLEEP] Waiting for MPU interrupt"));

        // Disable all modules
        if (_gps && _gps->isEnabled()) _gps->disable();
        if (_sim && _sim->isEnabled()) _sim->disable();

        // Go to sleep (implementation depends on hardware)
        // SystemModes::goToSleep();

        wakingUp = true;
    }

    // Check MPU for motion (wake condition)
    if (_mpu && _mpuEnabled && _mpu->isMotionDetected())
    {
        Serial.println(F("[SLEEP] Motion detected! Waking up..."));
        wakingUp = false;

        if (_gpsEnabled)
        {
            changeState(TrackerState::GPS_POWER_ON);
        }
    }
#else
    // Not in sleep mode - just go back to boot
    changeState(TrackerState::BOOT);
#endif
}

void TrackerFSM::handleError()
{
    Serial.println(F("[ERROR] FSM entered error state"));

    // Try to recover by resetting all modules
    if (_gps) _gps->disable();
    if (_sim) _sim->disable();
    if (_mpu) _mpu->disable();

    delay(1000);

    // Attempt restart
    _latitude = 0;
    _longitude = 0;
    memset(_urlBuffer, 0, sizeof(_urlBuffer));

    changeState(TrackerState::BOOT);
}