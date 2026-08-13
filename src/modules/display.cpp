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

void WidgetBase::writeText(const char* text, int16_t x, int16_t y, TextAlign align = TextAlign::LEFT) {
  int16_t calcX = x;
  int16_t calcY = y;

  if (align != TextAlign::LEFT) {
    int16_t x1, y1;
    uint16_t w, h;
    Display::screen.getTextBounds(text, x, y, &x1, &y1, &w, &h);

    if (align == TextAlign::CENTER) {
      calcX = x - w / 2;
    } else if (align == TextAlign::RIGHT) {
      calcX = x - w;
    }
  }

  Display::screen.setCursor(calcX, calcY);
  Display::screen.println(text);
}

void WidgetBase::drawTrailing(const char* mainText, const char* trailingText, int16_t x, int16_t y, TextAlign align,
                              uint8_t trailingTextSize, uint8_t trailingGap, uint8_t trailingVerticalPad) {
  if (trailingText == nullptr || trailingTextSize == 0) {
    return;
  }

  int16_t x1, y1;
  uint16_t w, h;
  Display::screen.getTextBounds(mainText, x, y, &x1, &y1, &w, &h);

  int16_t mainX = x;
  if (align == TextAlign::CENTER) {
    mainX = x - static_cast<int16_t>(w) / 2;
  } else if (align == TextAlign::RIGHT) {
    mainX = x - static_cast<int16_t>(w);
  }

  Display::screen.setTextSize(trailingTextSize);
  int16_t tx1, ty1;
  uint16_t tw, th;
  Display::screen.getTextBounds(trailingText, mainX + w, y, &tx1, &ty1, &tw, &th);

  int16_t tX = mainX + static_cast<int16_t>(w) + static_cast<int16_t>(trailingGap);
  int16_t tY = y + static_cast<int16_t>(h) - static_cast<int16_t>(th) - static_cast<int16_t>(trailingVerticalPad);
  Display::screen.setCursor(tX, tY);
  Display::screen.println(trailingText);
}
