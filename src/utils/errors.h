#pragma once

#include <Arduino.h>

#define CACHED_ERRORS 10

enum class Error : uint8_t {
  UNKNOWN,
  BMP280_UNINITIALISED,
  ATH20_UNINITIALISED,
  RTC_UNINITIALISED,
  RTC_LOST_POWER,
  IMU_UNINITIALISED,
  IMU_SAMPLE_RATE_ERR,
  IMU_INTERRUPT_ERR,
  IMU_DATA_READ_FAILED,
  NVRAM_UNINITIALISED,
  I2C_TIMEOUT,
  EEPROM_UNINITIALISED,
  SETTINGS_VERSION_CHANGED,
  SETTINGS_CHECKSUM_FAILED,
  SETTINGS_VOLATILE,
  NO_TEMPS_FOUND,
  INCORRECT_NUM_TEMPS,
  GHOST_TEMP_PROBE,
  TEMP_RESOLUTION_INCORRECT,
  DUPLICATE_TEMP_SENSOR_IDS,
  TEMP_UNREADABLE,
  VESC_DATA_UNAVAILABLE,
  VESC_REPORTED_UNCOMMON_ERROR,
  VESC_REPORTED_HARDWARE_FAULT,
  VESC_REPORTED_TEMPERATURE_FAULT,
  VESC_REPORTED_VOLTAGE_FAULT,
  VESC_REPORTED_FLASH_CORRUPTION,
  DISPLAY_UNINITIALISED,
  TOUCHSCREEN_UNINITIALISED
};

struct ErrorEvent {
  Error type;
  unsigned long board_time_ms;
  // TODO wall time
};

class Errors {
 public:
  inline static uint8_t errorCount = 0;
  inline static ErrorEvent errors[CACHED_ERRORS] = {};  // Circular buffer of last few errors for display on screen
  static void init();
  static void logError(Error error);
  static String errorDescription(Error error);

 private:
  inline static uint8_t errors_head = 0;
  static void flushToLogfile(const ErrorEvent& errorEvent);
};
