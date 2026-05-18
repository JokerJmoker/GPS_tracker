// =====================================================
// FILE: src/GSM/GSM.h
// =====================================================

#ifndef GSM_H
#define GSM_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class GSM {
  private:
    SoftwareSerial* _gsmSerial;
    int _rxPin;
    int _txPin;
    bool _enabled;
    String _buffer;
    String _lineBuffer;
    String _multiLineBuffer;      // НОВОЕ: буфер для многострочных ответов
    bool _hasNewData;
    bool _responseComplete;        // НОВОЕ: флаг завершения ответа
    unsigned long _responseStartTime;  // НОВОЕ: время начала ответа

  public:
    GSM(int rxPin, int txPin);
    void begin(long baudrate = 9600);
    void enable();
    void disable();
    bool isEnabled();
    void setEnabled(bool enabled);
    bool available();
    void update();
    String getData();
    void sendCommand(const String& cmd);
    void test();
    String sendCommandWithResponse(const String& cmd, int timeoutMs = 5000);
    void clearBuffers();
    bool hasData();
    
    // НОВЫЕ МЕТОДЫ для многострочных ответов
    String getFullResponse();      // Получить полный многострочный ответ
    bool hasFullResponse();        // Проверить наличие полного ответа
    void setResponseTimeout(int ms); // Установить таймаут для сбора ответа

    bool sendSMS(const String& phoneNumber, const String& message);
    bool sendSMS(const String& phoneNumber, const String& message, int timeoutMs);
};

#endif