#include "hardware.h"

#include "errors.h"
#include "modules/ath20.h"
#include "modules/bmp280.h"
#include "modules/eeprom.h"
#include "modules/imu.h"
#include "modules/rtc.h"
#include "modules/temps.h"

void HARDWARE::init(SensorState* state) {
  // TODO pin modes

  //* I2C Devices
  Wire.begin();
  Wire.setWireTimeout(25000, true);  // 25ms timeout with auto reset

  EEPROM::init(&Wire);
  RTC::init(&Wire, state);
  BMP::init(&Wire, state);
  ATH::init(&Wire, state);
  IMU::init(&Wire, state);

  // Wire.setClock(400000); // Must be set last! TODO enable after switching to high speed clock

  //* OneWire Devices
  TEMPS::init(&oneWire, state);
}

void HARDWARE::run() {
  if (Wire.getWireTimeoutFlag()) {
    Errors::logError(Error::I2C_TIMEOUT);
    Wire.clearWireTimeoutFlag();
  }
}
