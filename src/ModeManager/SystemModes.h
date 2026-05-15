#ifndef SYSTEM_MODES_H
#define SYSTEM_MODES_H

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>

// ============================================
// ПЕРЕЧИСЛЕНИЕ РЕЖИМОВ РАБОТЫ
// ============================================
enum class OperationMode {
  DEBUG_MODE,      // Режим отладки - всё работает постоянно
                   // - Все модули активны одновременно

                   
  TRACKER_MODE,    // Режим трекера - циклическая работа GPS->SIM
                   // - GPS и SIM работают по очереди
                   // - Экономия энергии для батарейного питания
                   
  SLEEP_MODE       // Режим сна - пробуждение от MPU6050
                   // - Устройство в глубоком сне большую часть времени
                   // - Пробуждение по прерыванию от датчика движения
};

// ============================================
// КЛАСС УПРАВЛЕНИЯ РЕЖИМАМИ
// ============================================
// Все методы и атрибуты статические - не нужно создавать объект
// Использование: SystemModes::begin(), SystemModes::setMode(...)
// ============================================
class SystemModes {
  private:
    // ===== ТЕКУЩЕЕ СОСТОЯНИЕ =====
    static OperationMode _currentMode;      // Активный режим работы (DEBUG/TRACKER/SLEEP)
    static bool _modeSwitchRequested;        // Флаг запроса на смену режима
    
    // ===== ТАЙМИНГИ ДЛЯ ТРЕКЕРА =====
    static unsigned long _gpsStartTime;      // Время начала работы GPS (миллисекунды)
    static unsigned long _simStartTime;      // Время начала работы SIM (миллисекунды)
    
    // ===== ФЛАГИ СОСТОЯНИЯ ЦИКЛА ТРЕКЕРА =====
    static bool _gpsCompleted;               // GPS завершил сбор данных?
    static bool _simCompleted;               // SIM завершил отправку данных?
    static bool _oneCycleComplete;           // Полный цикл (GPS+SIM) завершён?
    static bool _gpsDataReceived;            // Получены ли валидные данные от GPS?
    static bool _simCommandsCompleted;       // Выполнены ли все AT команды SIM?
    static bool _simCommandsDone;            // Алиас для совместимости (дубликат)
    
  public:
    // ===== УПРАВЛЕНИЕ РЕЖИМАМИ =====
    
    // Установка режима работы из кода
    // @param mode - один из режимов OperationMode
    static void setMode(OperationMode mode);
    
    // Получение текущего режима работы
    // @return активный режим (DEBUG_MODE/TRACKER_MODE/SLEEP_MODE)
    static OperationMode getCurrentMode();
    
    // ===== УПРАВЛЕНИЕ ЦИКЛОМ ТРЕКЕРА =====
    
    // Сброс состояния цикла трекера (начинаем с GPS)
    // Сбрасывает все флаги и таймеры в исходное состояние
    static void resetTrackerCycle();
    
    // Обновление состояния трекера на основе полученных данных
    // @param gpsHasData - true если GPS получил валидные координаты
    // @param simCommandsDone - true если SIM выполнил все AT команды
    static void updateTrackerState(bool gpsHasData, bool simCommandsDone);
    
    // Определяет, должен ли GPS быть активен в текущий момент
    // @return true - GPS нужно включить, false - GPS можно выключить
    static bool shouldGPSBeActive();
    
    // Определяет, должен ли SIM модуль быть активен в текущий момент
    // @return true - SIM нужно включить, false - SIM можно выключить
    static bool shouldSIMBeActive();
    
    // Проверка, завершён ли полный цикл трекера (GPS+SIM)
    // @return true - цикл завершён, можно сбрасывать состояние
    static bool isTrackerCycleComplete();
    
    // ===== УПРАВЛЕНИЕ РЕЖИМОМ СНА =====
    
    // Уход в глубокий сон (только в режиме SLEEP_MODE)
    // Останавливает выполнение программы до внешнего прерывания
    static void goToSleep();
    
    // Обработчик прерывания пробуждения от MPU6050
    // Вызывается автоматически при обнаружении движения
    static void wakeUp();
    
    // ===== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ =====
    
    // Визуальная индикация текущего режима (мигание светодиодом)
    // Также выводит информацию в Serial
    static void indicateModeChange();
    
    // Инициализация системы режимов
    // Настраивает пины и начальное состояние
    static void begin();
    
    // Принудительный полный сброс всех состояний
    // Более жёсткий, чем resetTrackerCycle() - сбрасывает абсолютно всё
    static void forceReset();
};

#endif