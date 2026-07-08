#include "ath20.h"

#include "utils/errors.h"

Adafruit_AHTX0 ATH::ath20 = Adafruit_AHTX0();
IntervalMetric ATH::dataProcessTime = IntervalMetric();

void ATH::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  if (!ath20.begin(wire)) {
    Errors::logError(Error::ATH20_UNINITIALISED);
    return;
  }

  dataProcessTime.init(F("ATH Proc"), F("Time to process ATH Data"));
  enabled = true;
}

void ATH::update() {
  if (enabled) {
    dataProcessTime.start();
    sensors_event_t humidity, temp;
    ath20.getEvent(&humidity, &temp);
    sensorState->relative_humidity = humidity.relative_humidity;
    sensorState->ambient_temperature_2 = temp.temperature;
    dataProcessTime.stop();
  }
}
