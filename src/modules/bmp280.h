#pragma once

#include <Adafruit_BMP280.h>
#include <Arduino.h>

#include "state.h"

class BMP {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
  inline static SensorState* sensorState;
  inline static Adafruit_BMP280 bmp280;  // Initialised in init to set wire
};
