#include "bmp280.h"

#include "errors.h"

void BMP::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  bmp280 = Adafruit_BMP280(wire);
  if (bmp280.begin()) {
    enabled = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,    /* Operating Mode. */
                       Adafruit_BMP280::SAMPLING_X2,    /* Temp. oversampling */
                       Adafruit_BMP280::SAMPLING_X8,    /* Pressure oversampling */
                       Adafruit_BMP280::FILTER_X8,      /* Filtering. */
                       Adafruit_BMP280::STANDBY_MS_63); /* Standby time. */
  } else {
    Errors::logError(Error::BMP280_UNINITIALISED);
  }
}

void BMP::update() {
  if (enabled) {
    sensorState->ambient_temperature = bmp280.readTemperature();
    sensorState->uncalibrated_altitude = bmp280.readAltitude();
    // bmp280.readPressure();
  }
}
