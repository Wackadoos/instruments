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
  float temp_motor_front = 0;  // Motor temp (front probe) in degrees C
  float temp_motor_back = 0;   // Motor temp (back probe) in degrees C
  //* Inertial Measurement Unit Accelerations & Temp
  float imu_accel_x = 0;          // Acceleration in the X axis (m/s/s)
  float imu_accel_y = 0;          // Acceleration in the Y axis (m/s/s)
  float imu_accel_z = 0;          // Acceleration in the Z axis (m/s/s)
  float imu_die_temp = 0;         // IMU chip temperature
  float max_1s_acceleration = 0;  // Max sustained acceleration 1 second
  //* BMP280 Atmospheric Pressure & Temperature Sensor
  float uncalibrated_altitude = 0;  // Altitude assuming a standard day (1013.25 hPa Sea Level)
  float ambient_temperature = 0;    // Ambient temp according to BMP280 in degrees C
  //* ATH20 Humidity & Temperature Sensor
  float relative_humidity = 0;      // Relative Humidity in % rH
  float ambient_temperature_2 = 0;  // Ambient temp according to ATH20 in degrees C
  //* Optical Gate Speed Sensor
  float kilometers_per_hour = 0;  // Speed in km/h
  //* Vesc
  float motor_current; // Current through the windings (during duty cycle on period ONLY)
  float battery_current; // Current into ESC (overall average regardless duty cycle)
  float duty_cycle; // Duty cycle right now
  float battery_voltage; // Battery Voltage
  float watts_used; // Total WH consumed
  float watts_charged; // Total WH charged
  float esc_temp; // Temperature of ESC MOSFET Chips
  //* System Stats
  uint16_t ram_free_bytes_minimum = 0;  // Least recorded available ram
  uint32_t longest_scheduler_isr = 0;   // Longest recorded scheduler ISR
  // TODO logging of percentages cpu used by each task & longest task executions before yield
};
