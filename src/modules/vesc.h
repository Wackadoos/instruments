#pragma once

#include <Arduino.h>
#include <VescUart.h>

#include "utils/metrics.h"

#define VOLTAGE_CALIBRATION 0.2f

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
