#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>

#include "utils/metrics.h"

#define TEXT_BUFFER_SIZE 16

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

struct TextRegion {
  int16_t x = 0;
  int16_t y = 0;
  uint16_t w = 0;
  uint16_t h = 0;
};

class WidgetBase {
 public:
  virtual void draw() = 0;
  virtual void clear() = 0;
  virtual void pressed(TS_Point point) = 0;
  virtual ~WidgetBase() = default;
  constexpr WidgetBase() = default;

  static void textBlockBounds(const char* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                              int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h);
  static void drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color,
                       uint8_t linePad = 0);
  static void drawTrailing(const char* mainText, int16_t x, int16_t y, TextAlign align, uint8_t mainTextSize,
                           const TrailingText& trailing);
  static void textRegion(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint8_t linePad,
                         TextRegion& out);
  static void trailingRegion(const TrailingText& trailing, const TextRegion& main, TextRegion& out);
  static void eraseStale(const TextRegion& oldR, const TextRegion& newR);
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
    char buf[TEXT_BUFFER_SIZE];
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

class StaticWidget : public TextWidget<const char*> {
 public:
  constexpr StaticWidget(const char* text, const TextConfig& cfg)
      : TextWidget<const char*>(&storedText, cfg), storedText(text) {}

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
  const char* storedText;
  bool firstDraw = true;
};

template <typename T>
class Widget : public TextWidget<T> {
 public:
  constexpr Widget(T* data, const TextConfig& cfg) : TextWidget<T>(data, cfg) {}

  void draw() override {
    char buf[TEXT_BUFFER_SIZE];
    WidgetPrinter::format(*this->data, buf, sizeof(buf), this->cfg.decimalDigits);

    if (firstDraw || strcmp(buf, prevString) != 0) {
      TextRegion main;
      WidgetBase::textRegion(buf, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, 0, main);

      bool sizeChanged = firstDraw || main.w != prevW || main.h != prevH;
      bool hasTrailing = this->cfg.trailing.text != nullptr && this->cfg.trailing.size != 0;

      if (!firstDraw) {
        TextRegion oldR;
        oldR.x = this->cfg.x;
        oldR.y = this->cfg.y;
        oldR.w = prevW;
        oldR.h = prevH;
        if (this->cfg.align == TextAlign::CENTER) {
          oldR.x -= static_cast<int16_t>(prevW / 2);
        } else if (this->cfg.align == TextAlign::RIGHT) {
          oldR.x -= static_cast<int16_t>(prevW);
        }
        WidgetBase::eraseStale(oldR, main);

        if (hasTrailing && sizeChanged) {
          TextRegion oldTrailing, newTrailing;
          WidgetBase::trailingRegion(this->cfg.trailing, oldR, oldTrailing);
          WidgetBase::trailingRegion(this->cfg.trailing, main, newTrailing);
          WidgetBase::eraseStale(oldTrailing, newTrailing);
        }
      }

      prevW = main.w;
      prevH = main.h;
      strncpy(prevString, buf, sizeof(prevString) - 1);
      prevString[sizeof(prevString) - 1] = '\0';

      WidgetBase::drawText(buf, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, this->cfg.color);
      if (hasTrailing && sizeChanged) {
        WidgetBase::drawTrailing(buf, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, this->cfg.trailing);
      }
      firstDraw = false;
    }
  }

  void clear() override {
    WidgetBase::drawText(prevString, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK);
    WidgetBase::drawTrailing(prevString, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, this->cfg.trailing);
    prevString[0] = '\0';
    prevW = 0;
    prevH = 0;
    firstDraw = true;
  }

 private:
  char prevString[TEXT_BUFFER_SIZE] = "";
  uint16_t prevW = 0;
  uint16_t prevH = 0;
  bool firstDraw = true;
};

class Button : public TextWidget<const char*> {
 public:
  constexpr Button(const char* text, const TextConfig& cfg, uint16_t rectWidth, uint16_t rectHeight, int16_t radius,
                   uint8_t linePad = 0)
      : TextWidget<const char*>(&storedText, cfg),
        storedText(text),
        rectWidth(rectWidth),
        rectHeight(rectHeight),
        radius(radius),
        linePad(linePad) {
    this->cfg.align = TextAlign::CENTER;
  }

  void draw() override {
    if (firstDraw) {
      int16_t x1, y1;
      uint16_t w, h;
      WidgetBase::textBlockBounds(storedText, this->cfg.x, this->cfg.y, this->cfg.size, linePad, x1, y1, w, h);
      int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
      int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
      WidgetBase::drawText(storedText, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, this->cfg.color,
                           linePad);
      Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, this->cfg.color);
      firstDraw = false;
    }
  }

  void clear() override {
    int16_t x1, y1;
    uint16_t w, h;
    WidgetBase::textBlockBounds(storedText, this->cfg.x, this->cfg.y, this->cfg.size, linePad, x1, y1, w, h);
    int16_t rectX = x1 - static_cast<int16_t>(rectWidth) / 2;
    int16_t rectY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
    WidgetBase::drawText(storedText, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK, linePad);
    Display::screen.drawRoundRect(rectX, rectY, rectWidth, rectHeight, radius, RGB565_BLACK);
    firstDraw = true;
  }

 private:
  const char* storedText;
  uint16_t rectWidth;
  uint16_t rectHeight;
  int16_t radius;
  uint8_t linePad;
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

// TODO display errors on screen?
// TODO Display average watts while driving, maybe 1min rolling average or something? Or exponential decay?
// TODO display battery voltage
