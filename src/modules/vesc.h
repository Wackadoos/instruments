#pragma once

#include <Arduino.h>
#include <VescUart.h>

#include "utils/metrics.h"

class VESC {
 public:
  static void init(Stream* port);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  static VescUart vesc;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
};
