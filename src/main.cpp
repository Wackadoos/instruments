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
#include "widgets.h"

AppState currentState = AppState::IDLE;
IntervalMetric mainLoopTime = IntervalMetric();

void logData();

void setspeed() {
  SensorState::kilometers_per_hour = random(max(0, SensorState::kilometers_per_hour - 2), min(99, SensorState::kilometers_per_hour + 2));
  SensorState::battery_power = random(max(-500, SensorState::battery_power - 50), min(1500, SensorState::battery_power + 50));
  // Serial.print("writetime: ");
  // Serial.println(Display::dataProcessTime.average());
  // Serial.print("writetimelongest: ");
  // Serial.println(Display::dataProcessTime.longest());
}

ScheduledTask tasks[] = {
    // ScheduledTask(25, 0, displayCritical),
    // ScheduledTask(50, 0, readTouchScreen),
    // ScheduledTask(100, 0, readGPS),
    // ScheduledTask(500, 0, displayBattStats),
    // ScheduledTask(20, 0, SPEED::update),
    // ScheduledTask(100, 0, VESC::update),
    ScheduledTask(500, 0, RTC::update),
    ScheduledTask(500, 100, ATH::update),
    ScheduledTask(500, 200, BMP::update),
    ScheduledTask(500, 300, setspeed),
    // ScheduledTask(500, 300, TEMPS::update),
    // ScheduledTask(500, 400, logData),
};

void setup() {
  Errors::init();

  //! Setup SD class first before errors class

  HARDWARE::init();

  SETTINGS::init();

  mainLoopTime.init(F("Main Loop"), F("Interval measurement of main loop"));

  Scheduler::start(tasks);
}

void loop() {
  mainLoopTime.start();  // TODO maybe have a minimum threshold on this? So a busy-wait isn't included
  Scheduler::runTasks();
  HARDWARE::run();
  // TEMPS::run();
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

void displayCritical() {
  RTC::update();
  // 10ms
  // - Speed
  // - Laptime etc

  // 50ms
  // - Power Usage (Watts)
  // - Battery Voltage
  // - Touchscreen
  // - (dev) framerate

  // 100ms
  // - GPS Speed

  // 500ms
  // - Motor Temps
  // - Percent Remaining
  // - Estimated Empty
  // - Relative altitude
}

void readTouchScreen() {};

void readGPS() {};

void displayBattStats() {
  // Percent Remaining & Estimated time till empty
};

void logData() {
  // timestamp
  // pressure
  // humidity
  // general temp
  // GPS
  // Speed
  // Acceleration
  // Amps
  // Power Usage (Watts)
  // Batt voltage
};
