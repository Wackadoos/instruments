#pragma once

#include <Arduino.h>

// If using on a non 8 bit platform, add pragmas for tight packing!
struct SettingsBlock {
  uint8_t _version;        //* Must always be first!
  int8_t timezone_offset;  // Timezone offset from UTC. 15min increments.
  uint16_t _checksum;      //* Checksum of settings. Must always be last!
};

class SETTINGS {
 public:
  static constexpr uint8_t SETTINGS_BLOCK_VERSION = 1;  // Bump every time the SettingsBlock object gets modified!

  static void init();
  static SettingsBlock getSettings();
  static void pushSetting(const SettingsBlock& newSettings);

 private:
  static constexpr uint32_t SETTINGS_INDEX = 0;  // Where in the eeprom to store the settings. Typically 0 (the start)
  inline static SettingsBlock settings = SettingsBlock();

  static SettingsBlock defaultSettings();
  static uint16_t checksum(const SettingsBlock& settings);
  static uint16_t fletcher16(const uint8_t* data, size_t len);
};
