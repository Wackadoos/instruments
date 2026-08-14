#pragma once

#include <Arduino.h>
#include <NMEAGPS.h>

#include "utils/metrics.h"

#ifndef NMEAGPS_INTERRUPT_PROCESSING
#error You must define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

class GPS {
 public:
  static void init();
  static void run();
  static inline bool isEnabled() { return enabled; };

 private:
  static NMEAGPS gps;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;

  static void GPSisr(uint8_t c);
};
