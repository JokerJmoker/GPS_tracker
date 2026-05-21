// =====================================================
// FILE: src/Config_Button.h (ВАРИАНТ С ОТДЕЛЬНЫМИ МАКРОСАМИ)
// =====================================================

#ifndef CONFIG_BUTTON_H
#define CONFIG_BUTTON_H

// ============================================
// НАСТРОЙКИ КНОПКИ
// ============================================
#define BUTTON_PIN 8
#define DEBOUNCE_DELAY_MS 50
#define DOUBLE_CLICK_MAX_MS 500
#define LONG_PRESS_MS 2000
#define REPEAT_CHECK_MS 10

// ============================================
// РЕЖИМ ПО УМОЛЧАНИЮ (раскомментировать ТОЛЬКО ОДНУ строку)
// ============================================
// #define DEFAULT_TRACKER_MODE
#define DEFAULT_SLEEP_MODE
// #define DEFAULT_DEBUG_MODE

// ============================================
// ПРЕОБРАЗОВАНИЕ В МАКРОС РЕЖИМА
// ============================================
#ifdef DEFAULT_TRACKER_MODE
    #define MODE_TRACKER
#elif defined(DEFAULT_SLEEP_MODE)
    #define MODE_SLEEP
#elif defined(DEFAULT_DEBUG_MODE)
    #define MODE_DEBUG
#else
    // По умолчанию DEBUG, если ничего не выбрано
    #define MODE_DEBUG
#endif

#endif // CONFIG_BUTTON_H