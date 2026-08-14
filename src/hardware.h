#pragma once

#include <OneWire.h>

//! Arduino Mega Pin Allocation
#define GPS_PULSE_PER_SECOND_PIN 2     // INT4
#define ACCELEROMETER_INTERRUPT_PIN 3  // INT5
#define DISPLAY_BACKLIGHT_PIN 4        // High Frequency PWM
//* Hardware Serial 3 (14, 15) [Uses NeoSerial3] -> GPS Module (ATGM336H)
#define VESC_SERIAL_PORT Serial2   //* Hardware Serial 2 (16, 17) -> ESC (Flipsky 75100)
#define WHEEL_SPEED_SENSOR_PIN 18     // INT3
#define TOUCHSCREEN_INTERRUPT_PIN 19  // INT2
#define ONEWIRE_TEMPERATURE_SENSORS_PIN 42
#define DISPLAY_RESET_PIN 43
#define DISPLAY_DATA_COMMAND_PIN 47
#define SD_CARD_CHIP_SELECT_PIN 48
#define TOUCHSCREEN_CHIP_SELECT_PIN 49
//* SPI (50, 51, 52) -> Display, Touchscreen, SD Card
#define DISPLAY_CHIP_SELECT_PIN 53
#define EEPROM_I2C_ADDR 0x57
#define IMU_I2C_ADDR bfs::Mpu6500::I2C_ADDR_SEC
//* Touchscreen Calibration (resolved via Arduino_GFX TouchCalibration example)
#define TOUCH_SWAP_XY false
#define TOUCH_MAP_X1 3950
#define TOUCH_MAP_X2 150
#define TOUCH_MAP_Y1 3850
#define TOUCH_MAP_Y2 100
//* Setpoint thresholds (warning/error) for SetpointWidgets
inline float POWER_WARNING_SETPOINT = 250.0f;
inline float POWER_ERROR_SETPOINT = 500.0f;
inline float TEMP1_WARNING_SETPOINT = 35.0f;
inline float TEMP1_ERROR_SETPOINT = 45.0f;
inline float ESC_TEMP_WARNING_SETPOINT = 60.0f;
inline float ESC_TEMP_ERROR_SETPOINT = 80.0f;
inline float MOTOR_TEMP_WARNING_SETPOINT = 50.0f;
inline float MOTOR_TEMP_ERROR_SETPOINT = 65.0f;
inline uint8_t GPS_WARNING_SETPOINT = 6;
inline uint8_t GPS_ERROR_SETPOINT = 3;

class HARDWARE {
 public:
  static void init();
  static void run();

 private:
  static OneWire oneWire;
};
