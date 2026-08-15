#pragma once

#include <Arduino.h>

#define DEBUG_ITEMS_PER_PAGE 23  // Content lines per page (line 0 is the header)

struct DebugLine {
  const __FlashStringHelper* label = nullptr;  // Flash label for label+value lines (drawn once per page)
  bool wholeLine = false;                      // true: valueBuf already holds the full line text (errors)
};

uint8_t debugPageCount();
// Renders the given page/line into valueBuf + out. Returns false when the slot is empty.
bool debugItem(uint8_t page, uint8_t line, DebugLine& out, char* valueBuf, size_t len);
