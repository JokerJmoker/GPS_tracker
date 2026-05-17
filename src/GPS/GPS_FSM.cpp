// =====================================================
// FILE: src/GPS/GPS_FSM.cpp
// =====================================================

#include "GPS_FSM.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

GPS_FSM::GPS_FSM(GPS* gps)
{
    _gps = gps;

    _state = GPSState::DISABLED;

    _enabled = false;

    _hasFix = false;
    _urlReady = false;
    _oneShotDone = false;

    _lat = 0;
    _lon = 0;

    _url[0] = '\0';
}

// =====================================================
// BEGIN
// =====================================================

void GPS_FSM::begin()
{
    Serial.println(F("[GPS FSM] BEGIN"));
}

// =====================================================
// ENABLE
// =====================================================

void GPS_FSM::enable()
{
    if (_enabled)
        return;

    _gps->begin(9600);

    _enabled = true;

    Serial.println(F("[GPS FSM] ENABLED"));
}

// =====================================================
// DISABLE
// =====================================================

void GPS_FSM::disable()
{
    if (!_enabled)
        return;

    _gps->disable();

    _enabled = false;

    Serial.println(F("[GPS FSM] DISABLED"));
}

// =====================================================
// UPDATE
// =====================================================

void GPS_FSM::update()
{
    if (!_enabled)
        return;

    // =====================================
    // UPDATE UART
    // =====================================

    _gps->update();

    // =====================================
    // FSM
    // =====================================

    switch (_state)
    {
        case GPSState::CONTINUOUS:
            processContinuous();
            break;

        case GPSState::MOCK_PARSE:
            processMock();
            break;

        case GPSState::REAL_FIX:
            processReal();
            break;

        default:
            break;
    }
}

// =====================================================
// MODE 1
// CONTINUOUS GPS OUTPUT
// =====================================================

void GPS_FSM::processContinuous()
{
    const char* data = _gps->getRawData();

    if (data && data[0] == '$')
    {
        Serial.println(F("[GPS CONTINUOUS]"));
        Serial.println(data);
    }
}

// =====================================================
// MODE 2
// MOCK PARSER
// =====================================================

void GPS_FSM::processMock()
{
    // только один проход

    if (_oneShotDone)
        return;

    const char* data =
        "$GNRMC,123519,A,5545.9469,N,3741.1349,E,0.13,309.62,150526,,,A*7C";

    Serial.println();
    Serial.println(F("===== GPS MOCK INPUT ====="));
    Serial.println(data);
    Serial.println(F("=========================="));

    bool ok = parseRMC(data, _lat, _lon);

    if (ok)
    {
        buildYandexURL(_lat, _lon, _url, sizeof(_url));

        _hasFix = true;
        _urlReady = true;

        _oneShotDone = true;

        Serial.println(F("[GPS MOCK] FIX OK"));

        Serial.print(F("LAT: "));
        Serial.println(_lat, 6);

        Serial.print(F("LON: "));
        Serial.println(_lon, 6);

        Serial.print(F("URL: "));
        Serial.println(_url);
    }
    else
    {
        Serial.println(F("[GPS MOCK] PARSE FAILED"));
    }
}

// =====================================================
// MODE 3
// REAL GPS + PARSER
// =====================================================

void GPS_FSM::processReal()
{
    // уже нашли FIX

    if (_oneShotDone)
        return;

    const char* data = _gps->getRawData();

    if (!data)
        return;

    if (data[0] != '$')
        return;

    // =====================================
    // FILTER ONLY RMC
    // =====================================

    if (strncmp(data, "$GNRMC", 6) != 0 &&
        strncmp(data, "$GPRMC", 6) != 0)
    {
        return;
    }

    Serial.println();
    Serial.println(F("===== GPS REAL INPUT ====="));
    Serial.println(data);
    Serial.println(F("=========================="));

    // =====================================
    // PARSER
    // =====================================

    bool ok = parseRMC(data, _lat, _lon);

    if (ok)
    {
        buildYandexURL(_lat, _lon, _url, sizeof(_url));

        _hasFix = true;
        _urlReady = true;

        _oneShotDone = true;

        Serial.println(F("[GPS REAL] FIX OK"));

        Serial.print(F("LAT: "));
        Serial.println(_lat, 6);

        Serial.print(F("LON: "));
        Serial.println(_lon, 6);

        Serial.print(F("URL: "));
        Serial.println(_url);
    }
    else
    {
        Serial.println(F("[GPS REAL] NO FIX"));
    }
}

// =====================================================
// SET STATE
// =====================================================

void GPS_FSM::setState(GPSState state)
{
    _state = state;

    _hasFix = false;
    _urlReady = false;
    _oneShotDone = false;

    Serial.print(F("[GPS FSM] STATE -> "));

    switch (_state)
    {
        case GPSState::DISABLED:
            Serial.println(F("DISABLED"));
            break;

        case GPSState::CONTINUOUS:
            Serial.println(F("CONTINUOUS"));
            break;

        case GPSState::MOCK_PARSE:
            Serial.println(F("MOCK_PARSE"));
            break;

        case GPSState::REAL_FIX:
            Serial.println(F("REAL_FIX"));
            break;
    }
}

// =====================================================
// GETTERS
// =====================================================

GPSState GPS_FSM::getState()
{
    return _state;
}

bool GPS_FSM::isEnabled()
{
    return _enabled;
}

bool GPS_FSM::hasFix()
{
    return _hasFix;
}

bool GPS_FSM::isURLReady()
{
    return _urlReady;
}

const char* GPS_FSM::getURL()
{
    return _url;
}

float GPS_FSM::getLat()
{
    return _lat;
}

float GPS_FSM::getLon()
{
    return _lon;
}

// =====================================================
// RESET
// =====================================================

void GPS_FSM::reset()
{
    _hasFix = false;

    _urlReady = false;

    _oneShotDone = false;

    _lat = 0;
    _lon = 0;

    _url[0] = '\0';

    Serial.println(F("[GPS FSM] RESET"));
}