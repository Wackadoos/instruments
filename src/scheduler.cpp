#include "scheduler.h"

void Scheduler::tickISR() {
  for (uint8_t i = 0; i < numTasks; i++) {
    if (tasks[i].ticksUntilRun == 0) {
      tasks[i].runNow = true;
      tasks[i].ticksUntilRun = tasks[i].period;
      taskReady = true;
    }
    tasks[i].ticksUntilRun--;
  }
}

void Scheduler::runTasks() {
  if (taskReady)  // Checking this first avoids looping over all tasks if nothing is ready anyway
  {
    taskReady = false;  // clear before scanning; ISR can re-set it if something new comes in during the loop
    for (uint8_t i = 0; i < numTasks; i++) {
      if (tasks[i].runNow) {
        tasks[i].runNow = false;  // Set before running, in case ISR retriggers during task
        tasks[i].task();
      }
    }
  }
}
