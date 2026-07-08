#pragma once

#include <Adafruit_AHTX0.h>
#include <Arduino.h>

#include "state.h"
#include "utils/metrics.h"

class ATH {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  static Adafruit_AHTX0 ath20;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
  inline static SensorState* sensorState = nullptr;
};
