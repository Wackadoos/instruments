#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "utils/metrics.h"

class Page;

class Display {
 public:
  static void init(SPIClass* spi);
  static void run();
  static inline bool isEnabled() { return enabled; };

  static Arduino_ILI9488_18bit screen;

 private:
  static IntervalMetric dataProcessTime;
  inline static bool enabled = false;

  static Page* currentPage;

  static Arduino_HWSPI bus;
};

class WidgetBase {
 public:
  virtual void draw() = 0;
  virtual void clear() = 0;
  virtual ~WidgetBase() = default;
};

template <typename T>
struct WidgetPrinter {
  static void print(T value, uint8_t decimalDigits) {
    Display::screen.println(value);
  }
};

template <>
struct WidgetPrinter<float> {
  static void print(float value, uint8_t decimalDigits) {
    Display::screen.println(value, decimalDigits);
  }
};

template <typename T>
class StaticWidget : public WidgetBase {
 public:
  T* data;
  int16_t cursorX;
  int16_t cursorY;
  uint16_t textColor;
  uint8_t textSize;
  uint8_t decimalDigits;  // only meaningful for float, harmless otherwise

  StaticWidget(T* data, int16_t cursorX, int16_t cursorY, uint16_t textColor, uint8_t textSize, uint8_t decimalDigits = 0)
      : data(data), cursorX(cursorX), cursorY(cursorY), textColor(textColor), textSize(textSize), decimalDigits(decimalDigits) {}

  void draw() override {
    if (firstDraw) {
      Display::screen.setCursor(cursorX, cursorY);
      Display::screen.setTextColor(textColor, RGB565_BLACK);
      Display::screen.setTextSize(textSize);
      WidgetPrinter<T>::print(*data, decimalDigits);
      firstDraw = false;
    }
  }

  void clear() override {
    Display::screen.setCursor(cursorX, cursorY);
    Display::screen.setTextColor(RGB565_BLACK);
    Display::screen.setTextSize(textSize);
    WidgetPrinter<T>::print(*data, decimalDigits);
    firstDraw = true;
  }

 private:
  bool firstDraw = true;
};

template <typename T>
class Widget : public WidgetBase {
 public:
  T* data;
  T prevData;
  int16_t cursorX;
  int16_t cursorY;
  uint16_t textColor;
  uint8_t textSize;
  uint8_t decimalDigits;  // only meaningful for float, harmless otherwise

  Widget(T* data, int16_t cursorX, int16_t cursorY, uint16_t textColor, uint8_t textSize, uint8_t decimalDigits = 0)
      : data(data), cursorX(cursorX), cursorY(cursorY), textColor(textColor), textSize(textSize), decimalDigits(decimalDigits) {}

  void draw() override {
    if (prevData != *data || firstDraw) {
      Display::screen.setCursor(cursorX, cursorY);
      Display::screen.setTextColor(textColor, RGB565_BLACK);
      Display::screen.setTextSize(textSize);
      prevData = *data;
      WidgetPrinter<T>::print(prevData, decimalDigits);
      firstDraw = false;
    }
  }

  void clear() override {
    Display::screen.setCursor(cursorX, cursorY);
    Display::screen.setTextColor(RGB565_BLACK);
    Display::screen.setTextSize(textSize);
    WidgetPrinter<T>::print(prevData, decimalDigits);
    prevData = T();
    firstDraw = true;
  }

 private:
  bool firstDraw = true;
};

class Page {
 public:
  template <size_t N>
  Page(const WidgetBase* (&widgets)[N]) : widgets(widgets), count(N) {}

  size_t size() const { return count; }
  const WidgetBase* operator[](size_t i) const { return widgets[i]; }

  void refresh();
  void clear();

 private:
  WidgetBase* const* widgets;
  size_t count;
};

// TODO display errors on screen?
// TODO Display average watts while driving, maybe 1min rolling average or something? Or exponential decay?
// TODO Frame buffers?
// TODO  Display class should use widgets with local state of last update (to update only if different - after decimal precision cutoff etc) and all widgets implement an erase function that redraws value in black to erase from screen using just those pixels. Will this cause problems with antialiasing on text?
// TODO Widgets get enabled/disabled per page. Page class calls initial draw and erase on exit. Widgets keep updating during when modified
// TODO Render one widget at a time and return. Avoid long contiguous render blocks as this will block sensor data acquisition. Have a selector that iterates through widgets yielding after one renders, to avoid prioritization and starving later widgets from rendering even under heavy contention.
