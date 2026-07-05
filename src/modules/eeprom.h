#pragma once

#include <Arduino.h>
#include <SparkFun_External_EEPROM.h>
#include <Wire.h>

class EEPROM {
 public:
  inline static ExternalEEPROM eeprom = ExternalEEPROM();

  static void init(TwoWire* wire);
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
};
