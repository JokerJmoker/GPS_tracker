//GSM.cpp
#include "GSM.h"

GSM::GSM(int rxPin, int txPin) {
  _rxPin = rxPin;
  _txPin = txPin;
  _gsmSerial = new SoftwareSerial(rxPin, txPin);
  _enabled = true;
  _buffer = "";
  _lineBuffer = "";
  _multiLineBuffer = "";
  _hasNewData = false;
  _responseComplete = false;
  _responseStartTime = 0;
}

void GSM::begin(long baudrate) {
  if (_enabled) {
    _gsmSerial->begin(baudrate);
    delay(100);
    clearBuffers();
  }
}

void GSM::enable() {
  _enabled = true;
  _gsmSerial->begin(9600);
  delay(100);
  clearBuffers();
}

void GSM::disable() {
  _enabled = false;
  _gsmSerial->end();
  clearBuffers();
}

bool GSM::isEnabled() {
  return _enabled;
}

void GSM::setEnabled(bool enabled) {
  if (enabled && !_enabled) enable();
  else if (!enabled && _enabled) disable();
}

bool GSM::available() {
  if (!_enabled) return false;
  return _gsmSerial->available();
}

// ============= УЛУЧШЕННЫЙ update() для многострочных ответов =============
void GSM::update() {
  if (!_enabled) return;

  while (_gsmSerial->available()) {
    char c = _gsmSerial->read();

    if (c == '\n') {
      // Сохраняем строку в lineBuffer
      _lineBuffer = _buffer;
      _buffer = "";
      
      // Добавляем строку в многострочный буфер
      if (_lineBuffer.length() > 0) {
        _multiLineBuffer += _lineBuffer;
        _multiLineBuffer += "\n";
      }
      
      _hasNewData = true;
      
      // Проверяем, завершён ли ответ
      // Ответ считается завершённым, если:
      // 1. Пришла строка "OK" или "ERROR"
      // 2. Прошло больше 100 мс с начала ответа и нет новых данных
      if (_lineBuffer == "OK" || _lineBuffer == "ERROR" || _lineBuffer.indexOf("ERROR") >= 0) {
        _responseComplete = true;
      }
    } 
    else if (c != '\r') {
      _buffer += c;
      
      if (_buffer.length() > 256) {
        _buffer = "";
      }
    }
    
    // Обновляем таймер активности ответа
    _responseStartTime = millis();
  }
  
  // Проверка таймаута ответа (если нет новых данных 100 мс - считаем ответ завершённым)
  if (_hasNewData && !_responseComplete && (millis() - _responseStartTime > 100)) {
    _responseComplete = true;
  }
}

// ============= ПОЛУЧЕНИЕ ОТДЕЛЬНЫХ СТРОК =============
String GSM::getData() {
  if (!_enabled) return "";

  if (_hasNewData) {
    _hasNewData = false;
    String temp = _lineBuffer;
    temp.trim();
    return temp;
  }
  return "";
}

// ============= НОВЫЙ МЕТОД: получение полного многострочного ответа =============
String GSM::getFullResponse() {
  if (!_enabled) return "";
  
  if (_responseComplete) {
    _responseComplete = false;
    String fullResponse = _multiLineBuffer;
    _multiLineBuffer = "";
    return fullResponse;
  }
  return "";
}

bool GSM::hasFullResponse() {
  return _responseComplete;
}

// ============= УЛУЧШЕННАЯ отправка команд =============
void GSM::sendCommand(const String& cmd) {
  if (!_enabled) return;
  
  clearBuffers();
  _responseComplete = false;
  _multiLineBuffer = "";
  delay(50);
  _gsmSerial->println(cmd);
}

// ============= УЛУЧШЕННЫЙ метод с ожиданием полного ответа =============
String GSM::sendCommandWithResponse(const String& cmd, int timeoutMs) {
  if (!_enabled) return "";
  
  clearBuffers();
  _responseComplete = false;
  _multiLineBuffer = "";
  _gsmSerial->println(cmd);
  
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    update();
    
    if (hasFullResponse()) {
      String response = getFullResponse();
      if (response.length() > 0) {
        response.trim();
        return response;
      }
    }
    delay(10);
  }
  return "";  // Таймаут
}

// ============= ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ =============
void GSM::clearBuffers() {
  _buffer = "";
  _lineBuffer = "";
  _multiLineBuffer = "";
  _hasNewData = false;
  _responseComplete = false;
  
  if (_enabled) {
    while (_gsmSerial->available()) {
      _gsmSerial->read();
    }
  }
}

bool GSM::hasData() {
  return _hasNewData;
}

bool GSM::sendSMS(const String& phoneNumber, const String& message) {
  return sendSMS(phoneNumber, message, 30000); // 30 секунд таймаут по умолчанию
}

bool GSM::sendSMS(const String& phoneNumber, const String& message, int timeoutMs) {
  if (!_enabled) {
    Serial.println("[GSM] ERROR: Module disabled");
    return false;
  }
  
  Serial.println("[GSM] Sending SMS...");
  
  // 1. Устанавливаем текстовый режим SMS
  _gsmSerial->println("AT+CMGF=1");
  delay(500);
  update();
  String response = getFullResponse();
  if (response.indexOf("OK") < 0) {
    Serial.println("[GSM] Failed to set SMS text mode");
    return false;
  }
  
  // 2. Указываем номер получателя
  _gsmSerial->print("AT+CMGS=\"");
  _gsmSerial->print(phoneNumber);
  _gsmSerial->println("\"");
  delay(500);
  
  // 3. Ждём приглашение для ввода текста (символ '>')
  unsigned long start = millis();
  bool readyForMessage = false;
  
  while (millis() - start < timeoutMs) {
    while (_gsmSerial->available()) {
      char c = _gsmSerial->read();
      if (c == '>') {
        readyForMessage = true;
        break;
      }
    }
    if (readyForMessage) break;
    delay(10);
  }
  
  if (!readyForMessage) {
    Serial.println("[GSM] No response for message input");
    return false;
  }
  
  // 4. Отправляем текст сообщения
  _gsmSerial->print(message);
  delay(100);
  
  // 5. Отправляем Ctrl+Z (ASCII 26) для завершения
  _gsmSerial->write(26);
  
  // 6. Ждём ответ (обычно "+CMGS: xx" и "OK")
  start = millis();
  bool sentSuccess = false;
  
  while (millis() - start < timeoutMs) {
    update();
    String fullResponse = getFullResponse();
    if (fullResponse.length() > 0) {
      Serial.print("[GSM] Response: ");
      Serial.println(fullResponse);
      
      if (fullResponse.indexOf("OK") >= 0 && fullResponse.indexOf("+CMGS") >= 0) {
        sentSuccess = true;
        break;
      }
    }
    delay(100);
  }
  
  if (sentSuccess) {
    Serial.println("[GSM] SMS sent successfully!");
    return true;
  } else {
    Serial.println("[GSM] Failed to send SMS");
    return false;
  }
}