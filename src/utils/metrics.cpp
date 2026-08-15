#include "metrics.h"

LinkedList<IntervalMetric*> METRICS::metrics = LinkedList<IntervalMetric*>();

void IntervalMetric::init(const __FlashStringHelper* name, const __FlashStringHelper* description) {
  name_ = name;
  description_ = description;
  METRICS::addMetric(this);
};

void IntervalMetric::start() {
  startTime = micros();
}

void IntervalMetric::stop() {
  auto time = micros() - startTime;

  if (time < minTime) minTime = time;
  if (time > maxTime) maxTime = time;

  totalTime += (time + 4) >> 3;  // Round & shift 3 binary places. (Add 4, then divide by 8). Reduces precision to avoid rollover
  count++;
}

uint32_t IntervalMetric::average() const {
  if (!count) return 0;
  return ((totalTime + (count >> 1)) / count) << 3;  // Rounded integer division, scaled up by 8x to revert to microseconds
}

void METRICS::addMetric(IntervalMetric* metric) {
  metrics.add(metric);
}

uint8_t METRICS::count() {
  return (uint8_t)metrics.size();
}

IntervalMetric* METRICS::get(uint8_t index) {
  if (index >= metrics.size()) {
    return nullptr;
  }
  return metrics.get(index);
}
