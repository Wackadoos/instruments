#pragma once

#include <Arduino.h>
#include <RTClib.h>

#include "state.h"

class RTC {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static void adjust();
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
  inline static SensorState* sensorState;
  inline static bool needs_adjust = false;
  inline static RTC_DS3231 rtc = RTC_DS3231();
};
