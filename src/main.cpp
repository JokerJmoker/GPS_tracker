#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
#include "TestManager/TestManager.h"

// Создаем объекты модулей
NEO gps(3, 4);
SIM sim800l(5, 6);
MPU mpu6050;

// Создаем менеджер тестирования
TestManager testManager(&gps, &sim800l, &mpu6050);

void setup() {
  Serial.begin(9600);
  testManager.begin();
}

void loop() {
  testManager.update();
}