#include "main.h"

#include "hardware.h"
#include "modules/GPS.h"
#include "modules/ath20.h"
#include "modules/bmp280.h"
#include "modules/display.h"
#include "modules/imu.h"
#include "modules/rtc.h"
#include "modules/sd.h"
#include "modules/speed.h"
#include "modules/temps.h"
#include "modules/vesc.h"
#include "settings.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/memory.h"
#include "utils/scheduler.h"

IntervalMetric mainLoopTime = IntervalMetric();

ScheduledTask tasks[] = {
    // ScheduledTask(500, 0, displayBattStats),
    // ScheduledTask(500, 0, displayLapTiming),
    ScheduledTask(200, 0, measureRam),
    ScheduledTask(100, 5, SPEED::update),
    ScheduledTask(250, 10, VESC::update),
    ScheduledTask(500, 100, ATH::update),
    ScheduledTask(500, 200, BMP::update),
    ScheduledTask(500, 300, TEMPS::update),
    ScheduledTask(500, 400, SD::logFrame),
    ScheduledTask(1000, 15, RTC::update),
    ScheduledTask(10000, 37, State::saveRaceState),
};

void setup() {
  Errors::init();

  HARDWARE::init();

  SETTINGS::init();

  SPEED::configure(SETTINGS::getSettings().speed_sensor_wheel_circumference, SETTINGS::getSettings().speed_sensor_pulses_per_revolution);

  State::tryResumeRace();

  mainLoopTime.init(F("Main Loop"), F("Interval measurement of main loop"));

  Scheduler::start(tasks);
}

void loop() {
  mainLoopTime.start();
  Scheduler::runTasks();
  HARDWARE::run();
  TEMPS::run();    // Runs once conversion is complete
  IMU::run();      // Internally scheduled via data ready interrupt
  GPS::run();      // Runs when new fix available
  Display::run();  // Updates displayed widgets
  SD::run();       // Drains the log ring buffer when the card is idle
  mainLoopTime.stop();
}
