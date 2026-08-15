#pragma once

#include <Arduino.h>

enum class AppMode : uint8_t {
  IDLE,
  RACE,
  DEBUG,
};

struct State {
 public:
  //* General
  static inline AppMode currentMode = AppMode::IDLE;

  // ---- AppMode state machine (definitions in src/utils/appmode.cpp) ----
  static bool isRace();              // Is currentMode RACE?
  static void enterMode(AppMode next);  // Transition: set current mode + associated logic
  static void runMode();             // Per-mode tick, dispatched from loop()
  static void saveRaceState();       // Persist temporary race state (periodic task)
  static void tryResumeRace();       // Boot-time decision + state restore
  //* DS18B20 Temp Sensors (Motor)
  static inline float temp_motor_1 = 0;  // Motor temp (front probe) in degrees C
  static inline float temp_motor_2 = 0;  // Motor temp (back probe) in degrees C
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
  static inline float battery_power = 0;    // Power currently being used (battery_voltage * battery_current)
  static inline float duty_cycle = 0;       // Duty cycle right now
  static inline float battery_voltage = 0;  // Battery Voltage
  static inline float watts_used = 0;       // Total WH consumed
  static inline float watts_charged = 0;    // Total WH charged
  static inline float esc_temp = 0;         // Temperature of ESC MOSFET Chips
  //* GPS
  static inline int32_t latitude = 0;            // GPS Lat, scaled by 10,000,000
  static inline int32_t longitude = 0;           // GPS Long, scaled by 10,000,000
  static inline int32_t altitude = 0;            // GPS Alt in cm
  static inline float gps_speed = 0;             // GPS Speed in km/h
  static inline float heading = 0;               // GPS Heading in km/h
  static inline uint8_t visible_satellites = 0;  // Number of satellites currently in view
  static inline uint8_t fix_satellites = 0;      // Number of satellites used in fix
  static inline String sat_string = "";
  //* Timing
  static inline String currentTime = "";
  static inline String splitDiff = "+~~:~~:~~";
  //* Battery
  static inline float battery_soc_compensated = 0;      // Battery State of Charge accounting for Peukert's Law, Temperature, etc.
  static inline float battery_time_remaining_mins = 0;  // Time estimate remaining in minutes
  static inline String battery_stats = "~% ~m";         // Battery Stats String
  //* System Stats
  static inline uint16_t ram_free_bytes_minimum = 0;  // Least recorded available ram
  static inline uint32_t longest_scheduler_isr = 0;   // Longest recorded scheduler ISR
};
