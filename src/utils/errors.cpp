#include "utils/errors.h"

// TODO extend to have info logging as well, ie log "TIME UPDATED FROM GPS etc"

void Errors::init() {
  // TODO create & open logfile with time as name.s
  // SD.open(time.timestamp()+".log", FILE_WRITE) use DateTime::TIMESTAMP_TIME for file DateTime::TIMESTAMP_DATE for folder
}

void Errors::logError(Error error) {
  errors[errors_head] = ErrorEvent{error, millis()};

  flushToLogfile(errors[errors_head]);

  errors_head++;
  if (errors_head >= CACHED_ERRORS) {
    errors_head = 0;
  }
}

String Errors::errorDescription(Error error) {
  switch (error) {
    case Error::UNKNOWN:
      return String(F("An unknown error occurred!"));
    case Error::BMP280_UNINITIALISED:
      return String(F("The BMP280 Pressure Sensor failed to initialise!"));
    case Error::ATH20_UNINITIALISED:
      return String(F("The ATH20 Humidity Sensor failed to initialise!"));
    case Error::RTC_UNINITIALISED:
      return String(F("The Realtime Clock failed to initialise!"));
    case Error::RTC_LOST_POWER:
      return String(F("The Realtime Clock lost power. Time is inaccurate!"));
    case Error::IMU_UNINITIALISED:
      return String(F("The IMU failed to initialise!"));
    case Error::IMU_SAMPLE_RATE_ERR:
      return String(F("The IMU failed to set the sample rate!"));
    case Error::IMU_INTERRUPT_ERR:
      return String(F("The IMU failed to enable data interrupt!"));
    case Error::IMU_DATA_READ_FAILED:
      return String(F("An IMU read failed... This may be intermittent."));
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::NVRAM_UNINITIALISED:
      return String(F("The NVRAM failed to initialise!"));
    case Error::I2C_TIMEOUT:
      return String(F("I2C timeout occurred!"));
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    case Error::EEPROM_UNINITIALISED:
      return String(F("The EEPROM failed to initialise!"));
    case Error::SETTINGS_VERSION_CHANGED:
      return String(F("The settings version has changed! Resetting!"));
    case Error::SETTINGS_CHECKSUM_FAILED:
      return String(F("The settings are corrupt! Resetting!"));
    case Error::SETTINGS_VOLATILE:
      return String(F("The settings aren't being saved! No EEPROM!"));
    case Error::NO_TEMPS_FOUND:
      return String(F("No DS18B20 temperature sensors found!"));
    case Error::INCORRECT_NUM_TEMPS:
      return String(F("Incorrect number of DS18B20 temperature sensors!"));
    case Error::GHOST_TEMP_PROBE:
      return String(F("(Ghost) OneWire temperature sensor didn't respond!"));
    case Error::TEMP_RESOLUTION_INCORRECT:
      return String(F("Failed to set temperature resolution on DS18B20!"));
    case Error::DUPLICATE_TEMP_SENSOR_IDS:
      return String(F("DS18B20 sensors detected with duplicate IDs!"));
    case Error::TEMP_UNREADABLE:
      return String(F("DS18B20 sensor temperature unreadable!"));
      // TODO this could fire very quickly. Need to have a cap of some kind to not fill up everything.
    default:
      return String(F("An undefined error occurred!"));
  }
}

void Errors::flushToLogfile(const ErrorEvent& errorEvent) {
  // TODO write out data to logfile in csv
}

// TODO maybe just do 5s debounce for errors? Log multiple occurrences as a multiplier
// TODO have logging out to usb serial as idle mode behaviour
