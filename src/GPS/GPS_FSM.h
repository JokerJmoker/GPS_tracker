// =====================================================
// FILE: src/GPS/GPS_FSM.h
// =====================================================

#ifndef GPS_FSM_H
#define GPS_FSM_H

#include <Arduino.h>
#include "GPS.h"
#include "GPSParser.h"

// =====================================================
// GPS FSM STATES
// =====================================================

enum class GPSState
{
    DISABLED,      // GPS выключен
    CONTINUOUS,    // Режим 1 - непрерывный вывод
    MOCK_PARSE,    // Режим 2 - mock parser
    REAL_FIX       // Режим 3 - реальный FIX
};

// =====================================================
// GPS FSM
// =====================================================

class GPS_FSM
{
private:

    // =====================================
    // GPS MODULE
    // =====================================

    GPS* _gps;

    // =====================================
    // STATE
    // =====================================

    GPSState _state;

    // =====================================
    // FLAGS
    // =====================================

    bool _enabled;
    bool _hasFix;
    bool _urlReady;
    bool _oneShotDone;

    // =====================================
    // GPS DATA
    // =====================================

    float _lat;
    float _lon;

    char _url[128];

    // =====================================
    // INTERNAL METHODS
    // =====================================

    void processContinuous();
    void processMock();
    void processReal();

public:

    // =====================================
    // CONSTRUCTOR
    // =====================================

    GPS_FSM(GPS* gps);

    // =====================================
    // SYSTEM
    // =====================================

    void begin();
    void update();

    // =====================================
    // STATE CONTROL
    // =====================================

    void setState(GPSState state);
    GPSState getState();

    // =====================================
    // MODULE CONTROL
    // =====================================

    void enable();
    void disable();

    bool isEnabled();

    // =====================================
    // DATA ACCESS
    // =====================================

    bool hasFix();

    bool isURLReady();

    const char* getURL();

    float getLat();
    float getLon();

    // =====================================
    // RESET
    // =====================================

    void reset();
};

#endif