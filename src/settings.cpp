#include "settings.h"

#include "errors.h"
#include "modules/eeprom.h"

void SETTINGS::init() {
  if (EEPROM::isEnabled()) {
    EEPROM::eeprom.get(SETTINGS_INDEX, settings);

    if (settings._version != SETTINGS_BLOCK_VERSION) {
      settings = defaultSettings();
      Errors::logError(Error::SETTINGS_VERSION_CHANGED);
    } else if (checksum(settings) != settings._checksum) {
      settings = defaultSettings();
      Errors::logError(Error::SETTINGS_CHECKSUM_FAILED);
    }
  } else {
    settings = defaultSettings();
    Errors::logError(Error::SETTINGS_VOLATILE);
  }
}

SettingsBlock SETTINGS::getSettings() {
  return settings;
}

void SETTINGS::pushSetting(const SettingsBlock& newSettings) {
  settings = newSettings;
  settings._checksum = checksum(settings);
  if (EEPROM::isEnabled()) {
    EEPROM::eeprom.put(SETTINGS_INDEX, newSettings);  // This could be optimised to only write the bytes that changed!
  }
}

SettingsBlock SETTINGS::defaultSettings() {
  auto default_block = SettingsBlock{
      SETTINGS::SETTINGS_BLOCK_VERSION,
      0,
      0,
  };
  default_block._checksum = SETTINGS::checksum(default_block);
  return default_block;
}

uint16_t SETTINGS::checksum(const SettingsBlock& settings) {
  return fletcher16(reinterpret_cast<const uint8_t*>(&settings), sizeof(settings) - sizeof(settings._checksum));
}

uint16_t SETTINGS::fletcher16(const uint8_t* data, size_t len) {  // Probably overkill for a checksum, but does the job
  uint16_t sum1 = 0, sum2 = 0;
  for (size_t i = 0; i < len; i++) {
    sum1 = (sum1 + data[i]) % 255;
    sum2 = (sum2 + sum1) % 255;
  }
  return (sum2 << 8) | sum1;
}
