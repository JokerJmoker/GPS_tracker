#include "GPSParser.h"
#include <string.h>
#include <stdlib.h>

bool parseRMC(const char* nmea, float &lat, float &lon) {

    if (strncmp(nmea, "$GNRMC", 6) != 0) return false;

    char buffer[120];
    strncpy(buffer, nmea, sizeof(buffer));
    buffer[sizeof(buffer)-1] = '\0';

    char* token = strtok(buffer, ",");

    int i = 0;

    char* status = nullptr;
    char* latStr = nullptr;
    char* latDir = nullptr;
    char* lonStr = nullptr;
    char* lonDir = nullptr;

    while (token) {

        switch(i) {
            case 2: status = token; break;
            case 3: latStr = token; break;
            case 4: latDir = token; break;
            case 5: lonStr = token; break;
            case 6: lonDir = token; break;
        }

        token = strtok(NULL, ",");
        i++;
    }

    if (!status || status[0] != 'A') return false;
    if (!latStr || !lonStr) return false;

    float latRaw = atof(latStr);
    float lonRaw = atof(lonStr);

    int latDeg = latRaw / 100;
    float latMin = latRaw - latDeg * 100;

    int lonDeg = lonRaw / 100;
    float lonMin = lonRaw - lonDeg * 100;

    lat = latDeg + latMin / 60.0;
    lon = lonDeg + lonMin / 60.0;

    if (latDir && latDir[0] == 'S') lat = -lat;
    if (lonDir && lonDir[0] == 'W') lon = -lon;

    return true;
}

void buildYandexURL(float lat, float lon, char* out, int size) {
    snprintf(out, size,
        "https://yandex.ru/maps/?pt=%.6f,%.6f&z=18",
        lon, lat
    );
}