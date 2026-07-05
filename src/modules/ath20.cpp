#include "ath20.h"

#include "errors.h"

void ATH::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  if (ath20.begin(wire)) {
    enabled = true;
  } else {
    Errors::logError(Error::ATH20_UNINITIALISED);
  }
}

void ATH::update() {
  if (enabled) {
    sensors_event_t humidity, temp;
    ath20.getEvent(&humidity, &temp);
    sensorState->relative_humidity = humidity.relative_humidity;
    sensorState->ambient_temperature_2 = temp.temperature;
  }
}
