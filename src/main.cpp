#include "main.h"

#include "hardware.h"
#include "modules/ath20.h"
#include "modules/bmp280.h"
#include "modules/display.h"
#include "modules/imu.h"
#include "modules/rtc.h"
#include "modules/speed.h"
#include "modules/temps.h"
#include "modules/vesc.h"
#include "settings.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/scheduler.h"

AppState currentState = AppState::IDLE;
IntervalMetric mainLoopTime = IntervalMetric();

void logData();

ScheduledTask tasks[] = {
    // ScheduledTask(100, 0, readGPS),
    // ScheduledTask(500, 0, displayBattStats),
    ScheduledTask(100, 5, SPEED::update),
    ScheduledTask(250, 10, VESC::update),
    ScheduledTask(500, 100, ATH::update),
    ScheduledTask(500, 200, BMP::update),
    ScheduledTask(500, 300, TEMPS::update),
    ScheduledTask(1000, 15, RTC::update),
    // ScheduledTask(500, 400, logData),
};

void setup() {
  Errors::init();

  //! Setup SD class first before errors class

  HARDWARE::init();

  SETTINGS::init();

  SPEED::configure(SETTINGS::getSettings().speed_sensor_wheel_circumference, SETTINGS::getSettings().speed_sensor_pulses_per_revolution);

  mainLoopTime.init(F("Main Loop"), F("Interval measurement of main loop"));

  Scheduler::start(tasks);
}

void loop() {
  mainLoopTime.start();  // TODO maybe have a minimum threshold on this? So a busy-wait isn't included
  Scheduler::runTasks();
  HARDWARE::run();
  TEMPS::run();
  IMU::run();  // Internally scheduled via data ready interrupt
  Display::run();

  switch (currentState) {
    case AppState::IDLE:
      break;
    case AppState::RACE:
      break;
    case AppState::DEBUG:
      break;
  }
  // for dev track framerate of display updates for different values?
  mainLoopTime.stop();
}
