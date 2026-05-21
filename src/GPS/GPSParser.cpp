// =====================================================
// FILE: src/GPS/GPSParser.cpp (ОПТИМИЗИРОВАННЫЙ)
// =====================================================

#include "GPSParser.h"
#include <string.h>
#include <stdlib.h>

bool parseRMC(const char* nmea, float &lat, float &lon) {
    if (strncmp(nmea, "$GNRMC", 6) != 0 && strncmp(nmea, "$GPRMC", 6) != 0) {
        return false;
    }
    
    // ОПТИМИЗАЦИЯ: разбираем на лету, без копирования
    const char* ptr = nmea;
    int commaCount = 0;
    const char* status = nullptr;
    const char* latStr = nullptr;
    const char* latDir = nullptr;
    const char* lonStr = nullptr;
    const char* lonDir = nullptr;
    
    // Проходим по строке, считая запятые
    while (*ptr && *ptr != '\0' && *ptr != '\r' && *ptr != '\n') {
        if (*ptr == ',') {
            commaCount++;
            ptr++;
            continue;
        }
        
        switch (commaCount) {
            case 2: status = ptr; while (*ptr && *ptr != ',') ptr++; continue;
            case 3: latStr = ptr; while (*ptr && *ptr != ',') ptr++; continue;
            case 4: latDir = ptr; while (*ptr && *ptr != ',') ptr++; continue;
            case 5: lonStr = ptr; while (*ptr && *ptr != ',') ptr++; continue;
            case 6: lonDir = ptr; while (*ptr && *ptr != ',') ptr++; continue;
            default: break;
        }
        ptr++;
    }
    
    if (!status || status[0] != 'A') return false;
    if (!latStr || !lonStr) return false;
    
    // Парсим числа (временно копируем только цифры)
    char temp[16];
    
    // Парсим latitude
    const char* p = latStr;
    uint8_t i = 0;
    while (*p && *p != ',' && i < sizeof(temp)-1) temp[i++] = *p++;
    temp[i] = '\0';
    float latRaw = strtod(temp, nullptr);
    
    // Парсим longitude
    p = lonStr;
    i = 0;
    while (*p && *p != ',' && i < sizeof(temp)-1) temp[i++] = *p++;
    temp[i] = '\0';
    float lonRaw = strtod(temp, nullptr);
    
    int latDeg = (int)(latRaw / 100);
    float latMin = latRaw - latDeg * 100;
    
    int lonDeg = (int)(lonRaw / 100);
    float lonMin = lonRaw - lonDeg * 100;
    
    lat = latDeg + latMin / 60.0;
    lon = lonDeg + lonMin / 60.0;
    
    if (latDir && latDir[0] == 'S') lat = -lat;
    if (lonDir && lonDir[0] == 'W') lon = -lon;
    
    return true;
}

void buildYandexURL(float lat, float lon, char* out, int size) {
    // ОПТИМИЗАЦИЯ: прямой вывод через dtostrf
    char latBuf[12];
    char lonBuf[12];
    
    dtostrf(lat, 0, 6, latBuf);
    dtostrf(lon, 0, 6, lonBuf);
    
    snprintf(out, size,
        "https://yandex.ru/maps/?pt=%s,%s&z=17",  // Короткий URL!
        lonBuf,
        latBuf
    );
}