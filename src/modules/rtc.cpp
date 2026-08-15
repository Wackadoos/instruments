#include "rtc.h"

#include "modules/GPS.h"
#include "settings.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

RTC_DS3231 RTC::clock = RTC_DS3231();
IntervalMetric RTC::dataProcessTime = IntervalMetric();

void RTC::init(TwoWire* wire) {
  if (!clock.begin(wire)) {
    Errors::logError(Error::RTC_UNINITIALISED);
    return;
  }

  if (clock.lostPower()) {
    Errors::logError(Error::RTC_LOST_POWER);
    needs_adjust = true;
  }

  dataProcessTime.init(F("RTC Proc"), F("Time to process RTC Data"));
  enabled = true;
}

void RTC::update() {
  if (enabled) {
    dataProcessTime.start();
    if (GPS::dateTimeCalibrated) {
      auto GPStime = DateTime(GPS::fix.dateTime.year, GPS::fix.dateTime.month, GPS::fix.dateTime.date, GPS::fix.dateTime.hours, GPS::fix.dateTime.minutes, GPS::fix.dateTime.seconds);
      auto diff = clock.now() - GPStime;

      if (abs(diff.totalseconds()) > 10) {
        clock.adjust(GPStime);
        Errors::logError(Error::RTC_OUT_OF_SYNC);
      } else if (needs_adjust) {
        clock.adjust(GPStime);
        needs_adjust = false;
      }
    }
    char buffer[] = "hh:mmap";
    State::currentTime = (clock.now() + TimeSpan(0, SETTINGS::getSettings().timezone_offset / 4, (SETTINGS::getSettings().timezone_offset % 4) * 15, 0)).toString(buffer);
    Logging::logDebug(F("RTC Time: "), clock.now().timestamp(DateTime::TIMESTAMP_FULL));
    dataProcessTime.stop();
  }
}
