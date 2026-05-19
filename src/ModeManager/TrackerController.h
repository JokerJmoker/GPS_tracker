// =====================================================
// FILE: FILE: src/ModeManager/TrackerController.h
// =====================================================

#pragma once

#include "GPS/GPS_FSM.h"
#include "GSM/GSM_FSM.h"
#include "MPU/MPU_FSM.h"

enum class TrackerState
{
    IDLE,

    GPS_ACTIVE,
    GPS_WAIT_FIX,

    GSM_ACTIVE,
    GSM_WAIT_RESPONSE,

    WAIT_NEXT_CYCLE,

    ERROR_STATE,

    SLEEP_LISTENING
};

class TrackerController
{
public:
    TrackerController(GPS_FSM*, GSM_FSM*, MPU_FSM*);

    void begin();
    void update();

private:

    GPS_FSM* _gps;
    GSM_FSM* _gsm;
    MPU_FSM* _mpu;

    TrackerState _state;

    // =========================
    // TIMERS
    // =========================

    unsigned long _gpsStartTime = 0;
    unsigned long _gsmStartTime = 0;
    unsigned long _cycleCooldownStart = 0;

    // =========================
    // CONFIG
    // =========================

    static constexpr unsigned long GPS_TIMEOUT = 15000;
    static constexpr unsigned long GSM_TIMEOUT = 20000;
    static constexpr unsigned long CYCLE_DELAY = 10000;

    // =========================
    // ANTI-SPAM FLAGS (IMPORTANT)
    // =========================

    bool _gpsLatch = false;
    bool _gsmLatch = false;

    // =========================
    // INTERNAL
    // =========================

    void processGPS();
    void processGSM();
    void processCooldown();

    void processSleepListening();

    void resetCycle();

    // =========================
    // CYCLE CONTROL (NEW)
    // =========================

    uint32_t _cycleCounter = 0;

    // dynamic cooldown per cycle
    unsigned long getCycleDelay() const;

    // watchdog
    unsigned long _cycleStartTime = 0;
    const unsigned long MAX_CYCLE_LIFETIME = 60000; // 60s hard reset
};