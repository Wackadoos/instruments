#pragma once

#include <Arduino.h>
#include <RTClib.h>

#include "state.h"
#include "utils/metrics.h"

class RTC {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static void adjust();
  static inline bool isEnabled() { return enabled; };

 private:
  static RTC_DS3231 rtc;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
  inline static SensorState* sensorState = nullptr;
  inline static bool needs_adjust = false;
};

// TODO Only adjust time when not in race mode and avoid updating too frequently!
