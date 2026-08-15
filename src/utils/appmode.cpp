#include "state.h"

#include "modules/battery.h"
#include "modules/rtc.h"
#include "settings.h"
#include "widgets.h"

bool State::isRace() {
  return currentMode == AppMode::RACE;
}

void State::enterMode(AppMode next) {
  if (next == currentMode) return;

  bool leavingRace = currentMode == AppMode::RACE;
  currentMode = next;

  if (next == AppMode::RACE) {
    RaceButton::button.setText("STOP\nRACE");
    RaceButton::button.setColor(RGB565_BROWN);
  } else {
    RaceButton::button.setText("START\nRACE");
    RaceButton::button.setColor(RGB565_BLUEVIOLET);
  }

  if (leavingRace) {
    auto settings = SETTINGS::getSettings();
    settings.race_mode_last_save_seconds = 0;
    SETTINGS::pushSetting(settings);
  }
}

void State::runMode() {
  switch (currentMode) {
    case AppMode::IDLE:
      break;
    case AppMode::RACE:
      break;
    case AppMode::DEBUG:
      break;
  }
}

void State::saveRaceState() {
  if (isRace() && !RTC::needs_adjust) {
    auto settings = SETTINGS::getSettings();
    settings.race_mode_last_save_seconds = RTC::clock.now().secondstime();
    settings.battery_I_avg = BatteryEstimator::I_avg;
    settings.totalDischargedWh_calibration = State::watts_used;
    settings.totalChargedWh_calibration = State::watts_charged;
    SETTINGS::pushSetting(settings);
  }
}

void State::tryResumeRace() {
  auto settings = SETTINGS::getSettings();
  if (!RTC::needs_adjust && (RTC::clock.now().secondstime() - settings.race_mode_last_save_seconds) <= 3600) {
    // If it's less than an hour since we were in race mode, re-engage and bring old values too;
    enterMode(AppMode::RACE);
    BatteryEstimator::init(settings.totalDischargedWh_calibration, settings.totalChargedWh_calibration);
    BatteryEstimator::I_avg = settings.battery_I_avg;
  }
}
