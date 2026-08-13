#include "display.h"

#include "hardware.h"
#include "utils/errors.h"
#include "widgets.h"

IntervalMetric Display::dataProcessTime = IntervalMetric();
Page* Display::currentPage = &RACE_PAGE;
Arduino_HWSPI Display::bus = Arduino_HWSPI(DISPLAY_DATA_COMMAND_PIN, DISPLAY_CHIP_SELECT_PIN);
Arduino_ILI9488_18bit Display::screen = Arduino_ILI9488_18bit(static_cast<Arduino_DataBus*>(&bus), DISPLAY_RESET_PIN, 1, false);
XPT2046_Touchscreen Display::touch = XPT2046_Touchscreen(TOUCHSCREEN_CHIP_SELECT_PIN, TOUCHSCREEN_INTERRUPT_PIN);

void Display::init(SPIClass* spi) {
  if (!touch.begin(*spi)) {
    Errors::logError(Error::TOUCHSCREEN_UNINITIALISED);
    return;
  }
  touch.setRotation(1);

  if (!screen.begin(8000000)) {
    Errors::logError(Error::DISPLAY_UNINITIALISED);
    return;
  }
  screen.fillScreen(RGB565_BLACK);
  screen.setTextWrap(false);
  digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);

  dataProcessTime.init(F("Display Updates"), F("Time to update Display"));
  enabled = true;
}

void Display::run() {
  if (enabled) {
    dataProcessTime.start();
    currentPage->refresh();
    if (touch.tirqTouched()) {
      if (touch.touched()) {
        TS_Point p = touch.getPoint();
        currentPage->pressed(touch.getPoint());
      }
    }
    dataProcessTime.stop();
  }
}

void Page::refresh() {
  for (uint8_t i = 0; i < count; i++) {
    widgets[i]->draw();
  }
}

void Page::clear() {
  for (uint8_t i = 0; i < count; i++) {
    widgets[i]->clear();
  }
}

void Page::pressed(TS_Point point) {
  for (uint8_t i = 0; i < count; i++) {
    widgets[i]->pressed(point);
  }
}

void WidgetBase::textBlockBounds(const char* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                                 int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h) {
  Display::screen.setTextSize(textSize);

  char buf[32];
  strncpy(buf, text, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  uint16_t lineHeight = 8 * textSize;
  uint16_t lineCount = 1;
  uint16_t maxW = 0;
  char* line = buf;
  for (char* p = buf;; p++) {
    if (*p == '\n' || *p == '\0') {
      bool last = (*p == '\0');
      *p = '\0';
      int16_t lx1, ly1;
      uint16_t lw, lh;
      Display::screen.getTextBounds(line, x, y, &lx1, &ly1, &lw, &lh);
      if (lw > maxW) {
        maxW = lw;
      }
      if (last) {
        break;
      }
      lineCount++;
      line = p + 1;
    }
  }

  outX = x;
  outY = y;
  w = maxW;
  h = lineCount * lineHeight + (lineCount - 1) * linePad;
}

void WidgetBase::textAnchor(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint8_t linePad,
                            int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h) {
  textBlockBounds(text, x, y, textSize, linePad, outX, outY, w, h);

  if (align == TextAlign::CENTER) {
    outX = x - static_cast<int16_t>(w) / 2;
  } else if (align == TextAlign::RIGHT) {
    outX = x - static_cast<int16_t>(w);
  }
}

void WidgetBase::drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color,
                          uint8_t linePad) {
  int16_t calcX, calcY;
  uint16_t w, h;
  textBlockBounds(text, x, y, textSize, linePad, calcX, calcY, w, h);

  Display::screen.setTextColor(color, RGB565_BLACK);

  char buf[32];
  strncpy(buf, text, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  uint16_t lineHeight = 8 * textSize;
  char* line = buf;
  uint16_t i = 0;
  for (char* p = buf;; p++) {
    if (*p == '\n' || *p == '\0') {
      bool last = (*p == '\0');
      *p = '\0';

      int16_t lx1, ly1;
      uint16_t lw, lh;
      Display::screen.getTextBounds(line, x, y, &lx1, &ly1, &lw, &lh);
      int16_t lineX = x;
      if (align == TextAlign::CENTER) {
        lineX = x - static_cast<int16_t>(lw) / 2;
      } else if (align == TextAlign::RIGHT) {
        lineX = x - static_cast<int16_t>(lw);
      }
      int16_t lineY = y + static_cast<int16_t>(i * (lineHeight + linePad));

      Display::screen.setCursor(lineX, lineY);
      Display::screen.println(line);
      if (last) {
        break;
      }
      i++;
      line = p + 1;
    }
  }
}

void WidgetBase::drawTrailing(const char* mainText, int16_t x, int16_t y, TextAlign align, uint8_t mainTextSize,
                              const TrailingText& trailing) {
  if (trailing.text == nullptr || trailing.size == 0) {
    return;
  }

  int16_t mainX, mainY;
  uint16_t w, h;
  textAnchor(mainText, x, y, align, mainTextSize, 0, mainX, mainY, w, h);

  Display::screen.setTextSize(trailing.size);
  int16_t tx, ty;
  uint16_t tw, th;
  textAnchor(trailing.text, mainX + static_cast<int16_t>(w), y, TextAlign::LEFT, trailing.size, 0, tx, ty, tw, th);

  int16_t tX = mainX + static_cast<int16_t>(w) + static_cast<int16_t>(trailing.gap);
  int16_t tY = y + static_cast<int16_t>(h) - static_cast<int16_t>(th) - static_cast<int16_t>(trailing.verticalPad);
  Display::screen.setCursor(tX, tY);
  Display::screen.println(trailing.text);

  Display::screen.setTextSize(mainTextSize);
}
