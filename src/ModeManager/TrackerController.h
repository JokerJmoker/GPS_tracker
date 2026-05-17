// =====================================================
// FILE: src/ModeManager/TrackerController.h
// =====================================================

#ifndef TRACKER_CONTROLLER_H
#define TRACKER_CONTROLLER_H

#pragma once

// =====================================================
// FSM
// =====================================================

#include "GPS/GPS_FSM.h"
#include "GSM/GSM_FSM.h"
#include "MPU/MPU_FSM.h"

// =====================================================
// TRACKER STATES
// =====================================================

enum class TrackerState
{
    IDLE,

    GPS_ACTIVE,
    GPS_WAIT_FIX,
    GPS_DONE,

    GSM_ACTIVE,
    GSM_SEND,
    GSM_WAIT_RESPONSE,
    GSM_DONE,

    SLEEP
};

// =====================================================
// TRACKER CONTROLLER
// =====================================================

class TrackerController
{
public:

    // =====================================
    // CONSTRUCTOR
    // =====================================

    TrackerController(
        GPS_FSM* gps,
        GSM_FSM* gsm,
        MPU_FSM* mpu
    );

    // =====================================
    // SYSTEM
    // =====================================

    void begin();
    void update();

    // =====================================
    // STATE
    // =====================================

    void setState(TrackerState state);

    TrackerState getState();

private:

    // =====================================
    // MODULE FSM
    // =====================================

    GPS_FSM* _gps;
    GSM_FSM* _gsm;
    MPU_FSM* _mpu;

    // =====================================
    // CURRENT STATE
    // =====================================

    TrackerState _state;

    // =====================================
    // INTERNAL FSM
    // =====================================

    void processGPS();
    void processGSM();
};

#endif