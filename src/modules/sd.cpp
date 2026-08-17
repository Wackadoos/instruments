#include "sd.h"

#include "hardware.h"
#include "modules/rtc.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

SPIClass* SD::spi = nullptr;
SdExFat SD::card = SdExFat();
ExFile SD::file = ExFile();
RingBuf<ExFile, 512> SD::buffer = RingBuf<ExFile, 512>();
IntervalMetric SD::dataProcessTime = IntervalMetric();
char SD::fileName[LOG_FILE_NAME_SIZE] = "";

void SD::init(SPIClass* spi_ref) {
  spi = spi_ref;

  if (!init_card()) {
    return;
  }

  dataProcessTime.init(F("SD Write"), F("Time to write to the SD card"));
  enabled = true;
}

bool SD::init_card() {
  if (!card.begin(SdSpiConfig(SD_CARD_CHIP_SELECT_PIN, SHARED_SPI, SD_SPI_SPEED, spi))) {
    Errors::logError(Error::SD_UNINITIALISED);
    return false;
  }
  return true;
}

bool SD::openLogFile() {
  // "<date-time>.bin" from the RTC, appending a numeric suffix if the name exists.
  DateTime now = RTC::clock.now();
  char base[LOG_FILE_NAME_SIZE];
  snprintf(base, sizeof(base), "%04d-%02d-%02d_%02d%02d%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

  for (uint8_t n = 0; n < 99; n++) {
    if (n == 0) {
      snprintf(fileName, sizeof(fileName), "%s.bin", base);
    } else {
      snprintf(fileName, sizeof(fileName), "%s_%u.bin", base, n);
    }
    if (!card.exists(fileName)) break;
  }

  if (!file.open(fileName, O_WRONLY | O_CREAT)) {
    Errors::logError(Error::SD_OPEN_FAILED);
    return false;
  }

  if (!file.preAllocate(LOG_PREALLOC_BYTES)) {
    Errors::logError(Error::SD_PREALLOCATE_FAILED);
    file.close();
    return false;
  }

  const uint8_t header[12] = {'I', 'N', 'S', 'T', 'R', 'L', 'O', 'G', 1, 0, 0, 0};
  if (file.write(header, sizeof(header)) != sizeof(header)) {
    Errors::logError(Error::SD_WRITE_FAILED);
    file.close();
    return false;
  }

  file.sync();  // Ensure header is immediately written
  return true;
}

void SD::startLog() {
  if (!enabled) return;

  auto started = openLogFile();
  if (!started) {
    // Try re-initialising the card and opening new log file
    if (!init_card() || !openLogFile()) {
      Errors::logError(Error::SD_STALLED);
      return;
    }
  }

  buffer.begin(&file);
  logging = true;
  Logging::logDebug(F("SD log started: "), fileName);
}

void SD::stopLog() {
  if (!enabled) return;

  buffer.sync();    // Flush whatever remains in the ring buffer
  file.sync();      // Sync all data & dir fields to the card
  file.truncate();  // Trim the preallocated space back to the written data
  file.close();
  logging = false;
  Logging::logDebug(F("SD log stopped: "), fileName);
}

void SD::logFrame() {
  if (!logging) return;

  struct Frame {
    uint16_t magic;
    uint32_t epoch;
    uint32_t millis;
    LogPayload payload;
  };
  Frame frame;
  frame.magic = LOG_MAGIC;
  frame.epoch = RTC::clock.now().secondstime();
  frame.millis = millis();

  LogPayload& p = frame.payload;
  p.mode = (uint8_t)State::currentMode;
  p.temp_motor_1 = State::temp_motor_1;
  p.temp_motor_2 = State::temp_motor_2;
  p.imu_accel_x = State::imu_accel_x;
  p.imu_accel_y = State::imu_accel_y;
  p.imu_accel_z = State::imu_accel_z;
  p.imu_die_temp = State::imu_die_temp;
  p.max_1s_acceleration = State::max_1s_acceleration;
  p.max_1s_accel_x = State::max_1s_accel_x;
  p.max_1s_accel_y = State::max_1s_accel_y;
  p.max_1s_accel_z = State::max_1s_accel_z;
  p.uncalibrated_altitude = State::uncalibrated_altitude;
  p.ambient_temperature = State::ambient_temperature;
  p.relative_humidity = State::relative_humidity;
  p.ambient_temperature_2 = State::ambient_temperature_2;
  p.kilometers_per_hour = State::kilometers_per_hour;
  p.motor_current = State::motor_current;
  p.battery_current = State::battery_current;
  p.battery_power = State::battery_power;
  p.duty_cycle = State::duty_cycle;
  p.battery_voltage = State::battery_voltage;
  p.watts_used = State::watts_used;
  p.watts_charged = State::watts_charged;
  p.esc_temp = State::esc_temp;
  p.gps_speed = State::gps_speed;
  p.heading = State::heading;
  p.battery_soc_compensated = State::battery_soc_compensated;
  p.battery_time_remaining_mins = State::battery_time_remaining_mins;
  p.latitude = State::latitude;
  p.longitude = State::longitude;
  p.altitude = State::altitude;
  p.visible_satellites = State::visible_satellites;
  p.fix_satellites = State::fix_satellites;
  p.ram_free_bytes = State::ram_free_bytes;
  p.ram_free_bytes_minimum = State::ram_free_bytes_minimum;
  p.reserved = 0;

  // Fast memcpy into the ring; the actual SPI write happens in run() when the card is idle.
  if (buffer.write(&frame, sizeof(frame)) != sizeof(frame)) {
    Errors::logError(Error::SD_LOG_OVERRUN);  // Card busy too long; this frame was dropped
  }
}

void SD::run() {
  if (!enabled) return;
  dataProcessTime.start();

  // Drain everything currently buffered to card cache, but only when the card reports idle so we never busy-wait on a flash erase/program operation inside loop. Writing as soon as data is available keeps the small ring near-empty and minimises data lost to a power cut. Card flushes it's internal sector cache to disk when full.
  if (buffer.bytesUsed() && !file.isBusy()) {
    if (buffer.writeOut(buffer.bytesUsed()) == 0) {
      Errors::logError(Error::SD_WRITE_FAILED);
    }
  }

  dataProcessTime.stop();
}
