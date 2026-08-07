#pragma once

#include <OneWire.h>

#include "state.h"

//! Arduino Mega Pin Allocation
#define GPS_PULSE_PER_SECOND_PIN 2     // INT4
#define ACCELEROMETER_INTERRUPT_PIN 3  // INT5
#define DISPLAY_BACKLIGHT_PIN 4        // High Frequency PWM
//* Hardware Serial 3 (14, 15) -> GPS Module (ATGM336H)
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

class HARDWARE {
 public:
  static void init(SensorState* state);
  static void run();

 private:
  static OneWire oneWire;
};
