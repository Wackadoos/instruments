#pragma once

#include <Adafruit_AHTX0.h>
#include <Arduino.h>

#include "utils/metrics.h"

class ATH {
 public:
  static void init(TwoWire* wire);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  static Adafruit_AHTX0 ath20;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
};
