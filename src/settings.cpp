#include "settings.h"

#include "modules/eeprom.h"
#include "modules/speed.h"
#include "state.h"
#include "utils/errors.h"

SettingsBlock SETTINGS::settings = SettingsBlock();

void SETTINGS::init() {
  if (EEPROM::isEnabled()) {
    EEPROM::eeprom.get(SETTINGS_INDEX, settings);

    if (settings._version != SETTINGS_BLOCK_VERSION) {
      pushSetting(defaultSettings());
      Errors::logError(Error::SETTINGS_VERSION_CHANGED);
    } else if (checksum(settings) != settings._checksum) {
      pushSetting(defaultSettings());
      Errors::logError(Error::SETTINGS_CHECKSUM_FAILED);
    }

    SETTINGS::apply();
  } else {
    settings = defaultSettings();
    Errors::logError(Error::SETTINGS_VOLATILE);
    SETTINGS::apply();  // Keep display strings & SPEED consistent even without EEPROM
  }
}

SettingsBlock SETTINGS::getSettings() {
  return settings;
}

void SETTINGS::pushSetting(const SettingsBlock& newSettings) {
  settings = newSettings;
  settings._checksum = checksum(settings);

  if (EEPROM::isEnabled()) {
    EEPROM::eeprom.put(SETTINGS_INDEX, settings);  // Write the checksummed copy, not the caller's stale-checksum block!
  }

  SETTINGS::apply();
}

void SETTINGS::apply() {
  SPEED::configure(
      settings.speed_sensor_wheel_circumference,
      settings.speed_sensor_pulses_per_revolution);

  // Format timezone offset (15-min units) as +hh:mm, e.g. 40 -> "+10:00"
  int8_t quarters = constrain(settings.timezone_offset, TIMEZONE_OFFSET_MIN, TIMEZONE_OFFSET_MAX);
  sprintf(State::timezone_string, "%c%02d:%02d", quarters < 0 ? '-' : '+', abs(quarters) / 4, (abs(quarters) % 4) * 15);
}

SettingsBlock SETTINGS::defaultSettings() {
  auto default_block = SettingsBlock{
      ._version = SETTINGS::SETTINGS_BLOCK_VERSION,
      .timezone_offset = 40,
      .speed_sensor_wheel_circumference = 1596,  // 20" Wheel
      .speed_sensor_pulses_per_revolution = 4,
      .race_mode_last_save_seconds = 0,
      .battery_I_avg = 0,
      .totalDischargedWh_calibration = 0,
      .totalChargedWh_calibration = 0,
      ._checksum = 0,
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
