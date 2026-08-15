#pragma once

#include <Arduino.h>
#include <mpu6500.h>

#include "utils/metrics.h"

class IMU {
 public:
  static void init(TwoWire* wire);
  static void run();
  static inline bool isEnabled() { return enabled; };

 private:
  static bfs::Mpu6500 imu;
  static IntervalMetric dataProcessTime;

  static void updateMax1s(uint8_t axis, float value);
  static void promote(uint8_t axis, float windowMin);

  static constexpr uint8_t AXES = 3;
  static constexpr uint8_t SAMPLES = 50;  // 1s dwell window at 50 Hz

  static float accelBuf[AXES][SAMPLES];  // Ring buffer of fabsf() magnitudes per axis
  static uint8_t accelMinIdx[AXES];      // Index of the current running minimum in each axis buffer
  static uint8_t accelHead;              // Next write position (also the eviction point when full)
  static uint16_t accelCount;            // Samples written so far (clamps at SAMPLES)

  inline static bool enabled = false;
  inline static volatile bool dataReady = false;  // Set by ISR when new sample is available

  static void imu_isr();
};
