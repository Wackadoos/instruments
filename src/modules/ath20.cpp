#include "ath20.h"

#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

Adafruit_AHTX0 ATH::ath20 = Adafruit_AHTX0();
IntervalMetric ATH::dataProcessTime = IntervalMetric();

void ATH::init(TwoWire* wire) {
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
    SensorState::relative_humidity = humidity.relative_humidity;
    Logging::logDebug(F("ATH Humidity: "), humidity.relative_humidity);
    SensorState::ambient_temperature_2 = temp.temperature;
    Logging::logDebug(F("ATH Temp: "), temp.temperature);
    dataProcessTime.stop();
  }
}
