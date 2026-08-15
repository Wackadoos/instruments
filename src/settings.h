#pragma once

#include <Arduino.h>

// If using on a non 8 bit platform, add pragmas for tight packing!
struct SettingsBlock {
  uint8_t _version;                            //* Must always be first!
  int8_t timezone_offset;                      // Timezone offset from UTC. 15min increments.
  uint16_t speed_sensor_wheel_circumference;   // Circumference of wheel in mm
  uint8_t speed_sensor_pulses_per_revolution;  // Number of pulses per revolution
  // Persisted Data
  uint32_t race_mode_last_save_seconds;  // Time of last save (seconds since 2000/1/1)
  float battery_I_avg;                   // Average battery current
  float totalDischargedWh_calibration;   // Previous value of dischargedWH
  float totalChargedWh_calibration;      // Previous value of chargedWH
  uint16_t _checksum;                    //* Checksum of settings. Must always be last!
};

class SETTINGS {
 public:
  static constexpr uint8_t SETTINGS_BLOCK_VERSION = 3;  // Bump every time the SettingsBlock object gets modified!

  static void init();
  static SettingsBlock getSettings();
  static void pushSetting(const SettingsBlock& newSettings);

 private:
  static SettingsBlock settings;

  static constexpr uint32_t SETTINGS_INDEX = 0;  // Where in the eeprom to store the settings. Typically 0 (the start)

  static void apply();
  static SettingsBlock defaultSettings();
  static uint16_t checksum(const SettingsBlock& settings);
  static uint16_t fletcher16(const uint8_t* data, size_t len);
};
