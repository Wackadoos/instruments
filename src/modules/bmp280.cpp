#include "bmp280.h"

#include "utils/errors.h"

Adafruit_BMP280 BMP::bmp280 = Adafruit_BMP280();
IntervalMetric BMP::dataProcessTime = IntervalMetric();

void BMP::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  bmp280 = Adafruit_BMP280(wire);  // Recreate object with wire now we have it

  if (!bmp280.begin()) {
    Errors::logError(Error::BMP280_UNINITIALISED);
    return;
  }

  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,    /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,    /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X8,    /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_X8,      /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_63); /* Standby time. */

  dataProcessTime.init(F("BMP Proc"), F("Time to process BMP Data"));
  enabled = true;
}

void BMP::update() {
  if (enabled) {
    dataProcessTime.start();
    sensorState->ambient_temperature = bmp280.readTemperature();
    sensorState->uncalibrated_altitude = bmp280.readAltitude();
    // bmp280.readPressure();
    dataProcessTime.stop();
  }
}
