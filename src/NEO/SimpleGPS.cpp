#include "SimpleGPS.h"

// ============================================
// КОНСТРУКТОР
// ============================================
SimpleGPS::SimpleGPS() {
  reset();
}

// ============================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// ============================================

String SimpleGPS::getField(const String& nmea, int fieldIndex) {
  int start = 0;
  int end = 0;
  int currentField = 0;
  
  for (size_t i = 0; i < nmea.length(); i++) {
    if (nmea[i] == ',' || nmea[i] == '*') {
      if (currentField == fieldIndex) {
        return nmea.substring(start, i);
      }
      start = i + 1;
      currentField++;
    }
  }
  
  return "";
}

double SimpleGPS::convertToDecimal(const String& coord, const String& direction) {
  if (coord.length() == 0) return 0.0;
  
  // Координата в формате: DDMM.MMMMM
  // Где DD - градусы, MM.MMMMM - минуты
  int degrees = coord.substring(0, 2).toInt();
  float minutes = coord.substring(2).toFloat();
  
  double decimal = degrees + (minutes / 60.0);
  
  // Южная широта или западная долгота - отрицательные значения
  if (direction == "S" || direction == "W") {
    decimal = -decimal;
  }
  
  return decimal;
}

// ============================================
// ПАРСИНГ GGA СТРОКИ ($GPGGA)
// ============================================
// Формат: $GPGGA,time,lat,NS,lon,EW,quality,sats,hdop,alt,M,...
void SimpleGPS::parseGGA(const String& nmea) {
  // Время (поле 1) - HHMMSS.SS
  String rawTime = getField(nmea, 1);
  if (rawTime.length() >= 6) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d",
            rawTime.substring(0, 2).toInt(),
            rawTime.substring(2, 4).toInt(),
            rawTime.substring(4, 6).toInt());
    _time = String(timeStr);
  }
  
  // Широта (поле 2) и направление (поле 3)
  String latStr = getField(nmea, 2);
  String latDir = getField(nmea, 3);
  if (latStr.length() > 0 && latDir.length() > 0) {
    _latitude = convertToDecimal(latStr, latDir);
  }
  
  // Долгота (поле 4) и направление (поле 5)
  String lonStr = getField(nmea, 4);
  String lonDir = getField(nmea, 5);
  if (lonStr.length() > 0 && lonDir.length() > 0) {
    _longitude = convertToDecimal(lonStr, lonDir);
  }
  
  // Качество фикса (поле 6)
  int quality = getField(nmea, 6).toInt();
  
  // Количество спутников (поле 7)
  String satsStr = getField(nmea, 7);
  if (satsStr.length() > 0) {
    _satellites = satsStr.toInt();
  }
  
  // HDOP (поле 8)
  String hdopStr = getField(nmea, 8);
  if (hdopStr.length() > 0) {
    _hdop = hdopStr.toFloat();
  }
  
  // Высота (поле 9)
  String altStr = getField(nmea, 9);
  if (altStr.length() > 0) {
    _altitude = altStr.toFloat();
  }
  
  // Фикс есть, если качество > 0
  _hasFix = (quality > 0);
}

// ============================================
// ПАРСИНГ RMC СТРОКИ ($GPRMC)
// ============================================
// Формат: $GPRMC,time,status,lat,NS,lon,EW,speed,course,date,...
void SimpleGPS::parseRMC(const String& nmea) {
  // Статус (поле 2) - A=активен, V=невалиден
  String status = getField(nmea, 2);
  _hasFix = (status == "A");
  
  // Время (поле 1)
  if (!_hasFix) return;
  
  String rawTime = getField(nmea, 1);
  if (rawTime.length() >= 6 && _time.length() == 0) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d",
            rawTime.substring(0, 2).toInt(),
            rawTime.substring(2, 4).toInt(),
            rawTime.substring(4, 6).toInt());
    _time = String(timeStr);
  }
  
  // Широта (поле 3) и направление (поле 4)
  String latStr = getField(nmea, 3);
  String latDir = getField(nmea, 4);
  if (latStr.length() > 0 && latDir.length() > 0 && _latitude == 0.0) {
    _latitude = convertToDecimal(latStr, latDir);
  }
  
  // Долгота (поле 5) и направление (поле 6)
  String lonStr = getField(nmea, 5);
  String lonDir = getField(nmea, 6);
  if (lonStr.length() > 0 && lonDir.length() > 0 && _longitude == 0.0) {
    _longitude = convertToDecimal(lonStr, lonDir);
  }
  
  // Скорость в узлах (поле 7)
  String speedStr = getField(nmea, 7);
  if (speedStr.length() > 0) {
    _speed = speedStr.toFloat() * 1.852; // Конвертация узлов в км/ч
  }
  
  // Дата (поле 9) - DDMMYY
  String rawDate = getField(nmea, 9);
  if (rawDate.length() == 6 && _date.length() == 0) {
    char dateStr[11];
    sprintf(dateStr, "%02d/%02d/%02d",
            rawDate.substring(0, 2).toInt(),
            rawDate.substring(2, 4).toInt(),
            rawDate.substring(4, 6).toInt());
    _date = String(dateStr);
  }
}

