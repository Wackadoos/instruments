#pragma once

#include <Arduino.h>
#include <LinkedList.h>
#include <limits.h>

class IntervalMetric {
 private:
  uint32_t startTime = 0;
  uint32_t minTime = ULONG_MAX;            // Minimum recorded time in microseconds. Starts at maximum
  uint32_t maxTime = 0;                    // Maximum recorded time in microseconds. Starts at minimum
  uint32_t totalTime = 0;                  // Total time in microseconds / 8 (~9hr rollover, sufficient for use case)
  uint32_t count = 0;                      // Number of measurements taken
  const __FlashStringHelper* name_;        // Name of the interval
  const __FlashStringHelper* description_; // Description of the interval

 public:
  IntervalMetric() = default;

  void init(const __FlashStringHelper* name, const __FlashStringHelper* description);
  void start();
  void stop();
  uint32_t average() const;
  inline uint32_t shortest() const { return minTime; };
  inline uint32_t longest() const { return maxTime; };
  inline uint32_t total() const { return totalTime; };
  inline uint32_t sampleCount() const { return count; };
  inline const __FlashStringHelper* name() const { return name_; };
};

class METRICS {
 public:
  static void addMetric(IntervalMetric* metric);
  static uint8_t count();
  static IntervalMetric* get(uint8_t index);

 private:
  static LinkedList<IntervalMetric*> metrics;
};
