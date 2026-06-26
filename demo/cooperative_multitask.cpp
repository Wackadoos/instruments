// TODO task prioritization? Scheduling?

// ============================================================
//  Cooperative Multitasking Demo for Arduino
//
//  Three independent "tasks" run concurrently without using
//  delay(), threads, or an RTOS.  Each task tracks its own
//  "next run" timestamp and only acts when its turn comes.
//
//  Tasks:
//    1. Blink LED (pin 13) every 500 ms
//    2. Pulse LED (pin 11, PWM) in a breathing pattern
//    3. Print a status report to Serial every 2 000 ms
// ============================================================
#include <Arduino.h>

// ── Pin assignments ─────────────────────────────────────────
const uint8_t PIN_BLINK  = 13;   // built-in LED on most boards
const uint8_t PIN_BREATH = 11;   // must be a PWM-capable pin

// ── Task state ──────────────────────────────────────────────
// Each task stores only the timestamp of its NEXT scheduled run.

unsigned long blinkNextMs   = 0;
unsigned long breathNextMs  = 0;
unsigned long reportNextMs  = 0;

bool blinkState = false;         // current LED on/off state

// Breathing: iterate brightness 0→255→0 in small steps
uint8_t  breathBrightness = 0;
int8_t   breathDirection  = 1;   // +1 rising, -1 falling
uint16_t breathCycleCount = 0;

unsigned long taskRunCount[3] = {0, 0, 0};  // for the status report


// ── Setup ────────────────────────────────────────────────────
void setup() {
  pinMode(PIN_BLINK,  OUTPUT);
  pinMode(PIN_BREATH, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {}           // wait for USB serial on Leonardo/Micro

  Serial.println(F("=== Cooperative Multitasking Demo ==="));
  Serial.println(F("Tasks:"));
  Serial.println(F("  [1] Blink  LED  on pin 13  every   500 ms"));
  Serial.println(F("  [2] Breath LED  on pin 11  (PWM step every 10 ms)"));
  Serial.println(F("  [3] Serial report           every 2 000 ms"));
  Serial.println();
}


// ── Main loop ────────────────────────────────────────────────
//
//  The loop() function runs as fast as possible.
//  Each task checks millis() and only executes when its
//  interval has elapsed – no blocking, no delay().
//
void loop() {
  unsigned long now = millis();

  // ── Task 1 : Blink ────────────────────────────────────────
  if (now >= blinkNextMs) {
    blinkState = !blinkState;
    digitalWrite(PIN_BLINK, blinkState ? HIGH : LOW);

    blinkNextMs = now + 500UL;   // reschedule 500 ms from NOW
    taskRunCount[0]++;
  }

  // ── Task 2 : Breathe ──────────────────────────────────────
  if (now >= breathNextMs) {
    analogWrite(PIN_BREATH, breathBrightness);

    breathBrightness += breathDirection * 5;

    // Reverse direction at the ends of the range
    if (breathBrightness >= 250) {
      breathBrightness = 255;
      breathDirection  = -1;
      breathCycleCount++;
    } else if (breathBrightness <= 5) {
      breathBrightness = 0;
      breathDirection  = 1;
    }

    breathNextMs = now + 10UL;   // reschedule 10 ms from NOW
    taskRunCount[1]++;
  }

  // ── Task 3 : Serial status report ─────────────────────────
  if (now >= reportNextMs) {
    Serial.print(F("[t="));
    Serial.print(now);
    Serial.print(F("ms]  blink_runs="));
    Serial.print(taskRunCount[0]);
    Serial.print(F("  breath_runs="));
    Serial.print(taskRunCount[1]);
    Serial.print(F("  breath_cycles="));
    Serial.print(breathCycleCount);
    Serial.print(F("  report_runs="));
    Serial.println(taskRunCount[2] + 1);

    reportNextMs = now + 2000UL;  // reschedule 2 000 ms from NOW
    taskRunCount[2]++;
  }

  // ── Idle ──────────────────────────────────────────────────
  // Nothing else to do this iteration – return immediately so
  // the scheduler can check again on the next loop pass.
  // In a real system you might call a power-saving sleep here.
}
