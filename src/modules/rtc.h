#pragma once

#include <Arduino.h>
#include <RTClib.h>
#undef SECONDS_PER_DAY  // Fix collision with NeoGPS

#include "utils/metrics.h"

class RTC {
 public:
  static RTC_DS3231 clock;
  inline static bool needs_adjust = false;

  static void init(TwoWire* wire);

  static void update();
  static void adjust();
  static inline bool isEnabled() { return enabled; };

 private:
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
};
