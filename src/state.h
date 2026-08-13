#pragma once

#include <Arduino.h>

enum class AppState : uint8_t {
  IDLE,
  RACE,
  DEBUG,
};

struct SensorState {
 public:
  //* DS18B20 Temp Sensors (Motor)
  static inline float temp_motor_front = 78;  // Motor temp (front probe) in degrees C
  static inline float temp_motor_back = 52;   // Motor temp (back probe) in degrees C
  //* Inertial Measurement Unit Accelerations & Temp
  static inline float imu_accel_x = 0;          // Acceleration in the X axis (m/s/s)
  static inline float imu_accel_y = 0;          // Acceleration in the Y axis (m/s/s)
  static inline float imu_accel_z = 0;          // Acceleration in the Z axis (m/s/s)
  static inline float imu_die_temp = 0;         // IMU chip temperature
  static inline float max_1s_acceleration = 0;  // Max sustained acceleration 1 second
  //* BMP280 Atmospheric Pressure & Temperature Sensor
  static inline float uncalibrated_altitude = 0;  // Altitude assuming a standard day (1013.25 hPa Sea Level)
  static inline float ambient_temperature = 0;    // Ambient temp according to BMP280 in degrees C
  //* ATH20 Humidity & Temperature Sensor
  static inline float relative_humidity = 0;      // Relative Humidity in % rH
  static inline float ambient_temperature_2 = 0;  // Ambient temp according to ATH20 in degrees C
  //* Optical Gate Speed Sensor
  static inline float kilometers_per_hour = 34;  // Speed in km/h
  //* Vesc
  static inline float motor_current = 0;    // Current through the windings (during duty cycle on period ONLY)
  static inline float battery_current = 0;  // Current into ESC (overall average regardless duty cycle)
  static inline float battery_power = 482;  // Power currently being used (battery_voltage * battery_current)
  static inline float duty_cycle = 0;       // Duty cycle right now
  static inline float battery_voltage = 0;  // Battery Voltage
  static inline float watts_used = 0;       // Total WH consumed
  static inline float watts_charged = 0;    // Total WH charged
  static inline float esc_temp = 34;        // Temperature of ESC MOSFET Chips
  //* GPS
  static inline uint8_t visible_satellites = 11;  // Number of satellites currently in view
  //* Timing
  static inline String currentTime = "8:58pm";
  static inline String splitDiff = "+00:06:42";
  //* Battery
  static inline String battery_stats = "65% 104m";
  static inline float battery_percentage_remaining = 0;  // Battery percentage remaining estimate
  static inline float battery_time_remaining = 0;        // Time estimate remaining in seconds
  //* System Stats
  static inline uint16_t ram_free_bytes_minimum = 0;  // Least recorded available ram
  static inline uint32_t longest_scheduler_isr = 0;   // Longest recorded scheduler ISR
  // TODO logging of percentages cpu used by each task & longest task executions before yield
};
