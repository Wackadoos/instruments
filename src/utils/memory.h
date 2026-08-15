#pragma once

#include <Arduino.h>

extern char* __brkval;
extern char __heap_start;

inline uint16_t freeMemory() {
  char top;
  return __brkval ? &top - __brkval : &top - &__heap_start;
}
