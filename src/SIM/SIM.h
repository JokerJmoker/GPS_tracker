#ifndef SIM_H
#define SIM_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SIM {
  private:
    SoftwareSerial* _simSerial;  // Указатель на объект программного UART для связи с SIM модулем
    int _rxPin;                   // Пин для приёма данных (RX) от SIM модуля
    int _txPin;                   // Пин для передачи данных (TX) в SIM модуль
    bool _enabled;                // Флаг логического включения/выключения модуля (НЕ физическое отключение питания)
    String _buffer;               // Буфер для накопления строки ответа до символа \n
    bool _hasNewData;             // Флаг наличия новой полной строки данных от модуля

  public:
    // Конструктор - сохраняет пины и создаёт объект SoftwareSerial
    SIM(int rxPin, int txPin);

    // Отправка AT команды в модуль (автоматически добавляет \r\n)
    void sendCommand(const String& cmd);

    // Инициализация - запускает UART с заданной скоростью (по умолчанию 9600)
    void begin(long baudrate = 9600);

    // Включение модуля (логически) - запускает UART на скорости 9600
    void enable();
    
    // Выключение модуля (логически) - закрывает UART порт и очищает буфер
    void disable();
    
    // Проверка, включён ли модуль (возвращает состояние флага _enabled)
    bool isEnabled();
    
    // Установка состояния включения/выключения
    void setEnabled(bool enabled);

    // Обновление данных - чтение из UART и накопление строк (вызывать часто в loop)
    void update();
    
    // Проверка наличия данных в буфере UART
    bool available();
    
    // Получение данных - возвращает одну полную строку ответа от модуля
    String getData();

    // Тест связи - отправляет базовую AT команду для проверки работы модуля
    void test(); // отправка AT
};

#endif