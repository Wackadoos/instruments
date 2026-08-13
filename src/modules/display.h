#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>

#include "utils/metrics.h"

class Page;

class Display {
 public:
  static void init(SPIClass* spi);
  static void run();
  static inline bool isEnabled() { return enabled; };

  static Arduino_ILI9488_18bit screen;
  static XPT2046_Touchscreen touch;

 private:
  static IntervalMetric dataProcessTime;
  inline static bool enabled = false;

  static Page* currentPage;

  static Arduino_HWSPI bus;
};

enum class TextAlign : uint8_t { LEFT,
                                 CENTER,
                                 RIGHT };

class WidgetBase {
 public:
  virtual void draw() = 0;
  virtual void clear() = 0;
  virtual void pressed(TS_Point point) = 0;
  static void writeText(const char* text, int16_t x, int16_t y, TextAlign align);
  static void drawTrailing(const char* mainText, const char* trailingText, int16_t x, int16_t y, TextAlign align,
                           uint8_t trailingTextSize, uint8_t trailingGap, uint8_t trailingVerticalPad);
  virtual ~WidgetBase() = default;
};

template <typename T>
struct WidgetPrinter {
  static void format(T value, char* buf, size_t len, uint8_t decimalDigits = 0) {
    // works for int, long, etc. via Arduino's String; adjust if you need bases other than base-10
    String(value).toCharArray(buf, len);
  }

  static void print(T value, int16_t x, int16_t y, TextAlign align = TextAlign::LEFT, uint8_t decimalDigits = 0,
                    const char* trailingText = nullptr, uint8_t trailingTextSize = 0, uint8_t trailingGap = 2, uint8_t trailingVerticalPad = 0) {
    char buf[32];
    format(value, buf, sizeof(buf), decimalDigits);
    WidgetBase::writeText(buf, x, y, align);
    WidgetBase::drawTrailing(buf, trailingText, x, y, align, trailingTextSize, trailingGap, trailingVerticalPad);
  }
};

template <>
struct WidgetPrinter<float> {
  static void format(float value, char* buf, size_t len, uint8_t decimalDigits = 0) {
    dtostrf(value, 0, decimalDigits, buf);  // width 0 = no padding, just decimalDigits precision
  }

  static void print(float value, int16_t x, int16_t y, TextAlign align = TextAlign::LEFT, uint8_t decimalDigits = 0,
                    const char* trailingText = nullptr, uint8_t trailingTextSize = 0, uint8_t trailingGap = 2, uint8_t trailingVerticalPad = 0) {
    char buf[32];
    format(value, buf, sizeof(buf), decimalDigits);
    WidgetBase::writeText(buf, x, y, align);
    WidgetBase::drawTrailing(buf, trailingText, x, y, align, trailingTextSize, trailingGap, trailingVerticalPad);
  }
};

template <>
struct WidgetPrinter<const char*> {
  static void format(const char* value, char* buf, size_t len, uint8_t decimalDigits = 0) {
    strncpy(buf, value, len - 1);
    buf[len - 1] = '\0';
  }

  static void print(const char* value, int16_t x, int16_t y, TextAlign align = TextAlign::LEFT, uint8_t decimalDigits = 0,
                    const char* trailingText = nullptr, uint8_t trailingTextSize = 0, uint8_t trailingGap = 2, uint8_t trailingVerticalPad = 0) {
    WidgetBase::writeText(value, x, y, align);
    WidgetBase::drawTrailing(value, trailingText, x, y, align, trailingTextSize, trailingGap, trailingVerticalPad);
  }
};

template <typename T>
class StaticWidget : public WidgetBase {
 public:
  T* data;
  int16_t cursorX;
  int16_t cursorY;
  TextAlign align;
  uint16_t textColor;
  uint8_t textSize;
  uint8_t decimalDigits;  // only meaningful for float, harmless otherwise

  StaticWidget(T* data, int16_t cursorX, int16_t cursorY, TextAlign align, uint16_t textColor, uint8_t textSize, uint8_t decimalDigits = 0)
      : data(data), cursorX(cursorX), cursorY(cursorY), align(align), textColor(textColor), textSize(textSize), decimalDigits(decimalDigits) {}

  void draw() override {
    if (firstDraw) {
      Display::screen.setCursor(cursorX, cursorY);
      Display::screen.setTextColor(textColor, RGB565_BLACK);
      Display::screen.setTextSize(textSize);
      WidgetPrinter<T>::print(*data, cursorX, cursorY, align, decimalDigits);
      firstDraw = false;
    }
  }

  void clear() override {
    Display::screen.setCursor(cursorX, cursorY);
    Display::screen.setTextColor(RGB565_BLACK);
    Display::screen.setTextSize(textSize);
    WidgetPrinter<T>::print(*data, cursorX, cursorY, align, decimalDigits);
    firstDraw = true;
  }

  void pressed(TS_Point point) override {};
  // TODO debounce presses to avoid multiple triggers within 1s window

 private:
  bool firstDraw = true;
};

