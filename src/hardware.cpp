#include "hardware.h"

#include "errors.h"
#include "modules/ath20.h"
#include "modules/bmp280.h"
#include "modules/eeprom.h"
#include "modules/imu.h"
#include "modules/rtc.h"
#include "modules/temps.h"

void HARDWARE::init(SensorState* state) {
  //* Set pin modes. Some of these may be set by libraries, but there's no harm in setting the same mode twice to be safe.
  pinMode(GPS_PULSE_PER_SECOND_PIN, INPUT);
  pinMode(ACCELEROMETER_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_BACKLIGHT_PIN, OUTPUT);
  pinMode(WHEEL_SPEED_SENSOR_PIN, INPUT);  // Has external pullup
  pinMode(TOUCHSCREEN_INTERRUPT_PIN, INPUT);
  pinMode(ONEWIRE_TEMPERATURE_SENSORS_PIN, INPUT);  // Technically bidirectional, library managed onewire comms pin.
  pinMode(DISPLAY_RESET_PIN, OUTPUT);
  pinMode(DISPLAY_DATA_COMMAND_PIN, OUTPUT);
  pinMode(SD_CARD_CHIP_SELECT_PIN, OUTPUT);
  pinMode(TOUCHSCREEN_CHIP_SELECT_PIN, OUTPUT);
  pinMode(DISPLAY_CHIP_SELECT_PIN, OUTPUT);

  //* I2C Devices
  Wire.begin();
  Wire.setWireTimeout(25000, true);  // 25ms timeout with auto reset

  EEPROM::init(&Wire);
  RTC::init(&Wire, state);
  BMP::init(&Wire, state);
  ATH::init(&Wire, state);
  IMU::init(&Wire, state);

  // Wire.setClock(400000); // Must be set last! TODO enable after switching to high speed RTC chip

  //* OneWire Devices
  TEMPS::init(&oneWire, state);

  //* SPI Devices
}

void HARDWARE::run() {
  if (Wire.getWireTimeoutFlag()) {
    Errors::logError(Error::I2C_TIMEOUT);
    Wire.clearWireTimeoutFlag();
  }
}
