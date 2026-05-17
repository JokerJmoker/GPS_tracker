//TestManager.cpp
#include "TestManager.h"

TestManager::TestManager(NEO* gps, SIM* sim800l, MPU* mpu6050) {

    _gps = gps;
    _sim800l = sim800l;
    _mpu6050 = mpu6050;

    _lastStatus = 0;
    _lastPrint = 0;
    _lastVoltageCheck = 0;
    _lastMpuPrint = 0;
    _cmdIndex = 0;
    _gpsHasFix = false;
    _pipelineState = WAIT_GPS;
    _gpsUrl[0] = '\0';
}

// =====================================================
// HEADER
// =====================================================

void TestManager::printHeader() {

    Serial.println();
    Serial.println(F("=========================================="));
    Serial.println(F(">>> MODE: DEBUG (All modules active) <<<"));
    Serial.println(F("=========================================="));
    Serial.println();
}

// =====================================================
// INIT MODULES
// =====================================================

void TestManager::initModules() {

#ifdef TEST_GPS
    Serial.println(F("GPS module ENABLED"));
    _gps->begin(9600);
#else
    _gps->disable();
#endif

#ifdef TEST_SIM800L
    Serial.println(F("SIM800L module ENABLED"));
    _sim800l->begin(9600);
#else
    _sim800l->disable();
#endif

#ifdef TEST_MPU6050
    Serial.println(F("MPU6050 module ENABLED"));
    _mpu6050->begin();
#else
    _mpu6050->disable();
#endif

#ifdef TEST_EEPROM
    Serial.println(F("EEPROM module ENABLED"));
#endif

    Serial.println();
}

// =====================================================
// UPDATE MODULES
// =====================================================

void TestManager::updateModules() {

#ifdef TEST_GPS
    if (_pipelineState == WAIT_GPS || !TEST_PIPELINE_MODE)
        _gps->update();
#endif

#ifdef TEST_SIM800L
    // ВСЕГДА читаем UART
    _sim800l->update();
#endif

#ifdef TEST_MPU6050
    _mpu6050->update();
#endif
}

// =====================================================
// GPS PROCESS
// =====================================================

void TestManager::processGPS() {

#ifdef TEST_GPS

    // обновляем GPS
    //_gps->update();

    // =========================================
    // MOCK MODE
    // =========================================
#if GPS_MOCK_MODE == 1

    static uint32_t lastMock = 0;

    if (millis() - lastMock < 2000) {
        return;
    }

    lastMock = millis();

    const char* data =
        "$GNRMC,123519,A,5545.9469,N,3741.1349,E,0.13,309.62,150526,,,A*7C";

    // ===== ВЫВОД ИСХОДНОЙ СТРОКИ =====
    Serial.println();
    Serial.println("===== INPUT NMEA =====");
    Serial.println(data);
    Serial.println("======================");

    float lat = 0;
    float lon = 0;

    if (parseRMC(data, lat, lon)) {

        buildYandexURL(lat, lon, _gpsUrl, sizeof(_gpsUrl));

        _gpsHasFix = true;
        _pipelineState = SEND_SMS;

        Serial.println();
        Serial.println("===== GPS PARSER TEST =====");

        Serial.print("LAT: ");
        Serial.println(lat, 6);

        Serial.print("LON: ");
        Serial.println(lon, 6);

        Serial.print("URL: ");
        Serial.println(_gpsUrl);

        Serial.println("===========================");
        Serial.println();
    }
    else {

        Serial.println("[GPS] parse failed");
        _gpsHasFix = false;  // Сбрасываем флаг
    }

    return;

    // =========================================
    // REAL GPS + PARSER MODE
    // =========================================
#elif GPS_MOCK_MODE == 0

    const char* data = _gps->getRawData();

    if (data == nullptr || data[0] != '$') {
        return;
    }

    // =========================================
    // общий фильтр RMC (оставляем только нужное)
    // =========================================
    if (strncmp(data, "$GNRMC", 6) != 0 &&
        strncmp(data, "$GPRMC", 6) != 0) {
        return;
    }

    // =========================================
    // ПЕЧАТЬ ВХОДА (одинаково для всех режимов)
    // =========================================
    Serial.println();
    Serial.println("===== INPUT RMC =====");
    Serial.println(data);
    Serial.println("=====================");

    // =========================================
    // ПАРСИНГ (единый буфер обработки)
    // =========================================
    float lat = 0;
    float lon = 0;

    bool ok = parseRMC(data, lat, lon);
    

    // =========================================
    // ЕСЛИ FIX ЕСТЬ
    // =========================================
    if (ok) {

        buildYandexURL(lat, lon, _gpsUrl, sizeof(_gpsUrl));

        _gpsHasFix = true;
        _pipelineState = SEND_SMS;

        Serial.println("===== GPS RESULT =====");

        Serial.print("LAT: ");
        Serial.println(lat, 6);

        Serial.print("LON: ");
        Serial.println(lon, 6);

        Serial.print("URL: ");
        Serial.println(_gpsUrl);

        Serial.println("[GPS] FIX OK");
        Serial.println();
    }

    // =========================================
    // ЕСЛИ FIX НЕТ
    // =========================================
    else {

        Serial.println("[GPS] NO FIX");
        Serial.println("[GPS] Waiting satellites...");
        Serial.println();
    }

    return;

#else

    const char* data = _gps->getRawData();

    if (data && data[0] == '$') {
        Serial.println(data);
    }

    return;

#endif

#endif
}