template <typename T>
class Button : public WidgetBase {
 public:
  T* data;
  int16_t cursorX;
  int16_t cursorY;
  uint16_t rectWidth;
  uint16_t rectHeight;
  int16_t radius;
  uint16_t textColor;
  uint8_t textSize;
  uint8_t decimalDigits;  // only meaningful for float, harmless otherwise

  Button(T* data, int16_t x, int16_t y, uint16_t rectWidth, uint16_t rectHeight, int16_t radius, uint16_t textColor, uint8_t textSize, uint8_t decimalDigits = 0)
      : data(data),
        cursorX(x),
        cursorY(y),
        rectWidth(rectWidth),
        rectHeight(rectHeight),
        radius(radius),
        textColor(textColor),
        textSize(textSize),
        decimalDigits(decimalDigits) {}

  void draw() override {
    if (firstDraw) {
      char buf[32];
      WidgetPrinter<T>::format(*data, buf, sizeof(buf), decimalDigits);

      int16_t x1, y1;
      uint16_t w, h;
      Display::screen.setTextSize(textSize);
      Display::screen.getTextBounds(buf, cursorX, cursorY, &x1, &y1, &w, &h);
      int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
      int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
      Display::screen.setCursor(cursorX, cursorY);
      Display::screen.setTextColor(textColor, RGB565_BLACK);
      WidgetPrinter<T>::print(*data, cursorX, cursorY, TextAlign::CENTER, decimalDigits);
      Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, textColor);
      firstDraw = false;
    }
  }

  void clear() override {
    char buf[32];
    WidgetPrinter<T>::format(*data, buf, sizeof(buf), decimalDigits);

    int16_t x1, y1;
    uint16_t w, h;
    Display::screen.setTextSize(textSize);
    Display::screen.getTextBounds(buf, cursorX, cursorY, &x1, &y1, &w, &h);
    int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
    int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
    Display::screen.setCursor(cursorX, cursorY);
    Display::screen.setTextColor(RGB565_BLACK);
    WidgetPrinter<T>::print(*data, cursorX, cursorY, TextAlign::CENTER, decimalDigits);
    Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, RGB565_BLACK);
    firstDraw = true;
  }

  void pressed(TS_Point point) override {
    // TODO if point is within rect, run callback
  };

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
  TextAlign align;
  uint16_t textColor;
  uint8_t textSize;
  uint8_t decimalDigits;  // only meaningful for float, harmless otherwise
  const char* trailingText;
  uint8_t trailingTextSize;
  uint8_t trailingGap;
  uint8_t trailingVerticalPad;

  Widget(T* data, int16_t cursorX, int16_t cursorY, TextAlign align, uint16_t textColor, uint8_t textSize,
         uint8_t decimalDigits = 0, const char* trailingText = nullptr, uint8_t trailingTextSize = 0, uint8_t trailingGap = 2,
         uint8_t trailingVerticalPad = 0)
      : data(data),
        cursorX(cursorX),
        cursorY(cursorY),
        align(align),
        textColor(textColor),
        textSize(textSize),
        decimalDigits(decimalDigits),
        trailingText(trailingText),
        trailingTextSize(trailingTextSize),
        trailingGap(trailingGap),
        trailingVerticalPad(trailingVerticalPad) {}

  void draw() override {
    if (prevData != *data || firstDraw) {
      Display::screen.setCursor(cursorX, cursorY);
      Display::screen.setTextColor(textColor, RGB565_BLACK);
      Display::screen.setTextSize(textSize);
      prevData = *data;
      WidgetPrinter<T>::print(prevData, cursorX, cursorY, align, decimalDigits, trailingText, trailingTextSize, trailingGap, trailingVerticalPad);
      firstDraw = false;
    }
  }

  void clear() override {
    Display::screen.setCursor(cursorX, cursorY);
    Display::screen.setTextColor(RGB565_BLACK);
    Display::screen.setTextSize(textSize);
    WidgetPrinter<T>::print(prevData, cursorX, cursorY, align, decimalDigits, trailingText, trailingTextSize, trailingGap, trailingVerticalPad);
    prevData = T();
    firstDraw = true;
  }

  void pressed(TS_Point point) override {};

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
  void pressed(TS_Point point);

 private:
  WidgetBase* const* widgets;
  size_t count;
};

// TODO when text width drops, ensure overwrite old space with black
// TODO display errors on screen?
// TODO Display average watts while driving, maybe 1min rolling average or something? Or exponential decay?
// TODO  Display class should use widgets with local state of last update (to update only if different - after decimal precision cutoff etc) and all widgets implement an erase function that redraws value in black to erase from screen using just those pixels. Will this cause problems with antialiasing on text?
// TODO Widgets get enabled/disabled per page. Page class calls initial draw and erase on exit. Widgets keep updating during when modified
// TODO Render one widget at a time and return. Avoid long contiguous render blocks as this will block sensor data acquisition. Have a selector that iterates through widgets yielding after one renders, to avoid prioritization and starving later widgets from rendering even under heavy contention.
