#include "display.h"

#include "hardware.h"
#include "utils/errors.h"
#include "widgets.h"

IntervalMetric Display::dataProcessTime = IntervalMetric();
Page* Display::currentPage = &RACE_PAGE;
Arduino_HWSPI Display::bus = Arduino_HWSPI(DISPLAY_DATA_COMMAND_PIN, DISPLAY_CHIP_SELECT_PIN);
Arduino_ILI9488_18bit Display::screen = Arduino_ILI9488_18bit(static_cast<Arduino_DataBus*>(&bus), DISPLAY_RESET_PIN, 1, false);

void Display::init(SPIClass* spi) {
  if (!screen.begin(8000000)) {
    Errors::logError(Error::DISPLAY_UNINITIALISED);
    return;
  }
  screen.fillScreen(RGB565_BLACK);
  digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);

  dataProcessTime.init(F("Display Updates"), F("Time to update Display"));
  enabled = true;
}

void Display::run() {
  if (enabled) {
    dataProcessTime.start();
    currentPage->refresh();
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
