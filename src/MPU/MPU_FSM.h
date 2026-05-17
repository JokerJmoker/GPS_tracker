#pragma once

enum class MPUState
{
    DISABLED,
    ACTIVE,
    SLEEP
};

class MPU_FSM
{
public:
    void setState(MPUState state);
    void update();

private:
    MPUState _state;
};