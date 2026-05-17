#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
#include "FSM/TrackerFSM.h"
#include "Config.h"

// ============================================
// СОЗДАНИЕ ОБЪЕКТОВ МОДУЛЕЙ
// ============================================

// GPS модуль: RX на пине 3, TX на пине 4
NEO gps(3, 4);

// SIM800L модуль: RX на пине 5, TX на пине 6
SIM sim800l(5, 6);

// MPU6050 датчик движения/акселерометр
MPU mpu6050;

// Указатель на FSM
TrackerFSM* fsm = nullptr;

// ============================================
// НАСТРОЙКА
// ============================================
void setup() {
  Serial.begin(9600);
  
  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F("GPS Tracker System v2.0"));
  Serial.println(F("=========================================="));
  Serial.println();
  
  // Создаем FSM в зависимости от режима
  #ifdef MODE_DEBUG
    Serial.println(F("[MAIN] Initializing in DEBUG mode"));
    fsm = new TrackerFSM(&gps, &sim800l, &mpu6050);
    
  #elif defined(MODE_TRACKER)
    Serial.println(F("[MAIN] Initializing in TRACKER mode"));
    fsm = new TrackerFSM(&gps, &sim800l, &mpu6050);
    
  #elif defined(MODE_SLEEP)
    Serial.println(F("[MAIN] Initializing in SLEEP mode"));
    fsm = new TrackerFSM(&gps, &sim800l, &mpu6050);
    
  #else
    // Режим по умолчанию - DEBUG
    Serial.println(F("[MAIN] No mode defined, using DEBUG mode"));
    fsm = new TrackerFSM(&gps, &sim800l, &mpu6050);
  #endif
  
  // Запускаем FSM
  if (fsm != nullptr) {
    fsm->begin();
  } else {
    Serial.println(F("[MAIN] ERROR: Failed to create FSM!"));
  }
  
  Serial.println(F("[MAIN] System ready!"));
  Serial.println();
}

// ============================================
// ГЛАВНЫЙ ЦИКЛ
// ============================================
void loop() {
  // Все режимы теперь управляются через FSM
  if (fsm != nullptr) {
    fsm->update();
  } else {
    // Если FSM не создан, пробуем восстановиться
    Serial.println(F("[MAIN] FSM is null! Rebooting..."));
    delay(1000);
    setup();  // Попытка переинициализации
  }
  
  // Небольшая задержка для стабильности
  // В режиме SLEEP FSM сам управляет временем
  delay(10);
}