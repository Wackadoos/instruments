#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "utils/metrics.h"

class TEMPS {
 public:
  static constexpr uint8_t EXPECTED_SENSOR_COUNT = 2;
  static constexpr uint8_t SENSOR_RESOLUTION = 9;

  static void init(OneWire* oneWire);
  static void update();
  static void run();
  static inline bool isEnabled() { return enabled; };

 private:
  static IntervalMetric dataRequestTime;
  static IntervalMetric dataAcquisitionTime;
  static IntervalMetric dataProcessTime;
  static DallasTemperature sensors;

  inline static bool enabled = false;
  inline static DeviceAddress addresses[EXPECTED_SENSOR_COUNT] = {};
  inline static uint8_t current_address = 0;  // Round robin sensor selector
  inline static bool newData = false;

  static int8_t compare(DeviceAddress a, DeviceAddress b);
  static bool sortAddresses(DeviceAddress* addresses, uint8_t count);
};