// =====================================================
// SIM PROCESS
// =====================================================
void TestManager::processSIM() {

#ifdef TEST_SIM800L

    static unsigned long lastCommand = 0;
    static unsigned long waitStart = 0;

    static int step = 0;
    static bool waitingResponse = false;

    // ==============================
    // GPS → SIM COMMON BUFFER (MOCK)
    // ==============================
#if GPS_MOCK_MODE == 1

    static bool sms_sent = false;
// Проверяем наличие GPS fix и что SMS еще не отправлен
    if (!_gpsHasFix || sms_sent) {
        return;
    }

    Serial.println();
    Serial.println("===== GSM MODULE ACTIVE =====");
    Serial.println("[SIM] GPS DATA RECEIVED");
    Serial.println("[SIM] Preparing SMS...");
    Serial.println();

    Serial.println("[SIM] AT");
    Serial.println("OK");

    Serial.println("[SIM] AT+CMGF=1");
    Serial.println("OK");

    Serial.println("[SIM] AT+CMGS=\"+79991234567\"");
    Serial.println(">");

    Serial.print("[SIM] MESSAGE: ");
    Serial.println(_gpsUrl);

    Serial.println((char)26);
    Serial.println("OK");

    Serial.println("[SIM] SMS SENT (MOCK)");

    sms_sent = true;
    _gpsHasFix = false;
    // ВОЗВРАТ К GPS
    _pipelineState = WAIT_GPS;

    Serial.println("[PIPELINE] RETURN TO GPS");

    return;

#endif

    // ==============================
    // REAL SIM800L STATE MACHINE
    // ==============================

    const unsigned long COMMAND_DELAY = 2000;
    const unsigned long RESPONSE_TIMEOUT = 30000;

    //_sim800l->update();

    // =====================================
    // FULL RESPONSE
    // =====================================

    String fullResponse = _sim800l->getFullResponse();

    if (fullResponse.length() > 0) {

        Serial.println(F("[SIM] Full response:"));
        Serial.println(fullResponse);

        if (waitingResponse) {

            waitingResponse = false;
            lastCommand = millis();

            if (fullResponse.indexOf("ERROR") >= 0) {
                Serial.println(F("[SIM] COMMAND FAILED"));
            } else {
                Serial.println(F("[SIM] COMMAND OK"));
            }

            step++;
        }
    }

    // =====================================
    // LINE DEBUG
    // =====================================

    String line;

    while ((line = _sim800l->getData()).length() > 0) {
        Serial.print(F("[SIM] Line: "));
        Serial.println(line);
    }

    // =====================================
    // TIMEOUT
    // =====================================

    if (waitingResponse &&
        millis() - waitStart >= RESPONSE_TIMEOUT) {

        Serial.println(F("[SIM] RESPONSE TIMEOUT"));

        waitingResponse = false;
        lastCommand = millis();

        const char* failedCommands[] = {
            "AT+CFUN=1",
            "AT",
            "AT+CPIN?",
            "AT+CCID",
            "AT+CREG=1",
            "AT+CREG?",
            "AT+COPS=0",
            "AT+COPS?",
            "AT+CSQ",
            "AT+CBC"
        };

        Serial.print(F("[SIM] SKIPPING: "));
        Serial.println(failedCommands[step]);

        step++;
    }

    // =====================================
    // SEND COMMANDS
    // =====================================

    if (!waitingResponse &&
        millis() - lastCommand >= COMMAND_DELAY) {

        const char* commands[] = {

            "AT+CFUN=1",
            "AT",
            "AT+CPIN?",
            "AT+CCID",
            "AT+CREG=1",
            "AT+CREG?",
            "AT+COPS=0",
            "AT+COPS?",
            "AT+CSQ",
            "AT+CBATCHK=1",
            "AT+CBC"
        };

        const int COMMAND_COUNT =
            sizeof(commands) / sizeof(commands[0]);

        if (step < COMMAND_COUNT) {

            Serial.print(F("[SIM] Sending: "));
            Serial.println(commands[step]);

            _sim800l->sendCommand(commands[step]);

            waitingResponse = true;
            waitStart = millis();
        }
        else {

            Serial.println(F("[SIM] TEST COMPLETE"));

            step = 0;
            _gpsHasFix = false;

            // ВОЗВРАТ К GPS
            _pipelineState = WAIT_GPS;

            Serial.println(F("[PIPELINE] RETURN TO GPS"));
            delay(5000);
        }
    }

#endif
}

