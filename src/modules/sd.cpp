#include "sd.h"

#include "hardware.h"
#include "modules/rtc.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

SdExFat SD::card = SdExFat();
ExFile SD::file = ExFile();
RingBuf<ExFile, 512> SD::buffer = RingBuf<ExFile, 512>();
IntervalMetric SD::dataProcessTime = IntervalMetric();
char SD::fileName[LOG_FILE_NAME_SIZE] = "";

// Two write failures inside this window mean the card is unhealthy, not just
// briefly wedged; logging is abandoned instead of restarting a new file each
// time (which would churn out many small files during a race).
static const uint32_t kMinRecoveryIntervalMs = 5000;
static const uint8_t kInitAttempts = 3;
static const uint16_t kInitRetryDelayMs = 100;
static const uint8_t kStartLogAttempts = 3;
static const uint16_t kStartLogRetryDelayMs = 25;

bool SD::ensureCardInited() {
  if (enabled) return true;

  // Marginal cards occasionally fail the first init at boot but succeed on a
  // retry. Re-try lazily from startLog() too, so a boot hiccup doesn't disable
  // logging for the whole session.
  for (uint8_t i = 0; i < kInitAttempts; i++) {
    if (card.begin(SD_CARD_CHIP_SELECT_PIN, SD_SPI_SPEED)) {
      enabled = true;
      return true;
    }
    delay(kInitRetryDelayMs);
  }
  return false;
}

void SD::init() {
  // Shared SPI: display/touch/SD all use the same bus with separate chip selects.
  dataProcessTime.init(F("SD Write"), F("Time to write to the SD card"));
  if (!ensureCardInited()) {
    Errors::logError(Error::SD_UNINITIALISED);
  }
}

bool SD::openLogFile() {
  // "<date-time>.bin" from the RTC, appending a numeric suffix if the name exists.
  DateTime now = RTC::clock.now();
  char base[LOG_FILE_NAME_SIZE];
  snprintf(base, sizeof(base), "%04d-%02d-%02d_%02d%02d%02d", now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());

  for (uint8_t n = 0; true; n++) {
    if (n == 0) {
      snprintf(fileName, sizeof(fileName), "%s.bin", base);
    } else {
      snprintf(fileName, sizeof(fileName), "%s_%u.bin", base, n);
    }
    if (!card.exists(fileName)) break;
  }

  if (!file.open(fileName, O_RDWR | O_CREAT)) {
    Errors::logError(Error::SD_OPEN_FAILED);
    return false;
  }

  // Preallocate BEFORE writing anything: on exFAT a write allocates the file's
  // first cluster, and ExFatFile::preAllocate() then bails out (m_firstCluster
  // already set) so the allocation would always fail.
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
  return true;
}

void SD::startLog() {
  if (!enabled || logging) return;

  if (!ensureCardInited()) {
    Errors::logError(Error::SD_UNINITIALISED);
    return;
  }

  for (uint8_t attempt = 0; attempt < kStartLogAttempts; attempt++) {
    if (openLogFile()) {
      buffer.begin(&file);
      logging = true;
      Logging::logDebug(F("SD log started: "), fileName);
      return;
    }
    // A failed open/write may have left the card wedged (SdFat aborts a
    // multi-block write without sending STOP_TRAN, so the card holds its busy
    // line). Re-running card init (a CMD0 reset) unsticks it; drop the blank
    // file it left behind, then space out before retrying.
    card.begin(SD_CARD_CHIP_SELECT_PIN, SD_SPI_SPEED);
    file.remove();
    delay(kStartLogRetryDelayMs);
  }

  Errors::logError(Error::SD_STALLED);
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
  if (!logging) return;
  dataProcessTime.start();

  bool stalled = false;
  // Drain everything currently buffered, but only when the card reports idle so
  // we never busy-wait on a flash erase/program operation inside loop(). Writing
  // as soon as data is available keeps the small ring near-empty and minimises
  // data lost to a power cut.
  while (buffer.bytesUsed() && !file.isBusy()) {
    if (buffer.writeOut(buffer.bytesUsed()) == 0) {
      Errors::logError(Error::SD_WRITE_FAILED);
      stalled = true;
      break;
    }
  }

  // Unconditional: SdFat buffers data in a single 512-byte sector cache that
  // only reaches the card when it fills up (or on sync), so this forces out a
  // partial sector each cycle at ~2-5 ms. It also converts a wedged card into
  // a detectable failure: the isBusy() gate must not skip it, or a card stuck
  // holding its busy line would silently stop the log forever.
  if (!stalled && !file.sync()) {
    Errors::logError(Error::SD_WRITE_FAILED);
    stalled = true;
  }

  if (stalled) {
    recoverLog();
  }
  dataProcessTime.stop();
}

void SD::recoverLog() {
  // A failed sector write aborts mid multi-block transfer: SdFat's error path
  // never sends STOP_TRAN, leaving the card holding its busy line and the log
  // permanently wedged. Re-running card init (a CMD0 reset) unsticks it.
  if (millis() - lastRecoveryMillis < kMinRecoveryIntervalMs) {
    // A second failure inside the recovery window: the card is genuinely
    // struggling, so abandon this race's logging rather than churn out files.
    Errors::logError(Error::SD_STALLED);
    logging = false;
    file.close();
    return;
  }
  lastRecoveryMillis = millis();

  if (!card.begin(SD_CARD_CHIP_SELECT_PIN, SD_SPI_SPEED)) {
    Errors::logError(Error::SD_STALLED);
    logging = false;
    file.close();
    return;
  }

  // Card healthy again: finish the current file (flush the recovered cache and
  // trim the preallocation) and continue in a fresh one.
  stopLog();
  startLog();
}

void SD::stopLog() {
  if (!logging) return;

  buffer.sync();        // Flush whatever remains in the ring buffer
  file.truncate();      // Trim the preallocated space back to the written data
  file.sync();
  file.close();
  logging = false;
  Logging::logDebug(F("SD log stopped: "), fileName);
}
