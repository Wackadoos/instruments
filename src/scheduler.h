#pragma once

#include <Arduino.h>
#include <TimerOne.h>

struct ScheduledTask {
 public:
  const uint16_t period;            // How often (in ticks) to run the task
  volatile uint16_t ticksUntilRun;  // Decrements every tick, flags task to run at 0, resetting to period
  volatile bool runNow = false;     // Set true by tickISR (during interrupt), cleared once task is run by runTick
  void (*const task)();             // Function pointer to the task you want to run

  ScheduledTask(const uint16_t period, const uint16_t initialOffset, void (*const task)())
      : period(period),
        ticksUntilRun(initialOffset),
        task(task) {};
};

class Scheduler {
 public:
  template <uint8_t N>
  static void start(ScheduledTask (&taskArray)[N]) {
    tasks = taskArray;
    numTasks = N;
    Timer1.initialize(1000);          // A tick is 1ms (1000us)
    Timer1.attachInterrupt(tickISR);  // Run the tick ISR to schedule whichever tasks are needed
  }
  static void runTasks();

 private:
  inline static uint8_t numTasks = 0;            // Only written in init, no need for volatile
  inline static ScheduledTask* tasks = nullptr;  // Only written in init, no need for volatile
  inline volatile static bool taskReady = false;
  static void tickISR();
};
