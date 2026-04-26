#include "SIM.h"

SIM::SIM(int rxPin, int txPin) {
  _rxPin = rxPin;
  _txPin = txPin;
  _simSerial = new SoftwareSerial(rxPin, txPin);
  _enabled = true;
  _buffer = "";
  _hasNewData = false;
}

void SIM::begin(long baudrate) {
  if (_enabled) {
    _simSerial->begin(baudrate);
  }
}

void SIM::enable() {
  _enabled = true;
  _simSerial->begin(9600);
}

void SIM::disable() {
  _enabled = false;
  _simSerial->end();
  _buffer = "";
  _hasNewData = false;
}

bool SIM::isEnabled() {
  return _enabled;
}

void SIM::setEnabled(bool enabled) {
  if (enabled && !_enabled) enable();
  else if (!enabled && _enabled) disable();
}

bool SIM::available() {
  if (!_enabled) return false;
  return _simSerial->available();
}

void SIM::update() {
  if (!_enabled) return;

  while (_simSerial->available()) {
    char c = _simSerial->read();

    if (c == '\n') {
      _hasNewData = true;
    } else if (c != '\r') {
      _buffer += c;
    }
  }
}

String SIM::getData() {
  if (!_enabled) return "";

  if (_hasNewData) {
    _hasNewData = false;
    String temp = _buffer;
    _buffer = "";
    return temp;
  }
  return "";
}

void SIM::sendCommand(const String& cmd) {
  if (!_enabled) return;
  _simSerial->println(cmd);
}

void SIM::test() {
  sendCommand("AT");
}