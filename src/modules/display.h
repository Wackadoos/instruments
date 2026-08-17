#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <XPT2046_Touchscreen.h>

#include "utils/metrics.h"

#define TEXT_BUFFER_SIZE 16
#define DEBOUNCE_PERIOD 300          // Guards bounce/fast double-taps now that presses are edge-triggered
#define TOUCH_HOLD_DELAY_MS 600      // Hold time before a press starts repeating
#define TOUCH_REPEAT_PERIOD_MS 400   // Interval between hold repeats
#define TOUCH_REPEAT_RADIUS_PX 30    // Max finger drift from touch-down that still allows repeat

class Page;
struct DebugLine;

class Display {
 public:
  static void init(SPIClass* spi);
  static void run();
  static inline bool isEnabled() { return enabled; };
  static void changePage(Page* page);

  static Arduino_ILI9488_18bit screen;
  static XPT2046_Touchscreen touch;

 private:
  static IntervalMetric dataProcessTime;
  inline static bool enabled = false;
  static Arduino_HWSPI bus;
  static Page* currentPage;
  static Page* pendingPage;   // Target page during a non-blocking switch
  static bool switching;      // True while the old page is being torn down
  inline static bool touchWasDown = false;   // Previous sample's touched state (edge detection)
  inline static int16_t touchDownX = 0;      // Mapped screen point where the current touch landed
  inline static int16_t touchDownY = 0;
  inline static unsigned long touchDownMs = 0;  // Touch-down timestamp (hold timer base)
  inline static unsigned long lastRepeatMs = 0; // Timestamp of the last hold repeat
  inline static Page* touchDownPage = nullptr;  // Page the current touch landed on (repeats stop on switch)
};

enum class TextAlign : uint8_t { LEFT,
                                 CENTER,
                                 RIGHT };

struct TrailingText {
  const char* text = nullptr;
  uint8_t size = 0;
  uint8_t gap = 2;
  uint8_t verticalPad = 0;

  constexpr TrailingText() = default;
  constexpr TrailingText(const char* t, uint8_t s = 0, uint8_t g = 2, uint8_t v = 0)
      : text(t), size(s), gap(g), verticalPad(v) {}

  bool present() const { return size != 0 && text != nullptr; }
  const char* current() const { return text; }
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
  // Returns true if a screen write was performed this call.
  virtual bool draw() = 0;
  // Returns true if clearing is still in progress (widget will be revisited), false when fully cleared.
  virtual bool clear() = 0;
  virtual void pressed(TS_Point point) = 0;
  virtual ~WidgetBase() = default;
  constexpr WidgetBase() = default;

  // Copy text from RAM or flash (F()) into a caller buffer; always NUL-terminates.
  static void copyString(const char* text, char* dst, size_t len);
  static void copyString(const __FlashStringHelper* text, char* dst, size_t len);

  // Flash (F()) overloads: copy into a temp buffer then delegate to the RAM versions.
  // Keeps the main RAM path untouched for speed; used by flash-resident debug-page strings.
  static void textBlockBounds(const char* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                              int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h);
  static void textBlockBounds(const __FlashStringHelper* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                              int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h);
  static void drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color,
                       uint8_t linePad = 0);
  static void drawText(const __FlashStringHelper* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize,
                       uint16_t color, uint8_t linePad = 0);
  static void drawTrailing(const char* mainText, int16_t x, int16_t y, TextAlign align, uint8_t mainTextSize,
                           uint16_t color, const TrailingText& trailing);
  static void textRegion(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint8_t linePad,
                         TextRegion& out);
  static void textRegion(const __FlashStringHelper* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize,
                         uint8_t linePad, TextRegion& out);
  static void trailingRegion(const char* text, const TrailingText& trailing, const TextRegion& main, TextRegion& out);
  static void eraseStale(const TextRegion& oldR, const TextRegion& newR);
  static int16_t mainAnchorX(int16_t x, const char* trailingText, const TrailingText& trailing, TextAlign align);
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

  // Explicit overloads so integer widgets don't fall through to the heap-allocating String template
  static void formatValue(uint8_t value, char* buf, size_t len, uint8_t) {
    ultoa(value, buf, 10);
  }

  static void formatValue(uint16_t value, char* buf, size_t len, uint8_t) {
    ultoa(value, buf, 10);
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
    int16_t x = WidgetBase::mainAnchorX(cfg.x, cfg.trailing.present() ? cfg.trailing.current() : nullptr, cfg.trailing,
                                        cfg.align);
    WidgetBase::drawText(buf, x, cfg.y, cfg.align, cfg.size, color);
    WidgetBase::drawTrailing(buf, x, cfg.y, cfg.align, cfg.size, color, cfg.trailing);
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

  bool draw() override {
    if (firstDraw) {
      this->render(*this->data, this->cfg.color);
      firstDraw = false;
      return true;
    }
    return false;
  }

  bool clear() override {
    this->render(*this->data, RGB565_BLACK);
    firstDraw = true;
    return false;
  }

 private:
  const char* storedText;
  bool firstDraw = true;
};

