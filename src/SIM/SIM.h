#ifndef SIM_H
#define SIM_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SIM {
  private:
    SoftwareSerial* _simSerial;
    int _rxPin;
    int _txPin;
    bool _enabled;
    String _buffer;
    bool _hasNewData;

    void sendCommand(const String& cmd);

  public:
    SIM(int rxPin, int txPin);

    void begin(long baudrate = 9600);

    void enable();
    void disable();
    bool isEnabled();
    void setEnabled(bool enabled);

    void update();
    bool available();
    String getData();

    void test(); // отправка AT
};

#endif