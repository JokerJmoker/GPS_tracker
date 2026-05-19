#pragma once

#include <Arduino.h>
#include "MPU.h"

// =====================================================
// STATES
// =====================================================

enum class MPUState
{
    DISABLED,
    LISTENING,
    AWAKEN
};

// =====================================================
// FSM
// =====================================================

class MPU_FSM
{
public:

    MPU_FSM(MPU* mpu);

    void begin();

    void update();

    void setState(MPUState state);

    MPUState getState();

    bool isAwaken();

private:

    MPU* _mpu;

    MPUState _state;

    // =====================================
    // POSITION TRACKING
    // =====================================

    int16_t _lastAX;
    int16_t _lastAY;
    int16_t _lastAZ;

    bool _hasInitialPosition;

    // sensitivity threshold
    int16_t _movementThreshold;

    bool detectMovement();
};