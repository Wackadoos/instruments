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

  char buf[TEXT_BUFFER_SIZE];
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

void WidgetBase::drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color, uint8_t linePad) {
  int16_t calcX, calcY;
  uint16_t w, h;
  textBlockBounds(text, x, y, textSize, linePad, calcX, calcY, w, h);

  Display::screen.setTextColor(color, RGB565_BLACK);

  char buf[TEXT_BUFFER_SIZE];
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

void WidgetBase::textRegion(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint8_t linePad,
                            TextRegion& out) {
  textBlockBounds(text, x, y, textSize, linePad, out.x, out.y, out.w, out.h);
  if (align == TextAlign::CENTER) {
    out.x = x - static_cast<int16_t>(out.w) / 2;
  } else if (align == TextAlign::RIGHT) {
    out.x = x - static_cast<int16_t>(out.w);
  }
}

void WidgetBase::trailingRegion(const TrailingText& trailing, const TextRegion& main, TextRegion& out) {
  textRegion(trailing.text, main.x + static_cast<int16_t>(main.w) + static_cast<int16_t>(trailing.gap), main.y,
             TextAlign::LEFT, trailing.size, 0, out);
  out.y = main.y + static_cast<int16_t>(main.h) - static_cast<int16_t>(out.h) -
          static_cast<int16_t>(trailing.verticalPad);
}

void WidgetBase::eraseStale(const TextRegion& oldR, const TextRegion& newR) {
  int16_t oldRight = oldR.x + static_cast<int16_t>(oldR.w);
  int16_t oldBottom = oldR.y + static_cast<int16_t>(oldR.h);
  int16_t newRight = newR.x + static_cast<int16_t>(newR.w);
  int16_t newBottom = newR.y + static_cast<int16_t>(newR.h);

  int16_t leftW = newR.x - oldR.x;
  if (leftW > 0) {
    Display::screen.fillRect(oldR.x, oldR.y, leftW, oldR.h, RGB565_BLACK);
  }
  int16_t rightW = oldRight - newRight;
  if (rightW > 0) {
    Display::screen.fillRect(newRight, oldR.y, rightW, oldR.h, RGB565_BLACK);
  }
  int16_t topH = newR.y - oldR.y;
  if (topH > 0) {
    Display::screen.fillRect(newR.x, oldR.y, newR.w, topH, RGB565_BLACK);
  }
  int16_t bottomH = oldBottom - newBottom;
  if (bottomH > 0) {
    Display::screen.fillRect(newR.x, newBottom, newR.w, bottomH, RGB565_BLACK);
  }
}

void WidgetBase::drawTrailing(const char* mainText, int16_t x, int16_t y, TextAlign align, uint8_t mainTextSize,
                              const TrailingText& trailing) {
  if (trailing.text == nullptr || trailing.size == 0) {
    return;
  }

  TextRegion main;
  textRegion(mainText, x, y, align, mainTextSize, 0, main);

  TextRegion t;
  trailingRegion(trailing, main, t);

  Display::screen.setCursor(t.x, t.y);
  Display::screen.println(trailing.text);

  Display::screen.setTextSize(mainTextSize);
}