template <typename T>
class Widget : public TextWidget<T> {
 public:
  constexpr Widget(T* data, const TextConfig& cfg) : TextWidget<T>(data, cfg) {}

  bool draw() override {
    char buf[TEXT_BUFFER_SIZE];
    WidgetPrinter::format(*this->data, buf, sizeof(buf), this->cfg.decimalDigits);
    uint16_t color = colorFor(*this->data);

    bool hasTrailing = this->cfg.trailing.present();
    const char* trailingStr = hasTrailing ? this->cfg.trailing.current() : "";
    bool trailingChanged = hasTrailing && strcmp(trailingStr, prevTrailing) != 0;

    bool textChanged = strcmp(buf, prevString) != 0;
    bool colorChanged = color != prevColor;
    bool needMain = firstDraw || textChanged || colorChanged;

    if (!needMain && !trailingChanged) {
      return false;
    }

    int16_t mainX = WidgetBase::mainAnchorX(this->cfg.x, trailingStr, this->cfg.trailing, this->cfg.align);

    TextRegion main;
    WidgetBase::textRegion(buf, mainX, this->cfg.y, this->cfg.align, this->cfg.size, 0, main);

    bool sizeChanged = firstDraw || main.w != prevW || main.h != prevH;

    TextRegion oldR;
    if (!firstDraw) {
      int16_t oldX = WidgetBase::mainAnchorX(this->cfg.x, prevTrailing, this->cfg.trailing, this->cfg.align);
      oldR.x = oldX;
      oldR.y = this->cfg.y;
      oldR.w = prevW;
      oldR.h = prevH;
      if (this->cfg.align == TextAlign::CENTER) {
        oldR.x -= static_cast<int16_t>(prevW / 2);
      } else if (this->cfg.align == TextAlign::RIGHT) {
        oldR.x -= static_cast<int16_t>(prevW);
      }
    }

    if (needMain) {
      if (!firstDraw) {
        WidgetBase::eraseStale(oldR, main);
      }
      prevW = main.w;
      prevH = main.h;
      strncpy(prevString, buf, sizeof(prevString) - 1);
      prevString[sizeof(prevString) - 1] = '\0';
      WidgetBase::drawText(buf, mainX, this->cfg.y, this->cfg.align, this->cfg.size, color);
    }

    if (hasTrailing && (sizeChanged || colorChanged || trailingChanged)) {
      TextRegion newTrailing;
      WidgetBase::trailingRegion(trailingStr, this->cfg.trailing, main, newTrailing);
      if (!firstDraw) {
        TextRegion oldTrailing;
        WidgetBase::trailingRegion(prevTrailing, this->cfg.trailing, oldR, oldTrailing);
        WidgetBase::eraseStale(oldTrailing, newTrailing);
      }
      WidgetBase::drawTrailing(buf, mainX, this->cfg.y, this->cfg.align, this->cfg.size, color, this->cfg.trailing);
      strncpy(prevTrailing, trailingStr, sizeof(prevTrailing) - 1);
      prevTrailing[sizeof(prevTrailing) - 1] = '\0';
    }

    prevColor = color;
    firstDraw = false;
    return true;
  }

  bool clear() override {
    int16_t mainX = WidgetBase::mainAnchorX(this->cfg.x, this->cfg.trailing.present() ? prevTrailing : nullptr,
                                            this->cfg.trailing, this->cfg.align);
    WidgetBase::drawText(prevString, mainX, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK);
    WidgetBase::drawTrailing(prevString, mainX, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK,
                             this->cfg.trailing);
    prevString[0] = '\0';
    prevTrailing[0] = '\0';
    prevW = 0;
    prevH = 0;
    prevColor = 0;
    firstDraw = true;
    return false;
  }