// ============================================
// ОСНОВНОЙ МЕТОД ПАРСИНГА
// ============================================
bool SimpleGPS::parse(const String& nmeaData) {
  // Проверка на пустую строку
  if (nmeaData.length() == 0 || nmeaData[0] != '$') {
    return false;
  }
  
  // Определяем тип NMEA сообщения
  if (nmeaData.startsWith("$GPGGA") || nmeaData.startsWith("$GNGGA")) {
    parseGGA(nmeaData);
  } 
  else if (nmeaData.startsWith("$GPRMC") || nmeaData.startsWith("$GNRMC")) {
    parseRMC(nmeaData);
  }
  
  return _hasFix;
}

// ============================================
// ПРОВЕРКА НАЛИЧИЯ СИГНАЛА
// ============================================
bool SimpleGPS::hasValidFix() {
  return _hasFix && _satellites >= 3;
}

// ============================================
// ГЕТТЕРЫ
// ============================================
double SimpleGPS::getLatitude() { return _latitude; }
double SimpleGPS::getLongitude() { return _longitude; }
float SimpleGPS::getAltitude() { return _altitude; }
float SimpleGPS::getSpeed() { return _speed; }
float SimpleGPS::getSpeedKnots() { return _speed / 1.852; }
int SimpleGPS::getSatellites() { return _satellites; }
float SimpleGPS::getHdop() { return _hdop; }
String SimpleGPS::getTime() { return _time; }
String SimpleGPS::getDate() { return _date; }

// ============================================
// ФОРМАТИРОВАННЫЙ ВЫВОД
// ============================================
String SimpleGPS::getFormattedData() {
  if (!hasValidFix()) {
    return "[GPS] Waiting for fix... (Satellites: " + String(_satellites) + ")";
  }
  
  String result = "";
  result += "📍 Location: " + String(_latitude, 6) + ", " + String(_longitude, 6);
  result += " | 🛰️ Sat: " + String(_satellites);
  
  if (_hdop > 0) {
    result += " | 📡 HDOP: " + String(_hdop, 1);
  }
  
  result += "\n";
  result += "   🏔️ Alt: " + String(_altitude, 1) + "m";
  result += " | 🚗 Speed: " + String(_speed, 1) + " km/h";
  
  if (_time.length() > 0) {
    result += " | 🕐 Time: " + _time;
  }
  
  return result;
}

String SimpleGPS::getCSVData() {
  if (!hasValidFix()) {
    return "NO_FIX," + String(_satellites) + ",,,,,";
  }
  
  String result = "";
  result += String(_latitude, 6) + ",";
  result += String(_longitude, 6) + ",";
  result += String(_altitude, 1) + ",";
  result += String(_speed, 1) + ",";
  result += String(_satellites) + ",";
  result += String(_hdop, 1) + ",";
  result += _time + "," + _date;
  
  return result;
}

// ============================================
// СБРОС ДАННЫХ
// ============================================
void SimpleGPS::reset() {
  _latitude = 0.0;
  _longitude = 0.0;
  _altitude = 0.0;
  _speed = 0.0;
  _satellites = 0;
  _hdop = 0.0;
  _time = "";
  _date = "";
  _hasFix = false;
}