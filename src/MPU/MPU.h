#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <Wire.h>

class MPU {
  private:
    bool _enabled;
    bool _initialized;

    int16_t _ax, _ay, _az;
    unsigned long _lastRead;

    void writeRegister(uint8_t reg, uint8_t data);
    void readRawData();

  public:
    MPU();

    void begin();
    
    void enable();
    void disable();
    bool isEnabled();
    void setEnabled(bool enabled);

    void update();
    bool available();

    String getData();
};

#endif