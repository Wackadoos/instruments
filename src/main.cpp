#include "main.h"

#include "hardware.h"
#include "modules/GPS.h"
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
#include "utils/memory.h"
#include "utils/scheduler.h"

IntervalMetric mainLoopTime = IntervalMetric();

void logData();
void reportRam() {
  Serial.print(F("Free RAM = "));  // F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(State::ram_free_bytes);  // print how much RAM is available in bytes.
}

ScheduledTask tasks[] = {
    // ScheduledTask(500, 0, displayBattStats),
    // ScheduledTask(500, 0, displayLapTiming),
    ScheduledTask(200, 0, reportRam),
    ScheduledTask(100, 5, SPEED::update),
    ScheduledTask(250, 10, VESC::update),
    ScheduledTask(500, 100, ATH::update),
    ScheduledTask(500, 200, BMP::update),
    ScheduledTask(500, 300, TEMPS::update),
    ScheduledTask(1000, 15, RTC::update),
    ScheduledTask(10000, 37, State::saveRaceState),
    // ScheduledTask(500, 400, logData),
};

void setup() {
  Errors::init();

  //! Setup SD class first before errors class

  HARDWARE::init();

  SETTINGS::init();

  SPEED::configure(SETTINGS::getSettings().speed_sensor_wheel_circumference, SETTINGS::getSettings().speed_sensor_pulses_per_revolution);

  State::tryResumeRace();

  mainLoopTime.init(F("Main Loop"), F("Interval measurement of main loop"));

  Scheduler::start(tasks);
}

void loop() {
  State::ram_free_bytes = freeMemory();  // Fixed measurement point so all consumers agree (display, serial, min)
  if (State::ram_free_bytes < State::ram_free_bytes_minimum) {
    State::ram_free_bytes_minimum = State::ram_free_bytes;
  }

  mainLoopTime.start();  // TODO maybe have a minimum threshold on this? So a busy-wait isn't included
  Scheduler::runTasks();
  HARDWARE::run();
  TEMPS::run();  // Runs once conversion is complete
  IMU::run();    // Internally scheduled via data ready interrupt
  GPS::run();    // Runs when new fix available
  Display::run();

  State::runMode();
  // for dev track framerate of display updates for different values?
  mainLoopTime.stop();
}
