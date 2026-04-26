#include <Arduino.h>

#include "NEO/NEO.h"
#include "SIM/SIM.h"
#include "MPU/MPU.h"
// ============================================
// НАСТРОЙКА ТЕСТИРУЕМЫХ МОДУЛЕЙ
// ============================================
// Раскомментируйте ТОЛЬКО тот модуль, который тестируете
// ============================================

//#define TEST_GPS          // Тестируем GPS модуль
#define TEST_SIM800L   // Тестируем GSM модуль (потом)
//#define TEST_MPU6050   // Тестируем акселерометр (потом)
// #define TEST_EEPROM    // Тестируем энергонезависимую память (потом)

// ============================================

// Создаём объект GPS на пинах D3 (RX) и D4 (TX)
NEO gps(3, 4);
SIM sim800l(5, 6);
MPU mpu6050;


unsigned long lastStatus = 0;
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════╗");
  Serial.println("║           MODULE TEST BENCH                      ║");
  Serial.println("║   Select which module to test in main.cpp       ║");
  Serial.println("╚══════════════════════════════════════════════════╝");
  Serial.println();
  
  // ============================================
  // ИНИЦИАЛИЗАЦИЯ ТОЛЬКО ВЫБРАННЫХ МОДУЛЕЙ
  // ============================================
  
  #ifdef TEST_GPS
    Serial.println("GPS module ENABLED");
    gps.begin(9600);
  #else
    gps.disable();
  #endif
  
  #ifdef TEST_SIM800L
    Serial.println("SIM800L module ENABLED");
    sim800l.begin(9600);
  #else
    sim800l.disable();
  #endif
  
  #ifdef TEST_MPU6050
    Serial.println("MPU6050 module ENABLED");
    mpu6050.begin();
  #else
    mpu6050.disable();
  #endif
  
  #ifdef TEST_EEPROM
    Serial.println("EEPROM module ENABLED (coming soon)");
    // eeprom.begin();
  #endif
  
  Serial.println();
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println();
}

void loop() {
  // ============================================
  // ОБНОВЛЕНИЕ ТОЛЬКО ВКЛЮЧЁННЫХ МОДУЛЕЙ
  // ============================================
  
  #ifdef TEST_GPS
    gps.update();
  #endif
  
  // ============================================
  // ОБРАБОТКА ДАННЫХ GPS
  // ============================================
  
  #ifdef TEST_GPS
    String data = gps.getRawData();
    if (data.length() > 0) {
      Serial.println(data);
    }
    
    // Статус каждые 5 секунд (только если нет данных)
    if (millis() - lastStatus >= 5000) {
      lastStatus = millis();
      
      if (!gps.available()) {
        Serial.println("[GPS] Waiting for signal...");
        Serial.println();
      }
    }
  #endif
  
  // ============================================
  // ОБРАБОТКА ДАННЫХ SIM
  // ============================================
  
  #ifdef TEST_SIM800L
  sim800l.update();
  #endif
  
  #ifdef TEST_SIM800L

  // отправляем AT каждые 3 секунды
  if (millis() - lastPrint >= 3000) {
    lastPrint = millis();
    Serial.println("[SIM] Sending AT...");
    sim800l.test();
  }

  String simData = sim800l.getData();
  if (simData.length() > 0) {
    Serial.print("[SIM] ");
    Serial.println(simData);
  }

  #endif
  // ============================================
  // ОБРАБОТКА ДАННЫХ SIM
  // ============================================

  #ifdef TEST_MPU6050
    mpu6050.update();
  #endif
  
  #ifdef TEST_MPU6050

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    if (mpu6050.available()) {
      Serial.print("[MPU] ");
      Serial.println(mpu6050.getData());
    } else {
      Serial.println("[MPU] Not initialized");
    }
  }

  #endif


  #ifdef TEST_EEPROM
    // Код для EEPROM
  #endif
}