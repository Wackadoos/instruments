#pragma once

#include <Arduino.h>
#include <mpu9250.h>

#include "state.h"

class IMU {
 public:
  static void init(TwoWire* wire, SensorState* state);
  static void run();
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
  inline static SensorState* sensorState;
  inline static bfs::Mpu9250 imu = bfs::Mpu9250();
  inline static volatile bool dataReady = false;  // Set by ISR when new sample is available

  static void imu_isr();
};
