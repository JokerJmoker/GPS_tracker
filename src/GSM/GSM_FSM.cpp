// =====================================================
// FILE: src/GSM/GSM_FSM.cpp
// =====================================================

#include "GSM_FSM.h"

// =====================================================
// CONSTRUCTOR
// =====================================================

GSM_FSM::GSM_FSM(GSM* gsm)
{
    _gsm = gsm;

    _state = GSMState::DISABLED;

    _enabled = false;

    _mock = true;

    _done = false;

    _url[0] = '\0';

    _t0 = 0;
}

// =====================================================
// BEGIN
// =====================================================

void GSM_FSM::begin()
{
    Serial.println(F("[GSM FSM] BEGIN"));
}

// =====================================================
// ENABLE
// =====================================================

void GSM_FSM::enable()
{
    if (_enabled)
        return;

    _gsm->begin(9600);

    _enabled = true;

    Serial.println(F("[GSM FSM] ENABLED"));
}

// =====================================================
// DISABLE
// =====================================================

void GSM_FSM::disable()
{
    if (!_enabled)
        return;

    _gsm->disable();

    _enabled = false;

    Serial.println(F("[GSM FSM] DISABLED"));
}

// =====================================================
// UPDATE
// =====================================================

void GSM_FSM::update()
{
    if (!_enabled)
        return;

    _gsm->update();

    switch (_state)
    {
        case GSMState::MOCK_SEND:
            sendMock();
            break;

        case GSMState::REAL_SEND:
            sendReal();
            break;

        case GSMState::WAIT_RESPONSE:

            if (_gsm->hasFullResponse())
            {
                Serial.println(F("[GSM FSM] RESPONSE OK"));

                Serial.println(_gsm->getFullResponse());

                _done = true;

                _state = GSMState::DONE;
            }

            break;

        default:
            break;
    }
}

// =====================================================
// MOCK SEND
// =====================================================

void GSM_FSM::sendMock()
{
    Serial.println();
    Serial.println(F("===== GSM MOCK SEND ====="));

    Serial.println(F("[GSM] AT"));
    Serial.println(F("OK"));

    Serial.println(F("[GSM] AT+CMGF=1"));
    Serial.println(F("OK"));

    Serial.println(F("[GSM] SMS SENT"));

    Serial.println(_url);

    Serial.println(F("========================="));
    Serial.println();

    _done = true;

    _state = GSMState::DONE;
}

// =====================================================
// REAL SEND
// =====================================================

void GSM_FSM::sendReal()
{
    Serial.println(F("[GSM FSM] REAL SEND"));

    _gsm->sendSMS("+79991234567", _url);

    _t0 = millis();

    _state = GSMState::WAIT_RESPONSE;
}

// =====================================================
// SET URL
// =====================================================

void GSM_FSM::setURL(const char* url)
{
    strncpy(_url, url, sizeof(_url));

    _url[sizeof(_url) - 1] = '\0';

    Serial.println(F("[GSM FSM] URL RECEIVED"));
}

// =====================================================
// SET MODE
// =====================================================

void GSM_FSM::setMode(bool mock)
{
    _mock = mock;

    if (_mock)
    {
        _state = GSMState::MOCK_SEND;
    }
    else
    {
        _state = GSMState::REAL_SEND;
    }
}

// =====================================================
// SET STATE
// =====================================================

void GSM_FSM::setState(GSMState state)
{
    _state = state;
}

// =====================================================
// GET STATE
// =====================================================

GSMState GSM_FSM::getState()
{
    return _state;
}

// =====================================================
// GETTERS
// =====================================================

bool GSM_FSM::isEnabled()
{
    return _enabled;
}

bool GSM_FSM::isDone()
{
    return _done;
}

// =====================================================
// RESET
// =====================================================

void GSM_FSM::reset()
{
    _done = false;

    _url[0] = '\0';

    _state = GSMState::IDLE;

    Serial.println(F("[GSM FSM] RESET"));
}