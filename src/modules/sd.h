#pragma once

#include <Arduino.h>
#include <SdFat.h>

#include "RingBuf.h"
#include "utils/metrics.h"

//! MicroSD binary logging of the shared State struct.
//! Format (little-endian, AVR native):
//!   12-byte file header: "INSTRLOG" + u16 format version + u16 reserved
//!   frames, each LOG_FRAME_BYTES long:
//!     u16 LOG_MAGIC   - frame delimiter / corruption check
//!     u32 epoch       - RTC seconds since 1970-01-01 UTC
//!     u32 millis      - ms since boot (intra-second resolution)
//!     LogPayload      - snapshot of the numeric State members
//! Frames are fixed-size, so the decoder validates LOG_MAGIC per frame and
//! stops/resyncs on the first mismatch (truncated write, preallocated tail).

#define LOG_MAGIC 0x4C47  // "LG"
#define LOG_FORMAT_VERSION 1
#define LOG_PAYLOAD_BYTES 128
#define LOG_FRAME_HEADER_BYTES 10  // magic u16 + epoch u32 + millis u32
#define LOG_FRAME_BYTES (LOG_FRAME_HEADER_BYTES + LOG_PAYLOAD_BYTES)
#define LOG_SAMPLES_PER_SECOND 2  // Scheduled task runs every 500ms
#define LOG_DURATION_SECONDS (3UL * 60 * 60)
#define LOG_PREALLOC_BYTES ((uint64_t)LOG_DURATION_SECONDS * LOG_SAMPLES_PER_SECOND * LOG_FRAME_BYTES)
#define LOG_FILE_NAME_SIZE 48

//! SPI clock for the SD card. We only log ~280 B/s so this is deliberately far below the 8 MHz AVR maximum (F_CPU/2):
#define SD_SPI_SPEED SD_SCK_MHZ(4)

// Fixed-layout snapshot of every numeric State member (char arrays excluded). sizeof() is exactly LOG_PAYLOAD_BYTES on AVR (no struct padding).
struct LogPayload {
  uint8_t mode;  // AppMode::RACE during logging
  float temp_motor_1;
  float temp_motor_2;
  float imu_accel_x;
  float imu_accel_y;
  float imu_accel_z;
  float imu_die_temp;
  float max_1s_acceleration;
  float max_1s_accel_x;
  float max_1s_accel_y;
  float max_1s_accel_z;
  float uncalibrated_altitude;
  float ambient_temperature;
  float relative_humidity;
  float ambient_temperature_2;
  float kilometers_per_hour;
  float motor_current;
  float battery_current;
  float battery_power;
  float duty_cycle;
  float battery_voltage;
  float watts_used;
  float watts_charged;
  float esc_temp;
  float gps_speed;
  float heading;
  float battery_soc_compensated;
  float battery_time_remaining_mins;
  int32_t latitude;
  int32_t longitude;
  int32_t altitude;
  uint8_t visible_satellites;
  uint8_t fix_satellites;
  uint16_t ram_free_bytes;
  uint16_t ram_free_bytes_minimum;
  uint8_t reserved;
};
static_assert(sizeof(LogPayload) == LOG_PAYLOAD_BYTES, "LogPayload layout mismatch");

class SD {
 public:
  static void init(SPIClass* spi);
  static void startLog();
  static void stopLog();
  static void logFrame();
  static void run();

 private:
  static bool init_card();
  static bool openLogFile();
  static SPIClass* spi;
  static SdExFat card;
  static ExFile file;
  static RingBuf<ExFile, 512> buffer;
  static IntervalMetric dataProcessTime;
  static char fileName[LOG_FILE_NAME_SIZE];

  inline static bool enabled = false;
  inline static bool logging = false;
};
