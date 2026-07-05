#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "state.h"

class TEMPS {
 public:
  static constexpr uint8_t EXPECTED_SENSOR_COUNT = 2;
  static constexpr uint8_t SENSOR_RESOLUTION = 9;

  static void init(OneWire* oneWire, SensorState* state);
  static void update();
  static void run();
  static inline bool isEnabled() { return enabled; };

 private:
  inline static bool enabled = false;
  inline static SensorState* sensorState;
  inline static DallasTemperature sensors = DallasTemperature();
  inline static DeviceAddress addresses[EXPECTED_SENSOR_COUNT] = {};
  inline static uint8_t current_address = 0;  // Round robin sensor selector

  static int8_t compare(DeviceAddress a, DeviceAddress b);
  static bool sortAddresses(DeviceAddress* addresses, uint8_t count);
};
