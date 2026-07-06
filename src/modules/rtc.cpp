#include "rtc.h"

#include "settings.h"
#include "utils/errors.h"

void RTC::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  if (rtc.begin(wire)) {
    enabled = true;
    if (rtc.lostPower()) {
      Errors::logError(Error::RTC_LOST_POWER);
      needs_adjust = true;
    }
  } else {
    Errors::logError(Error::RTC_UNINITIALISED);
  }
}

void RTC::update() {
  if (needs_adjust) {
    // TODO
    adjust();
  }
}

void RTC::adjust() {
  // TODO make this only work validly if GPS lock is acquired
  SETTINGS::getSettings().timezone_offset;
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

// TODO have a warning on screen if RTC was too far out of sync when GPS time acquired. (Also have temporary time correction seconds popup?). That way we know when battery isn't working.