 protected:
  virtual uint16_t colorFor(T value) { return static_cast<uint16_t>(this->cfg.color); }

 private:
  char prevString[TEXT_BUFFER_SIZE] = "";
  char prevTrailing[TEXT_BUFFER_SIZE] = "";
  uint16_t prevW = 0;
  uint16_t prevH = 0;
  uint16_t prevColor = 0;
  bool firstDraw = true;
};

template <typename T>
class SetpointWidget : public Widget<T> {
 public:
  constexpr SetpointWidget(T* data, const TextConfig& cfg, const T* warningSetpoint, const T* errorSetpoint,
                           uint16_t baseColor, uint16_t warningColor, uint16_t errorColor, bool lowEscalates = false)
      : Widget<T>(data, cfg),
        warningSetpoint(warningSetpoint),
        errorSetpoint(errorSetpoint),
        baseColor(baseColor),
        warningColor(warningColor),
        errorColor(errorColor),
        lowEscalates(lowEscalates) {}

 protected:
  uint16_t colorFor(T value) override {
    if (lowEscalates) {
      if (errorSetpoint && value <= *errorSetpoint) return errorColor;
      if (warningSetpoint && value <= *warningSetpoint) return warningColor;
    } else {
      if (errorSetpoint && value >= *errorSetpoint) return errorColor;
      if (warningSetpoint && value >= *warningSetpoint) return warningColor;
    }
    return baseColor;
  }

 private:
  const T* warningSetpoint;
  const T* errorSetpoint;
  uint16_t baseColor;
  uint16_t warningColor;
  uint16_t errorColor;
  bool lowEscalates;
};

class Button : public TextWidget<const char*> {
 public:
  using PressCallback = void (*)();

  constexpr Button(const char* text, const TextConfig& cfg, uint16_t rectWidth, uint16_t rectHeight, int16_t radius,
                   uint8_t linePad = 0, PressCallback onPress = nullptr)
      : TextWidget<const char*>(&storedText, cfg),
        storedText(text),
        rectWidth(rectWidth),
        rectHeight(rectHeight),
        radius(radius),
        linePad(linePad),
        onPress(onPress) {
    this->cfg.align = TextAlign::CENTER;
  }

  void setText(const char* text) {
    if (text == storedText) return;
    if (storedText != nullptr && text != nullptr && strcmp(storedText, text) == 0) return;
    storedText = text;
    dirty = true;
  }

  void setColor(int16_t color) {
    if (this->cfg.color == color) return;
    this->cfg.color = color;
    dirty = true;
  }

  bool draw() override {
    if (firstDraw || dirty) {
      if (dirty && !firstDraw) {
        int16_t rectX, rectY;
        uint16_t rw, rh;
        rectBounds(prevText, rectX, rectY, rw, rh);
        WidgetBase::drawText(prevText, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK, linePad);
        Display::screen.drawRoundRect(rectX, rectY, rw, rh, radius, RGB565_BLACK);
      }
      int16_t rectX, rectY;
      uint16_t rw, rh;
      rectBounds(storedText, rectX, rectY, rw, rh);
      WidgetBase::drawText(storedText, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, this->cfg.color,
                           linePad);
      Display::screen.drawRoundRect(rectX, rectY, rw, rh, radius, this->cfg.color);
      prevText = storedText;
      firstDraw = false;
      dirty = false;
      return true;
    }
    return false;
  }

  bool clear() override {
    const char* text = (prevText != nullptr) ? prevText : storedText;
    int16_t rectX, rectY;
    uint16_t rw, rh;
    rectBounds(text, rectX, rectY, rw, rh);
    WidgetBase::drawText(text, this->cfg.x, this->cfg.y, this->cfg.align, this->cfg.size, RGB565_BLACK, linePad);
    Display::screen.drawRoundRect(rectX, rectY, rw, rh, radius, RGB565_BLACK);
    firstDraw = true;
    dirty = false;
    prevText = nullptr;
    return false;
  }

  void pressed(TS_Point point) override {
    if (onPress == nullptr) {
      return;
    }
    uint32_t now = millis();
    if (now - lastPressMs < DEBOUNCE_PERIOD) {
      return;
    }
    int16_t rectX, rectY;
    uint16_t rw, rh;
    rectBounds(storedText, rectX, rectY, rw, rh);
    if (point.x >= rectX && point.x < rectX + static_cast<int16_t>(rw) && point.y >= rectY &&
        point.y < rectY + static_cast<int16_t>(rh)) {
      lastPressMs = now;
      onPress();
    }
  }

