// =====================================================
// FILE: src/MPU/MPU_FSM.cpp
// =====================================================

#include "MPU_FSM.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

MPU_FSM::MPU_FSM(MPU* mpu)
{
    _mpu = mpu;

    _state = MPUState::DISABLED;
}

// =====================================================
// BEGIN
// =====================================================

void MPU_FSM::begin()
{
    if (_mpu != nullptr)
    {
        _mpu->begin();
    }

    _state = MPUState::ACTIVE;

    Serial.println(F("[MPU FSM] BEGIN"));
}

// =====================================================
// SET STATE
// =====================================================

void MPU_FSM::setState(MPUState state)
{
    _state = state;
}

// =====================================================
// GET STATE
// =====================================================

MPUState MPU_FSM::getState()
{
    return _state;
}

// =====================================================
// UPDATE
// =====================================================

void MPU_FSM::update()
{
    if (_mpu == nullptr)
    {
        return;
    }

    switch (_state)
    {
        case MPUState::ACTIVE:

            _mpu->update();

            break;

        case MPUState::SLEEP:

            // sleep logic later

            break;

        case MPUState::DISABLED:
        default:

            break;
    }
}