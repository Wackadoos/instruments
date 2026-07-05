#pragma once

#include <Adafruit_AHTX0.h>
#include <Arduino.h>

#include "state.h"

class ATH {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
  inline static SensorState* sensorState;
  inline static Adafruit_AHTX0 ath20 = Adafruit_AHTX0();
};
