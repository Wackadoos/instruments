#include "rtc.h"

#include "settings.h"
#include "utils/logging.h"
#include "utils/errors.h"

RTC_DS3231 RTC::rtc = RTC_DS3231();
IntervalMetric RTC::dataProcessTime = IntervalMetric();

void RTC::init(TwoWire* wire, SensorState* state) {
  sensorState = state;

  if (!rtc.begin(wire)) {
    Errors::logError(Error::RTC_UNINITIALISED);
    return;
  }

  if (rtc.lostPower()) {
    Errors::logError(Error::RTC_LOST_POWER);
    needs_adjust = true;
  }

  dataProcessTime.init(F("RTC Proc"), F("Time to process RTC Data"));
  enabled = true;
}

void RTC::update() {
  if (enabled) {
    dataProcessTime.start();
    if (needs_adjust) {
      // TODO
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // adjust();
    }
    Logging::logDebug(F("RTC Time: "), rtc.now().timestamp(DateTime::TIMESTAMP_FULL));
    dataProcessTime.stop();
  }
}

void RTC::adjust() {
  // TODO make this only work validly if GPS lock is acquired
  SETTINGS::getSettings().timezone_offset;
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

// TODO have a warning on screen if RTC was too far out of sync when GPS time acquired. (Also have temporary time correction seconds popup?). That way we know when battery isn't working.
