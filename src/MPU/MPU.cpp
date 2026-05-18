// =====================================================
// FILE: src/MPU/MPU.cpp
// =====================================================

#include "MPU.h"

#define MPU_ADDR 0x68

MPU::MPU() {
  _enabled = true;
  _initialized = false;
  _ax = _ay = _az = 0;
  _lastRead = 0;
}

void MPU::begin() {
  if (!_enabled) return;

  Wire.begin();

  // Пробуждаем MPU6050 (по умолчанию он в sleep)
  writeRegister(0x6B, 0x00);

  _initialized = true;
}

void MPU::enable() {
  _enabled = true;
  begin();
}

void MPU::disable() {
  _enabled = false;
  _initialized = false;
}

bool MPU::isEnabled() {
  return _enabled;
}

void MPU::setEnabled(bool enabled) {
  if (enabled && !_enabled) enable();
  else if (!enabled && _enabled) disable();
}

void MPU::writeRegister(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void MPU::readRawData() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // начало данных акселерометра
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6, true);

  if (Wire.available() >= 6) {
    _ax = Wire.read() << 8 | Wire.read();
    _ay = Wire.read() << 8 | Wire.read();
    _az = Wire.read() << 8 | Wire.read();
  }
}

void MPU::update() {
  if (!_enabled || !_initialized) return;

  // читаем примерно каждые 500 мс
  if (millis() - _lastRead >= 500) {
    _lastRead = millis();
    readRawData();
  }
}

bool MPU::available() {
  return _initialized;
}

String MPU::getData() {
  if (!_enabled || !_initialized) return "";

  String data = "AX: " + String(_ax) +
                " AY: " + String(_ay) +
                " AZ: " + String(_az);

  return data;
}