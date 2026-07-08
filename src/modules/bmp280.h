#pragma once

#include <Adafruit_BMP280.h>
#include <Arduino.h>

#include "state.h"
#include "utils/metrics.h"

class BMP {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  static Adafruit_BMP280 bmp280;
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
  inline static SensorState* sensorState = nullptr;
};
