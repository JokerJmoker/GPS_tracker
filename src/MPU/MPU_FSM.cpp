#include "MPU_FSM.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

MPU_FSM::MPU_FSM(MPU* mpu)
{
    _mpu = mpu;

    _state = MPUState::DISABLED;

    _lastAX = 0;
    _lastAY = 0;
    _lastAZ = 0;

    _hasInitialPosition = false;

    // sensitivity
    _movementThreshold = 2000;
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

    _state = MPUState::LISTENING;

    Serial.println(F("[MPU FSM] BEGIN"));
    Serial.println(F("[MPU FSM] STATE -> LISTENING"));
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
// IS AWAKEN
// =====================================================

bool MPU_FSM::isAwaken()
{
    return (_state == MPUState::AWAKEN);
}

// =====================================================
// MOVEMENT DETECTION
// =====================================================

bool MPU_FSM::detectMovement()
{
    int16_t currentAX = _mpu->getAX();
    int16_t currentAY = _mpu->getAY();
    int16_t currentAZ = _mpu->getAZ();

    // first calibration
    if (!_hasInitialPosition)
    {
        _lastAX = currentAX;
        _lastAY = currentAY;
        _lastAZ = currentAZ;

        _hasInitialPosition = true;

        return false;
    }

    int16_t deltaX = abs(currentAX - _lastAX);
    int16_t deltaY = abs(currentAY - _lastAY);
    int16_t deltaZ = abs(currentAZ - _lastAZ);

    // update baseline
    _lastAX = currentAX;
    _lastAY = currentAY;
    _lastAZ = currentAZ;

    if (
        deltaX > _movementThreshold ||
        deltaY > _movementThreshold ||
        deltaZ > _movementThreshold
    )
    {
        Serial.println(F("[MPU FSM] MOVEMENT DETECTED"));

        Serial.print(F("[DELTA X] "));
        Serial.println(deltaX);

        Serial.print(F("[DELTA Y] "));
        Serial.println(deltaY);

        Serial.print(F("[DELTA Z] "));
        Serial.println(deltaZ);

        return true;
    }

    return false;
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
        // =====================================
        // LISTENING
        // =====================================

        case MPUState::LISTENING:

            _mpu->update();

            if (_mpu->available())
            {
                if (detectMovement())
                {
                    _state = MPUState::AWAKEN;

                    Serial.println(F("[MPU FSM] STATE -> AWAKEN"));
                }
            }

            break;

        // =====================================
        // AWAKEN
        // =====================================

        case MPUState::AWAKEN:

            // waiting external system handling

            break;

        // =====================================
        // DISABLED
        // =====================================

        case MPUState::DISABLED:
        default:

            break;
    }
}