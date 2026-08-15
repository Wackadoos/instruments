#include "display.h"

#include "hardware.h"
#include "utils/debug.h"
#include "utils/errors.h"
#include "widgets.h"

IntervalMetric Display::dataProcessTime = IntervalMetric();
Page* Display::currentPage = &RACE_PAGE;
Page* Display::pendingPage = nullptr;
bool Display::switching = false;
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
    if (switching) {
      // Tear down the old page one widget at a time so loop() keeps running.
      if (!currentPage->clearStep()) {
        switching = false;
        currentPage = pendingPage;
        pendingPage = nullptr;
      }
    } else {
      currentPage->refresh();
    }
    // Edge-triggered touch dispatch: a press fires exactly once, on the rising edge, so a
    // single touch can never trigger a neighbouring button via sample jitter or finger drift.
    // Holding still repeats the SAME button (same touch-down point) for settings +/- style
    // controls, but only while the finger stays within TOUCH_REPEAT_RADIUS_PX of where it
    // landed and the page hasn't changed. touchWasDown is tracked even during switching so a
    // press that began mid-teardown can't fire once the new page appears.
    bool nowTouched = touch.tirqTouched() && touch.touched();
    if (nowTouched) {
      TS_Point p = touch.getPoint();
      int16_t px, py;
      if (TOUCH_SWAP_XY) {
        px = map(p.y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, screen.width() - 1);
        py = map(p.x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, screen.height() - 1);
      } else {
        px = map(p.x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, screen.width() - 1);
        py = map(p.y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, screen.height() - 1);
      }
      if (!touchWasDown) {
        touchDownX = px;
        touchDownY = py;
        touchDownMs = millis();
        lastRepeatMs = touchDownMs;
        touchDownPage = currentPage;
        if (!switching) {
          currentPage->pressed(TS_Point(px, py, p.z));
        }
      } else if (!switching && currentPage == touchDownPage) {
        unsigned long now = millis();
        if (now - touchDownMs >= TOUCH_HOLD_DELAY_MS && now - lastRepeatMs >= TOUCH_REPEAT_PERIOD_MS &&
            abs(px - touchDownX) <= TOUCH_REPEAT_RADIUS_PX && abs(py - touchDownY) <= TOUCH_REPEAT_RADIUS_PX) {
          lastRepeatMs = now;
          currentPage->pressed(TS_Point(touchDownX, touchDownY, 0));
        }
      }
    }
    touchWasDown = nowTouched;
    dataProcessTime.stop();
  }
}

void Display::changePage(Page* page) {
  if (currentPage == page) {
    return;
  }
  pendingPage = page;
  switching = true;
}

bool Page::refresh() {
  for (size_t i = 0; i < count; i++) {
    size_t idx = (index + i) % count;
    if (widgets[idx]->draw()) {
      index = (idx + 1) % count;
      return true;
    }
  }
  return false;
}

bool Page::clearStep() {
  if (clearIndex >= count) {
    clearIndex = 0;
    return false;  // Whole page cleared
  }
  if (widgets[clearIndex]->clear()) {
    return true;  // Mid-clear: revisit this widget next call
  }
  clearIndex++;
  return true;
}

void Page::pressed(TS_Point point) {
  for (uint8_t i = 0; i < count; i++) {
    widgets[i]->pressed(point);
  }
}

void WidgetBase::copyString(const char* text, char* dst, size_t len) {
  strncpy(dst, text, len - 1);
  dst[len - 1] = '\0';
}

void WidgetBase::copyString(const __FlashStringHelper* text, char* dst, size_t len) {
  strncpy_P(dst, reinterpret_cast<const char*>(text), len - 1);
  dst[len - 1] = '\0';
}

void WidgetBase::textBlockBounds(const __FlashStringHelper* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                                 int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h) {
  char buf[TEXT_BUFFER_SIZE];
  copyString(text, buf, sizeof(buf));
  textBlockBounds(buf, x, y, textSize, linePad, outX, outY, w, h);
}

