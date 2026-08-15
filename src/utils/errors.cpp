#include "utils/errors.h"

#include "modules/rtc.h"
#include "utils/logging.h"

// TODO extend to have info logging as well, ie log "TIME UPDATED FROM GPS etc"

void Errors::init() {
  // TODO create & open logfile with time as name.s
  // SD.open(time.timestamp()+".log", FILE_WRITE) use DateTime::TIMESTAMP_TIME for file DateTime::TIMESTAMP_DATE for folder
}

void Errors::logError(Error error) {
  errors[errors_head] = ErrorEvent{error, RTC::clock.now().secondstime()};

  flushToLogfile(errors[errors_head]);

  errors_head++;
  if (errors_head >= CACHED_ERRORS) {
    errors_head = 0;
  }
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
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::NVRAM_UNINITIALISED:
      return F("The NVRAM failed to initialise!");
    case Error::I2C_TIMEOUT:
      return F("I2C timeout occurred!");
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
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
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_DATA_UNAVAILABLE:
      return F("Failed to read data from VESC!");
    // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_REPORTED_UNCOMMON_ERROR:
      return F("Uncommon/unknown error reported on VESC!");
    // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_REPORTED_HARDWARE_FAULT:
      return F("Hardware error reported on VESC!");
    // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_REPORTED_TEMPERATURE_FAULT:
      return F("Temperature error reported on VESC!");
    // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_REPORTED_VOLTAGE_FAULT:
      return F("Voltage error reported on VESC!");
    // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::VESC_REPORTED_FLASH_CORRUPTION:
      return F("Flash Corruption error reported on VESC!");
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::DISPLAY_UNINITIALISED:
      return F("The Display failed to initialise!");
    case Error::TOUCHSCREEN_UNINITIALISED:
      return F("The Touchscreen failed to initialise!");
    case Error::GPS_OVERRUN:
      return F("GPS data overrun, not read frequently enough!");
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

// TODO maybe just do 5s debounce for errors? Log multiple occurrences as a multiplier
// TODO have logging out to usb serial as idle mode behaviour
