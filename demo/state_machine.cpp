/*
 * State Machine Demo - Traffic Light Controller
 *
 * Simulates a simple traffic light using a state machine.
 * Three LEDs (or just Serial output) cycle through:
 *   RED -> GREEN -> YELLOW -> RED -> ...
 *
 * Hardware (optional):
 *   - Red LED    on pin 9
 *   - Yellow LED on pin 10
 *   - Green LED  on pin 11
 *   - Button     on pin 2 (INPUT_PULLUP) to trigger EMERGENCY state
 */

#include <Arduino.h>

// ── Pin definitions ──────────────────────────────────────────────
const int PIN_RED = 9;
const int PIN_YELLOW = 10;
const int PIN_GREEN = 11;
const int PIN_BTN = 2;

// ── State definitions ────────────────────────────────────────────
enum TrafficState
{
    STATE_RED,
    STATE_GREEN,
    STATE_YELLOW,
    STATE_EMERGENCY // all lights flash red
};

// ── Timing constants (ms) ────────────────────────────────────────
const unsigned long RED_DURATION = 4000;
const unsigned long GREEN_DURATION = 3000;
const unsigned long YELLOW_DURATION = 1500;
const unsigned long EMERGENCY_DURATION = 500; // flash period

// ── Global state ─────────────────────────────────────────────────
TrafficState currentState = STATE_RED;
unsigned long stateStartTime = 0;
bool emergencyFlashOn = false;

// ── Helpers ──────────────────────────────────────────────────────
void setLights(bool red, bool yellow, bool green)
{
    digitalWrite(PIN_RED, red ? HIGH : LOW);
    digitalWrite(PIN_YELLOW, yellow ? HIGH : LOW);
    digitalWrite(PIN_GREEN, green ? HIGH : LOW);
}

void enterState(TrafficState next)
{
    currentState = next;
    stateStartTime = millis();
    emergencyFlashOn = false;

    switch (next)
    {
    case STATE_RED:
        setLights(true, false, false);
        Serial.println("→ STATE: RED");
        break;
    case STATE_GREEN:
        setLights(false, false, true);
        Serial.println("→ STATE: GREEN");
        break;
    case STATE_YELLOW:
        setLights(false, true, false);
        Serial.println("→ STATE: YELLOW");
        break;
    case STATE_EMERGENCY:
        Serial.println("→ STATE: EMERGENCY (press button again to clear)");
        break;
    }
}

// ── Setup ────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_YELLOW, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP); // LOW when pressed

    Serial.println("Traffic Light State Machine Demo");
    Serial.println("=================================");
    Serial.println("Button on pin 2 triggers EMERGENCY mode.");
    Serial.println();
    enterState(STATE_RED);
}

// ── Main loop ────────────────────────────────────────────────────
void loop()
{
    unsigned long now = millis();
    unsigned long elapsed = now - stateStartTime;
    bool buttonPressed = (digitalRead(PIN_BTN) == LOW);

    switch (currentState)
    {

    // ── RED: wait, then go GREEN ──────────────────────────────
    case STATE_RED:
        if (buttonPressed)
        {
            enterState(STATE_EMERGENCY);
        }
        else if (elapsed >= RED_DURATION)
        {
            enterState(STATE_GREEN);
        }
        break;

    // ── GREEN: wait, then go YELLOW ───────────────────────────
    case STATE_GREEN:
        if (buttonPressed)
        {
            enterState(STATE_EMERGENCY);
        }
        else if (elapsed >= GREEN_DURATION)
        {
            enterState(STATE_YELLOW);
        }
        break;

    // ── YELLOW: wait, then go RED ─────────────────────────────
    case STATE_YELLOW:
        if (buttonPressed)
        {
            enterState(STATE_EMERGENCY);
        }
        else if (elapsed >= YELLOW_DURATION)
        {
            enterState(STATE_RED);
        }
        break;

    // ── EMERGENCY: flash red; button press exits ──────────────
    case STATE_EMERGENCY:
        // Flash red LED on/off
        if (elapsed >= EMERGENCY_DURATION)
        {
            emergencyFlashOn = !emergencyFlashOn;
            setLights(emergencyFlashOn, false, false);
            stateStartTime = now; // reset timer for next flash
        }
        // Button press exits emergency back to RED
        if (buttonPressed)
        {
            Serial.println("Emergency cleared.");
            delay(300); // simple debounce
            enterState(STATE_RED);
        }
        break;
    }
}