 private:
  void rectBounds(const char* text, int16_t& outX, int16_t& outY, uint16_t& outW, uint16_t& outH) const {
    int16_t x1, y1;
    uint16_t w, h;
    WidgetBase::textBlockBounds(text, this->cfg.x, this->cfg.y, this->cfg.size, linePad, x1, y1, w, h);
    outX = x1 - static_cast<int16_t>(rectWidth) / 2;
    outY = y1 + (static_cast<int16_t>(h) - static_cast<int16_t>(rectHeight)) / 2;
    outW = rectWidth;
    outH = rectHeight;
  }

  const char* storedText;
  const char* prevText = nullptr;
  uint16_t rectWidth;
  uint16_t rectHeight;
  int16_t radius;
  uint8_t linePad;
  PressCallback onPress;
  uint32_t lastPressMs = 0;
  bool firstDraw = true;
  bool dirty = false;
};

class Page {
 public:
  template <size_t N>
  Page(WidgetBase* const (&widgets)[N]) : widgets(widgets), count(N) {}

  size_t size() const { return count; }
  const WidgetBase* operator[](size_t i) const { return widgets[i]; }

  // Round-robin: draws at most one widget per call (the first that reports a screen write).
  bool refresh();
  // Teardown: clears at most one widget per call; stays on a widget while its clear() reports in progress.
  bool clearStep();
  void pressed(TS_Point point);

 private:
  WidgetBase* const* widgets;
  size_t count;
  size_t index = 0;       // Round-robin cursor: next widget to consider
  size_t clearIndex = 0;  // Teardown cursor: next widget to clear
};

//* Debug page: single widget rendering a paginated list of flash-resident debug lines.
//* Labels are drawn once per page; only the changed value tail re-renders (never the leading text).
#define DEBUG_MAX_LINES 24   // Max content lines per page
#define DEBUG_LINE_BUF 64    // Longest rendered line (error descriptions)
#define DEBUG_LABEL_GAP 4    // Pixels between label and value

class DebugText : public WidgetBase {
 public:
  DebugText(int16_t x, int16_t y, uint8_t linePitch, uint8_t contentLines, uint16_t color);

  bool draw() override;
  bool clear() override;
  void pressed(TS_Point point) override {}

  void prev();  // Wrap backwards
  void next();  // Wrap forwards
  void reset();  // Back to page 0 (erasure handled by draw's page-transition path)

 private:
  void drawLine(const char* text, int16_t lx, int16_t ly, uint16_t col) const;
  uint16_t textWidth(const char* text) const;
  void drawHeader();
  void eraseBand(uint8_t band) const;
  void resetLines();
  // Draws the fixed prefix (wholeLine: buf[0..valueOffset]; else the label) and returns the value start x.
  int16_t drawPrefix(char* buf, const DebugLine& dl, const char* labelBuf, int16_t lineY, uint16_t col) const;

  int16_t x;
  int16_t y;
  uint8_t linePitch;
  uint8_t contentLines;
  uint16_t color;
  uint8_t page = 0;
  uint8_t drawnPage = 0xFF;
  bool firstDraw = true;
  uint8_t eraseLine = 0;  // Page-transition erasure cursor (band 0 = header, then content lines)
  uint8_t clearLine = 0;  // Teardown erasure cursor (same band indexing)
  uint8_t lineCursor = 0; // Round-robin cursor into content lines (prevents a hot line starving the rest)
  int16_t valueX[DEBUG_MAX_LINES];       // Start x of the value segment (label + gap)
  uint16_t hash[DEBUG_MAX_LINES];        // FNV-16 of the last rendered debugItem buffer
  uint16_t prefixHash[DEBUG_MAX_LINES];  // FNV-16 of the last rendered prefix (label / time+description)
  uint16_t prevFullW[DEBUG_MAX_LINES];   // Pixel width of the whole last-drawn line (prefix + value)
  uint16_t prevW[DEBUG_MAX_LINES];       // Pixel width of the last rendered value segment
  uint8_t present[DEBUG_MAX_LINES];      // Line currently drawn
};
