#include "speed.h"

#include "hardware.h"

IntervalMetric SPEED::dataProcessTime = IntervalMetric();

void SPEED::init(SensorState* state) {
  sensorState = state;

  previousMicros = micros();
  attachInterrupt(digitalPinToInterrupt(WHEEL_SPEED_SENSOR_PIN), sensor_isr, FALLING);  // Sense on falling, pulls to ground through phototransistor

  dataProcessTime.init(F("Speed Proc"), F("Time to process Speed Data"));
  enabled = true;
}

void SPEED::configure(uint16_t wheelCircumferenceMillimeters, uint8_t pulsesPerRotation) {
  // Pre-calculated coefficient such that speed (km/h) = coefficient * pulses / pulsePeriodMicroseconds
  coefficient = (float)wheelCircumferenceMillimeters / ((float)pulsesPerRotation * 3600.0f);  // Use multiply rather than second division, it's faster!

  //* The math to resolve this:
  // Speed (km/h) = circumference (km) * pulsesOccurred / pulsesPerRevolution / sampleTime (hr)
  // Adjusting units (we want to measure wheel in millimeters and sample time in microseconds): (km/h) = circumference (mm) * pulsesOccurred / pulsesPerRevolution / sampleTime (us) * 1000000 / 3600000000
  // Rearrange & simplify: (km/h) = circumference (mm) / pulsesPerRevolution / 3600 * pulsesOccurred / sampleTime (us)
  // First part is constant: circumference (mm) / pulsesPerRevolution / 3600
}

void SPEED::update() {
  if (enabled) {
    dataProcessTime.start();
    noInterrupts();  // Must disable interrupts to copy - uint16_t isn't atomic!
    auto pulsesRecorded = pulseCount;
    pulseCount = 0;
    interrupts();

    auto time = micros();
    auto microsElapsed = time - previousMicros;  // Note! This correctly handles rollover (as long as less than ~71 mins)
    previousMicros = time;

    sensorState->kilometers_per_hour = coefficient * pulsesRecorded / microsElapsed;
    dataProcessTime.stop();
  }
}

void SPEED::sensor_isr() {
  pulseCount++;
}
