#include <Arduino.h>
#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
#include "TestManager/TestManager.h"
#include "ModeManager/SystemModes.h"
#include "ModeManager/TrackerController.h"

// ============================================
// ВЫБОР РЕЖИМА РАБОТЫ (РАСКОММЕНТИРОВАТЬ НУЖНЫЙ)
// ============================================
//#define MODE_DEBUG        // Режим отладки (активен по умолчанию)
//#define MODE_TRACKER      // Режим трекера (GPS->SIM циклично)
#define MODE_SLEEP        // Режим сна (пробуждение от MPU)
// ============================================

// Создаем объекты модулей
NEO gps(3, 4);
SIM sim800l(5, 6);
MPU mpu6050;

// Создаем контроллер трекера
TrackerController tracker(&gps, &sim800l, &mpu6050);

// Объект для тестирования (только в DEBUG режиме)
TestManager* testManager = nullptr;

void setup() {
  Serial.begin(9600);
  
  // Инициализация системы режимов
  SystemModes::begin();
  
  // Настройка режима
  #ifdef MODE_DEBUG
    SystemModes::setMode(OperationMode::DEBUG_MODE);
    testManager = new TestManager(&gps, &sim800l, &mpu6050);
    testManager->begin();
    
  #elif defined(MODE_TRACKER)
    SystemModes::setMode(OperationMode::TRACKER_MODE);
    // В режиме трекера отключаем всё лишнее
    mpu6050.disable();
    tracker.begin();
    
    // Настройка начального состояния
    if (SystemModes::shouldGPSBeActive()) {
      gps.begin(9600);
      sim800l.disable();
    } else if (SystemModes::shouldSIMBeActive()) {
      sim800l.begin(9600);
      gps.disable();
    }
    
  #elif defined(MODE_SLEEP)
    SystemModes::setMode(OperationMode::SLEEP_MODE);
    tracker.begin();
    // В режиме сна все модули отключаются
    // MPU будет в режиме обнаружения движения
    gps.disable();
    sim800l.disable();
    mpu6050.begin();
  #endif
  
  Serial.println("System ready!");
}

void loop() {
  OperationMode currentMode = SystemModes::getCurrentMode();
  
  // ============================================
  // РЕЖИМ 1: ОТЛАДКА (всё работает постоянно)
  // ============================================
  #ifdef MODE_DEBUG
    if (testManager != nullptr) {
      testManager->update();
    }
    delay(10);
  
  // ============================================
  // РЕЖИМ 2: ТРЕКЕР (цикл GPS -> SIM)
  // ============================================
  #elif defined(MODE_TRACKER)
    
    // Управление GPS
    if (SystemModes::shouldGPSBeActive()) {
      if (!gps.isEnabled()) {
        gps.enable();
        sim800l.disable();
        Serial.println("[Tracker] GPS active, SIM sleeping");
      }
      tracker.update();
    }
    
    // Управление SIM
    if (SystemModes::shouldSIMBeActive()) {
      if (!sim800l.isEnabled()) {
        sim800l.enable();
        gps.disable();
        Serial.println("[Tracker] SIM active, GPS sleeping");
      }
      tracker.update();
    }
    
    // Проверка завершения цикла
    if (SystemModes::isTrackerCycleComplete()) {
      Serial.println("[Tracker] Cycle complete - resetting for next round");
      SystemModes::resetTrackerCycle();
      
      // Сброс флагов в контроллере
      // Принудительно обновляем для нового цикла
      delay(1000);
    }
    
    delay(100);
  
  // ============================================
  // РЕЖИМ 3: СОН (пробуждение от MPU)
  // ============================================
  #elif defined(MODE_SLEEP)
    
    if (SystemModes::getCurrentMode() == OperationMode::SLEEP_MODE) {
      // Если цикл завершен - уходим в сон
      if (SystemModes::isTrackerCycleComplete()) {
        SystemModes::resetTrackerCycle();
        SystemModes::goToSleep();  // Уход в сон до прерывания
      }
      
      // Проснулись - выполняем один цикл
      if (SystemModes::shouldGPSBeActive()) {
        if (!gps.isEnabled()) {
          gps.enable();
          sim800l.disable();
        }
        tracker.update();
      }
      
      if (SystemModes::shouldSIMBeActive()) {
        if (!sim800l.isEnabled()) {
          sim800l.enable();
          gps.disable();
        }
        tracker.update();
      }
      
      // После выполнения цикла будет уход в сон на следующей итерации
    }
    
    delay(100);
    
  #endif
}