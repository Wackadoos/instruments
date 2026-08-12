#pragma once

#include <Arduino.h>

#include "utils/metrics.h"

class SPEED {
 public:
  static void init();
  static void configure(uint16_t wheelCircumferenceMillimeters, uint8_t pulsesPerRotation);
  static void update();
  static inline bool isEnabled() { return enabled; };

 private:
  static IntervalMetric dataProcessTime;

  inline static bool enabled = false;
  inline static float coefficient = 0;             // Coefficient pre-calculated from timescale, wheel size & pulses per rotation
  inline volatile static uint16_t pulseCount = 0;  // Updated in ISR
  inline static uint32_t previousMicros = 0;       // Last time we reset the pulse count

  static void sensor_isr();
};
