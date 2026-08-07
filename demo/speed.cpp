//! TCST2103 based speed sensing using interrups
#include <Arduino.h>

// speed_test.ino
// Standalone test rig for wheel-speed sensing logic.
// Mirrors SPEED::init/configure/update from speed.h/.cpp so you can
// validate the math and sensor wiring in isolation on an Uno.

const uint8_t WHEEL_SPEED_SENSOR_PIN = 18;  // Must be an interrupt-capable pin

// --- Sensor config (must match your real wheel/sensor setup) ---
const uint16_t WHEEL_CIRCUMFERENCE_MM = 1596;  // 20" wheel
const uint8_t PULSES_PER_ROTATION = 4;

// --- Derived constant ---
// speed (km/h) = coefficient * pulses / sampleTime(us)
// coefficient = circumference(mm) / (pulsesPerRotation * 3600)
float coefficient;

volatile uint16_t pulseCount = 0;  // Incremented in ISR, must be volatile
unsigned long previousMicros = 0;

void sensor_isr() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("TCST2103 test"));

  coefficient = (float)WHEEL_CIRCUMFERENCE_MM / ((float)PULSES_PER_ROTATION * 3600.0f);

  Serial.print("Coefficient: ");
  Serial.println(coefficient, 8);

  previousMicros = micros();
  attachInterrupt(digitalPinToInterrupt(WHEEL_SPEED_SENSOR_PIN), sensor_isr, FALLING);

  Serial.println("Speed test started. Waiting for pulses...");
}

void loop() {
  // Sample once per second for readable logging. (Real firmware may call update() much more often; interval doesn't affect correctness since we measure actual elapsed time each call.)
  delay(1000);

  noInterrupts();  // pulseCount is >1 byte, not atomic on AVR — must guard
  uint16_t pulsesRecorded = pulseCount;
  pulseCount = 0;
  interrupts();

  unsigned long now = micros();
  unsigned long microsElapsed = now - previousMicros;  // unsigned subtraction handles micros() rollover correctly
  previousMicros = now;

  float kmh = 0.0f;
  if (microsElapsed > 0) {  // guard divide-by-zero, shouldn't happen with 1s delay but cheap to check
    kmh = coefficient * pulsesRecorded / (float)microsElapsed;
  }

  Serial.print("Pulses: ");
  Serial.print(pulsesRecorded);
  Serial.print("  Elapsed(us): ");
  Serial.print(microsElapsed);
  Serial.print("  Speed(km/h): ");
  Serial.println(kmh, 3);
}