void WidgetBase::textBlockBounds(const char* text, int16_t x, int16_t y, uint8_t textSize, uint8_t linePad,
                                 int16_t& outX, int16_t& outY, uint16_t& w, uint16_t& h) {
  Display::screen.setTextSize(textSize);

  char buf[TEXT_BUFFER_SIZE];
  copyString(text, buf, sizeof(buf));

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

void WidgetBase::drawText(const __FlashStringHelper* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize,
                          uint16_t color, uint8_t linePad) {
  char buf[TEXT_BUFFER_SIZE];
  copyString(text, buf, sizeof(buf));
  drawText(buf, x, y, align, textSize, color, linePad);
}

void WidgetBase::drawText(const char* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize, uint16_t color, uint8_t linePad) {
  int16_t calcX, calcY;
  uint16_t w, h;
  textBlockBounds(text, x, y, textSize, linePad, calcX, calcY, w, h);

  Display::screen.setTextColor(color, RGB565_BLACK);

  char buf[TEXT_BUFFER_SIZE];
  copyString(text, buf, sizeof(buf));

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

void WidgetBase::textRegion(const __FlashStringHelper* text, int16_t x, int16_t y, TextAlign align, uint8_t textSize,
                            uint8_t linePad, TextRegion& out) {
  char buf[TEXT_BUFFER_SIZE];
  copyString(text, buf, sizeof(buf));
  textRegion(buf, x, y, align, textSize, linePad, out);
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

void WidgetBase::trailingRegion(const char* text, const TrailingText& trailing, const TextRegion& main, TextRegion& out) {
  textRegion(text, main.x + static_cast<int16_t>(main.w) + static_cast<int16_t>(trailing.gap), main.y,
             TextAlign::LEFT, trailing.size, 0, out);
  out.y = main.y + static_cast<int16_t>(main.h) - static_cast<int16_t>(out.h) -
          static_cast<int16_t>(trailing.verticalPad);
}

int16_t WidgetBase::mainAnchorX(int16_t x, const char* trailingText, const TrailingText& trailing, TextAlign align) {
  if (align != TextAlign::RIGHT || trailingText == nullptr || !trailing.present()) {
    return x;
  }
  TextRegion t;
  textRegion(trailingText, 0, 0, TextAlign::LEFT, trailing.size, 0, t);
  return x - static_cast<int16_t>(t.w) - static_cast<int16_t>(trailing.gap);
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
                              uint16_t color, const TrailingText& trailing) {
  if (!trailing.present()) {
    return;
  }

  TextRegion main;
  textRegion(mainText, x, y, align, mainTextSize, 0, main);

  TextRegion t;
  trailingRegion(trailing.current(), trailing, main, t);

  Display::screen.setTextColor(color, RGB565_BLACK);
  Display::screen.setCursor(t.x, t.y);
  Display::screen.println(trailing.current());

  Display::screen.setTextSize(mainTextSize);
}

// ---- DebugText ----

static uint16_t fnv16(const char* s) {
  uint32_t h = 0x811C9DC5u;
  while (*s) {
    h ^= (uint8_t)(*s++);
    h *= 0x01000193u;
  }
  return (uint16_t)h;
}

static uint16_t fnv16n(const char* s, size_t len) {
  uint32_t h = 0x811C9DC5u;
  for (size_t i = 0; i < len; i++) {
    h ^= (uint8_t)s[i];
    h *= 0x01000193u;
  }
  return (uint16_t)h;
}

DebugText::DebugText(int16_t x, int16_t y, uint8_t linePitch, uint8_t contentLines, uint16_t color)
    : x(x), y(y), linePitch(linePitch), contentLines(contentLines), color(color) {
  resetLines();
}

void DebugText::resetLines() {
  memset(valueX, 0, sizeof(valueX));
  memset(hash, 0, sizeof(hash));
  memset(prefixHash, 0, sizeof(prefixHash));
  memset(prevFullW, 0, sizeof(prevFullW));
  memset(prevW, 0, sizeof(prevW));
  memset(present, 0, sizeof(present));
  lineCursor = 0;
}

// Erases a band. Band 0 is the header; bands 1..contentLines are the lines. Erasure is
// bounded by the last-*drawn* widths (prevW/prevFullW), not the current item text.
//
// IMPORTANT: this library's drawChar skips painting the background when bg == color, so
// drawing black text only writes the glyphs' lit pixels. That fully erases a *matching*
// string but leaves residue wherever the old and new strings differ. Values and wholeLine
// error prefixes can change between the last draw and this erase, so those regions are
// erased with a fillRect box. Only the static label (and header) are erased with identical
// black text, which overwrites every old pixel.
void DebugText::eraseBand(uint8_t band) const {
  if (drawnPage == 0xFF) {
    return;  // Never drawn; nothing to erase
  }
  if (band == 0) {
    char hdr[] = "DEBUG 0/0";
    hdr[6] = (char)('0' + drawnPage + 1);
    hdr[8] = (char)('0' + debugPageCount());
    drawLine(hdr, x, y, RGB565_BLACK);
    return;
  }
  uint8_t cl = band - 1;
  if (!present[cl]) {
    return;  // Slot was never drawn on the old page
  }
  int16_t ly = y + (int16_t)band * linePitch;
  char buf[DEBUG_LINE_BUF];
  char labelBuf[16];
  DebugLine dl;
  if (!debugItem(drawnPage, cl, dl, buf, sizeof(buf))) {
    // Slot's content vanished since it was drawn; clear the recorded line width.
    Display::screen.fillRect(x, ly, prevFullW[cl], linePitch, RGB565_BLACK);
    return;
  }
  if (dl.wholeLine) {
    // Prefix (time + description) can rotate between errors; box the whole recorded line.
    Display::screen.fillRect(x, ly, prevFullW[cl], linePitch, RGB565_BLACK);
    return;
  }
  // Label is static (identical on screen), so black text erases it cleanly; the value may
  // have changed since it was drawn, so box its recorded region.
  WidgetBase::copyString(dl.label, labelBuf, sizeof(labelBuf));
  drawLine(labelBuf, x, ly, RGB565_BLACK);
  if (prevW[cl] > 0) {
    int16_t vx = (valueX[cl] > x) ? valueX[cl] : x + (int16_t)textWidth(labelBuf) + DEBUG_LABEL_GAP;
    Display::screen.fillRect(vx, ly, prevW[cl], linePitch, RGB565_BLACK);
  }
}

bool DebugText::clear() {
  if (clearLine <= contentLines) {
    eraseBand(clearLine++);
    return true;  // Still erasing: page teardown will revisit this widget
  }
  clearLine = 0;
  eraseLine = 0;
  resetLines();
  firstDraw = true;
  drawnPage = 0xFF;
  return false;
}

void DebugText::prev() {
  uint8_t total = debugPageCount();
  page = (page == 0) ? total - 1 : page - 1;
}

void DebugText::next() {
  uint8_t total = debugPageCount();
  page = (page + 1 >= total) ? 0 : page + 1;
}

void DebugText::reset() {
  page = 0;
  eraseLine = 0;
}

void DebugText::drawLine(const char* text, int16_t lx, int16_t ly, uint16_t col) const {
  Display::screen.setTextSize(1);
  Display::screen.setTextColor(col, RGB565_BLACK);
  Display::screen.setCursor(lx, ly);
  Display::screen.print(text);
}

uint16_t DebugText::textWidth(const char* text) const {
  int16_t x1, y1;
  uint16_t w, h;
  Display::screen.setTextSize(1);
  Display::screen.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}

void DebugText::drawHeader() {
  char hdr[] = "DEBUG 0/0";  // page+1 and total are single digits (3 pages)
  hdr[6] = (char)('0' + page + 1);
  hdr[8] = (char)('0' + debugPageCount());
  drawLine(hdr, x, y, color);
}

// Draws the fixed prefix of a line and returns the x where its value segment starts. For
// wholeLine items the prefix is buf[0..valueOffset] (buf is temporarily split); otherwise it
// is the flash label. Text is drawn with a black background, so it overwrites the old glyphs.
int16_t DebugText::drawPrefix(char* buf, const DebugLine& dl, const char* labelBuf, int16_t lineY, uint16_t col) const {
  if (!dl.wholeLine) {
    drawLine(labelBuf, x, lineY, col);
    return x + (int16_t)textWidth(labelBuf) + DEBUG_LABEL_GAP;
  }
  if (dl.valueOffset == 0) {
    drawLine(buf, x, lineY, col);
    return x + (int16_t)textWidth(buf);
  }
  char saved = buf[dl.valueOffset];
  buf[dl.valueOffset] = '\0';
  drawLine(buf, x, lineY, col);
  uint16_t w = textWidth(buf);
  buf[dl.valueOffset] = saved;
  return x + (int16_t)w;
}

bool DebugText::draw() {
  if (firstDraw) {
    // Boot / after teardown: region already black, render fresh.
    firstDraw = false;
    drawnPage = page;
    drawHeader();
    resetLines();
  } else if (page != drawnPage) {
    // In-place page transition: erase the old page one band at a time.
    if (eraseLine <= contentLines) {
      eraseBand(eraseLine++);
      return true;
    }
    eraseLine = 0;
    drawnPage = page;
    drawHeader();
    resetLines();
  }

  char buf[DEBUG_LINE_BUF];
  char labelBuf[16];
  for (uint8_t i = 0; i < contentLines; i++) {
    uint8_t cl = (uint8_t)(lineCursor + i) % contentLines;
    int16_t lineY = y + (int16_t)(cl + 1) * linePitch;
    DebugLine dl;
    if (!debugItem(page, cl, dl, buf, sizeof(buf))) {
      if (present[cl]) {
        // Line vanished (no current page does this; harmless fallback).
        Display::screen.fillRect(x, lineY, prevFullW[cl], linePitch, RGB565_BLACK);
        present[cl] = 0;
        hash[cl] = 0;
        lineCursor = (uint8_t)(cl + 1) % contentLines;
        return true;
      }
      continue;
    }

    uint16_t h = fnv16(buf);
    if (present[cl] && h == hash[cl]) {
      continue;
    }

    if (!dl.wholeLine) {
      WidgetBase::copyString(dl.label, labelBuf, sizeof(labelBuf));
    }
    uint16_t prefixHashVal = dl.wholeLine ? ((dl.valueOffset > 0) ? fnv16n(buf, dl.valueOffset) : fnv16(buf))
                                          : fnv16(labelBuf);
    uint16_t valueWidth = dl.wholeLine ? ((dl.valueOffset > 0) ? (uint16_t)textWidth(buf + dl.valueOffset) : 0)
                                       : (uint16_t)textWidth(buf);

    // A wholeLine with valueOffset == 0 is entirely a "prefix", so it always goes through the
    // full redraw below (e.g. an error count returning to 1 must not re-draw the whole line
    // at valueX). Otherwise only the prefix hash decides prefix vs value-only redraw.
    if (!present[cl] || prefixHashVal != prefixHash[cl] || (dl.wholeLine && dl.valueOffset == 0)) {
      // Full (re)draw: prefix + value. Black-background text overwrites the old line; the
      // rare prefix change (error slot replaced) clears the old line's own width first.
      if (present[cl]) {
        Display::screen.fillRect(x, lineY, prevFullW[cl], linePitch, RGB565_BLACK);
      }
      valueX[cl] = drawPrefix(buf, dl, labelBuf, lineY, color);
      if (dl.wholeLine) {
        if (dl.valueOffset > 0) {
          drawLine(buf + dl.valueOffset, valueX[cl], lineY, color);
        }
      } else {
        drawLine(buf, valueX[cl], lineY, color);
      }
      prevW[cl] = valueWidth;
      prefixHash[cl] = prefixHashVal;
    } else {
      // Value-only update: the black background overwrites the old value; only a shrunk
      // value tail needs clearing (no full-line rect, so no black flash).
      if (prevW[cl] > valueWidth) {
        Display::screen.fillRect(valueX[cl] + valueWidth, lineY, prevW[cl] - valueWidth, linePitch, RGB565_BLACK);
      }
      drawLine(buf + (dl.wholeLine ? dl.valueOffset : 0), valueX[cl], lineY, color);
      prevW[cl] = valueWidth;
    }

    prevFullW[cl] = (uint16_t)(valueX[cl] + prevW[cl] - x);
    hash[cl] = h;
    present[cl] = 1;
    lineCursor = (uint8_t)(cl + 1) % contentLines;
    return true;  // One line per call; other widgets get their turns in between
  }
  return false;
}