// =====================================================
// MPU PROCESS
// =====================================================

void TestManager::processMPU() {

#ifdef TEST_MPU6050

    if (millis() - _lastMpuPrint >= 1000) {

        _lastMpuPrint = millis();

        if (_mpu6050->available()) {

            Serial.print(F("[MPU] "));
            Serial.println(_mpu6050->getData());
        }
        else {

            Serial.println(F("[MPU] Not initialized"));
        }
    }

#endif
}

// =====================================================
// BEGIN
// =====================================================

void TestManager::begin() {

    printHeader();
    initModules();
}

// =====================================================
// UPDATE
// =====================================================

void TestManager::update() {

    updateModules();

    // =====================================
    // DEBUG PIPELINE STATE
    // =====================================

    static unsigned long lastDebug = 0;

    if (millis() - lastDebug >= 1000) {

        lastDebug = millis();

        Serial.println();
        Serial.println(F("========== PIPELINE DEBUG =========="));

        // ---------------------------------
        // STATE
        // ---------------------------------

        Serial.print(F("[STATE] "));

        switch (_pipelineState)
        {
            case WAIT_GPS:
                Serial.println(F("WAIT_GPS"));
                break;

            case SEND_SMS:
                Serial.println(F("SEND_SMS"));
                break;

            default:
                Serial.println(F("UNKNOWN"));
                break;
        }

        // ---------------------------------
        // GPS FIX
        // ---------------------------------

        Serial.print(F("[GPS FIX] "));
        Serial.println(_gpsHasFix ? F("TRUE") : F("FALSE"));

        // ---------------------------------
        // URL CONTENT
        // ---------------------------------

        Serial.print(F("[URL] "));
        Serial.println(_gpsUrl);

        // ---------------------------------
        // URL LENGTH
        // ---------------------------------

        Serial.print(F("[URL LEN] "));
        Serial.println(strlen(_gpsUrl));

        // ---------------------------------
        // MEMORY ADDRESS
        // ---------------------------------

        Serial.print(F("[URL ADDR] 0x"));
        Serial.println((uintptr_t)_gpsUrl, HEX);

        Serial.println(F("===================================="));
        Serial.println();
    }

#if TEST_PIPELINE_MODE

    switch (_pipelineState)
    {
        case WAIT_GPS:
            processGPS();
            break;

        case SEND_SMS:
            processSIM();
            break;
    }

#else

    processGPS();
    processSIM();
    processMPU();

#endif
}