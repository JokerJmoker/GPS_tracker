// =====================================================
// FILE: src/GPS/GPSParser.h
// =====================================================

#pragma once
#include <Arduino.h>

bool parseRMC(const char* nmea, float &lat, float &lon);

void buildYandexURL(float lat, float lon, char* out, int size);