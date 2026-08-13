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

struct TrailingText {
  const char* text = nullptr;
  uint8_t size = 0;
  uint8_t gap = 2;
  uint8_t verticalPad = 0;
};

struct TextConfig {
  int16_t x = 0;
  int16_t y = 0;
  TextAlign align = TextAlign::LEFT;
  int16_t color = RGB565_WHITE;  // RGB565 macros are signed int on AVR; bits are preserved when converted to uint16_t at draw time
  uint8_t size = 1;
  uint8_t decimalDigits = 0;
  TrailingText trailing;
};

class WidgetBase {
 public:
  virtual void draw() = 0;
  virtual void clear() = 0;
  virtual void pressed(TS_Point point) = 0;
  virtual ~WidgetBase() = default;
  constexpr WidgetBase() = default;

  static void textAnchor(const char* text, int16_t x, int16_t y, TextAlign align, int16_t& outX, int16_t& outY,
                         uint16_t& w, uint16_t& h);
  static void drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color);
  static void drawTrailing(const char* mainText, int16_t x, int16_t y, TextAlign align, uint8_t mainTextSize,
                           const TrailingText& trailing);
};

struct WidgetPrinter {
  static void formatValue(float value, char* buf, size_t len, uint8_t decimalDigits) {
    dtostrf(value, 0, decimalDigits, buf);  // width 0 = no padding, just decimalDigits precision
  }

  static void formatValue(double value, char* buf, size_t len, uint8_t decimalDigits) {
    dtostrf(value, 0, decimalDigits, buf);
  }

  static void formatValue(const char* value, char* buf, size_t len, uint8_t) {
    strncpy(buf, value, len - 1);
    buf[len - 1] = '\0';
  }

  static void formatValue(char* value, char* buf, size_t len, uint8_t decimalDigits) {
    formatValue(static_cast<const char*>(value), buf, len, decimalDigits);
  }

  template <typename T>
  static void formatValue(T value, char* buf, size_t len, uint8_t) {
    String(value).toCharArray(buf, len);
  }

  template <typename T>
  static void format(T value, char* buf, size_t len, uint8_t decimalDigits = 0) {
    formatValue(value, buf, len, decimalDigits);
  }

  template <typename T>
  static void print(T value, const TextConfig& cfg, uint16_t color) {
    char buf[32];
    format(value, buf, sizeof(buf), cfg.decimalDigits);
    WidgetBase::drawText(buf, cfg.x, cfg.y, cfg.align, cfg.size, color);
    WidgetBase::drawTrailing(buf, cfg.x, cfg.y, cfg.align, cfg.size, cfg.trailing);
  }
};

template <typename T>
class TextWidget : public WidgetBase {
 public:
  T* data;
  TextConfig cfg;

  constexpr TextWidget(T* data, const TextConfig& cfg) : data(data), cfg(cfg) {}

  void render(T value, uint16_t color) { WidgetPrinter::print(value, cfg, color); }

  void pressed(TS_Point point) override {};
};

template <typename T>
class StaticWidget : public TextWidget<T> {
 public:
  constexpr StaticWidget(T* data, const TextConfig& cfg) : TextWidget<T>(data, cfg) {}

  void draw() override {
    if (firstDraw) {
      this->render(*this->data, this->cfg.color);
      firstDraw = false;
    }
  }

  void clear() override {
    this->render(*this->data, RGB565_BLACK);
    firstDraw = true;
  }

 private:
  bool firstDraw = true;
};

template <typename T>
class Widget : public TextWidget<T> {
 public:
  constexpr Widget(T* data, const TextConfig& cfg) : TextWidget<T>(data, cfg) {}

  void draw() override {
    if (prevData != *this->data || firstDraw) {
      prevData = *this->data;
      this->render(prevData, this->cfg.color);
      firstDraw = false;
    }
  }

  void clear() override {
    this->render(prevData, RGB565_BLACK);
    prevData = T();
    firstDraw = true;
  }

 private:
  T prevData = T();
  bool firstDraw = true;
};

template <typename T>
class Button : public TextWidget<T> {
 public:
  constexpr Button(T* data, const TextConfig& cfg, uint16_t rectWidth, uint16_t rectHeight, int16_t radius)
      : TextWidget<T>(data, cfg), rectWidth(rectWidth), rectHeight(rectHeight), radius(radius) {
    this->cfg.align = TextAlign::CENTER;
  }

  void draw() override {
    if (firstDraw) {
      char buf[32];
      WidgetPrinter::format(*this->data, buf, sizeof(buf), this->cfg.decimalDigits);

      int16_t x1, y1;
      uint16_t w, h;
      Display::screen.setTextSize(this->cfg.size);
      Display::screen.getTextBounds(buf, this->cfg.x, this->cfg.y, &x1, &y1, &w, &h);
      int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
      int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
      this->render(*this->data, this->cfg.color);
      Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, this->cfg.color);
      firstDraw = false;
    }
  }

  void clear() override {
    char buf[32];
    WidgetPrinter::format(*this->data, buf, sizeof(buf), this->cfg.decimalDigits);

    int16_t x1, y1;
    uint16_t w, h;
    Display::screen.setTextSize(this->cfg.size);
    Display::screen.getTextBounds(buf, this->cfg.x, this->cfg.y, &x1, &y1, &w, &h);
    int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
    int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
    this->render(*this->data, RGB565_BLACK);
    Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, RGB565_BLACK);
    firstDraw = true;
  }

 private:
  uint16_t rectWidth;
  uint16_t rectHeight;
  int16_t radius;
  bool firstDraw = true;
};

class Page {
 public:
  template <size_t N>
  Page(WidgetBase* const (&widgets)[N]) : widgets(widgets), count(N) {}

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
