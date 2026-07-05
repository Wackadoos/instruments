#pragma once

#include <OneWire.h>

#include "state.h"

class HARDWARE {
 public:
  static void init(SensorState* state);
  static void run();

 private:
  inline static OneWire oneWire = OneWire(42);
};
