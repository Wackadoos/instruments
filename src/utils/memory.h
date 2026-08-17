#pragma once

#include <Arduino.h>

#include "state.h"

extern char* __brkval;
extern char __heap_start;

inline uint16_t freeMemory() {
  char top;
  return __brkval ? &top - __brkval : &top - &__heap_start;
}

inline void measureRam() {
  State::ram_free_bytes = freeMemory();
  if (State::ram_free_bytes < State::ram_free_bytes_minimum) {
    State::ram_free_bytes_minimum = State::ram_free_bytes;
  }  // Put this in an isr triggered at a random time to everything else for more accuracy

#ifdef DEBUG_LOGGING
  Serial.print(F("Free RAM = "));         // F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(State::ram_free_bytes);  // print how much RAM is available in bytes.
#endif
}
