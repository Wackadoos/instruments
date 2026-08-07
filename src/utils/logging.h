#pragma once

#include <Arduino.h>

class Logging {
 public:
  template <typename T>
  static void logDebug(T logMessage) {
#ifdef DEBUG_LOGGING
    Serial.println(logMessage);
#endif
  }

  template <typename T>
  static void logDebug(const __FlashStringHelper* context, T logMessage) {
#ifdef DEBUG_LOGGING
    Serial.print(context);
    Serial.println(logMessage);
#endif
  }
};
