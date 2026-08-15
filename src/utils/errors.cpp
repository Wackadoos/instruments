#include "utils/errors.h"

#include "modules/rtc.h"
#include "utils/logging.h"

// TODO extend to have info logging as well, ie log "TIME UPDATED FROM GPS etc"

void Errors::init() {
  // TODO create & open logfile with time as name.s
  // SD.open(time.timestamp()+".log", FILE_WRITE) use DateTime::TIMESTAMP_TIME for file DateTime::TIMESTAMP_DATE for folder
}

void Errors::logError(Error error) {
  // Each error type occupies a single slot; repeat occurrences just bump its count.
  for (uint8_t i = 0; i < CACHED_ERRORS; i++) {
    if (errors[i].type == error) {
      errors[i].count++;
      return;
    }
  }

  // First occurrence: timestamp + log, then advance the head.
  errors[errors_head] = ErrorEvent{error, RTC::clock.now().secondstime(), 1};

  flushToLogfile(errors[errors_head]);

  errors_head++;
  if (errors_head >= CACHED_ERRORS) {
    errors_head = 0;
  }
}

ErrorEvent Errors::newestError(uint8_t index) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < CACHED_ERRORS; i++) {
    uint8_t j = (uint8_t)((errors_head + CACHED_ERRORS - 1 - i) % CACHED_ERRORS);
    ErrorEvent e = errors[j];
    if (e.type == Error::UNKNOWN) {
      continue;
    }
    if (n == index) {
      return e;
    }
    n++;
  }
  return ErrorEvent{Error::UNKNOWN, 0};
}

const __FlashStringHelper* Errors::errorDescription(Error error) {
  switch (error) {
    case Error::UNKNOWN:
      return F("An unknown error occurred!");
    case Error::BMP280_UNINITIALISED:
      return F("The BMP280 Pressure Sensor failed to initialise!");
    case Error::ATH20_UNINITIALISED:
      return F("The ATH20 Humidity Sensor failed to initialise!");
    case Error::RTC_UNINITIALISED:
      return F("The Realtime Clock failed to initialise!");
    case Error::RTC_LOST_POWER:
      return F("The Realtime Clock lost power. Time is inaccurate!");
    case Error::RTC_OUT_OF_SYNC:
      return F("The Realtime Clock > 10s out of sync with GPS time!");
    case Error::IMU_UNINITIALISED:
      return F("The IMU failed to initialise!");
    case Error::IMU_SAMPLE_RATE_ERR:
      return F("The IMU failed to set the sample rate!");
    case Error::IMU_INTERRUPT_ERR:
      return F("The IMU failed to enable data interrupt!");
    case Error::IMU_DATA_READ_FAILED:
      return F("An IMU read failed... This may be intermittent.");
    case Error::NVRAM_UNINITIALISED:
      return F("The NVRAM failed to initialise!");
    case Error::I2C_TIMEOUT:
      return F("I2C timeout occurred!");
    case Error::EEPROM_UNINITIALISED:
      return F("The EEPROM failed to initialise!");
    case Error::SETTINGS_VERSION_CHANGED:
      return F("The settings version has changed! Resetting!");
    case Error::SETTINGS_CHECKSUM_FAILED:
      return F("The settings are corrupt! Resetting!");
    case Error::SETTINGS_VOLATILE:
      return F("The settings aren't being saved! No EEPROM!");
    case Error::NO_TEMPS_FOUND:
      return F("No DS18B20 temperature sensors found!");
    case Error::INCORRECT_NUM_TEMPS:
      return F("Incorrect number of DS18B20 temperature sensors!");
    case Error::GHOST_TEMP_PROBE:
      return F("(Ghost) OneWire temperature sensor didn't respond!");
    case Error::TEMP_RESOLUTION_INCORRECT:
      return F("Failed to set temperature resolution on DS18B20!");
    case Error::DUPLICATE_TEMP_SENSOR_IDS:
      return F("DS18B20 sensors detected with duplicate IDs!");
    case Error::TEMP_UNREADABLE:
      return F("DS18B20 sensor temperature unreadable!");
    case Error::VESC_DATA_UNAVAILABLE:
      return F("Failed to read data from VESC!");
    case Error::VESC_REPORTED_UNCOMMON_ERROR:
      return F("Uncommon/unknown error reported on VESC!");
    case Error::VESC_REPORTED_HARDWARE_FAULT:
      return F("Hardware error reported on VESC!");
    case Error::VESC_REPORTED_TEMPERATURE_FAULT:
      return F("Temperature error reported on VESC!");
    case Error::VESC_REPORTED_VOLTAGE_FAULT:
      return F("Voltage error reported on VESC!");
    case Error::VESC_REPORTED_FLASH_CORRUPTION:
      return F("Flash Corruption error reported on VESC!");
    case Error::DISPLAY_UNINITIALISED:
      return F("The Display failed to initialise!");
    case Error::TOUCHSCREEN_UNINITIALISED:
      return F("The Touchscreen failed to initialise!");
    case Error::GPS_OVERRUN:
      return F("GPS data overrun, not read frequently enough!");
    case Error::SD_UNINITIALISED:
      return F("The SD card failed to initialise!");
    case Error::SD_OPEN_FAILED:
      return F("Failed to create the SD log file!");
    case Error::SD_PREALLOCATE_FAILED:
      return F("Failed to preallocate space on the SD card!");
    case Error::SD_WRITE_FAILED:
      return F("Failed to write data to the SD card!");
    case Error::SD_LOG_OVERRUN:
      return F("SD card too busy - log frames were dropped!");
    case Error::SD_STALLED:
      return F("SD card is not responding - logging stopped!");
    default:
      return F("An undefined error occurred!");
  }
}

void Errors::flushToLogfile(const ErrorEvent& errorEvent) {
#ifdef DEBUG_LOGGING
  Logging::logDebug(F("ERROR OCCURRED: "), errorDescription(errorEvent.type));
#endif
  // TODO write out data to logfile in csv
}
