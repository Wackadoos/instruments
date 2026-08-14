#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "hardware.h"

Arduino_HWSPI bus = Arduino_HWSPI(DISPLAY_DATA_COMMAND_PIN, DISPLAY_CHIP_SELECT_PIN);
Arduino_GFX* gfx = new Arduino_ILI9488_18bit(static_cast<Arduino_DataBus*>(&bus), DISPLAY_RESET_PIN, 1, false);

int16_t w = -1, h = -1;
int16_t point_x[4] = {-1};
int16_t point_y[4] = {-1};
int16_t current_point = -1, next_point = 0;
int16_t touched_x[4] = {-1};
int16_t touched_y[4] = {-1};

#define TOUCH_XPT2046_ROTATION 1
#define TOUCH_XPT2046_SAMPLES 50

// Please fill below values from Arduino_GFX Example - TouchCalibration
// bool touch_swap_xy = false;
// int16_t touch_map_x1 = -1;
// int16_t touch_map_x2 = -1;
// int16_t touch_map_y1 = -1;
// int16_t touch_map_y2 = -1;
bool touch_swap_xy = false;
int16_t touch_map_x1 = 3950;
int16_t touch_map_x2 = 150;
int16_t touch_map_y1 = 3850;
int16_t touch_map_y2 = 100;

int16_t touch_max_x = 0, touch_max_y = 0;
int16_t touch_raw_x = 0, touch_raw_y = 0;
int16_t touch_last_x = 0, touch_last_y = 0;

#include <SPI.h>
#include <XPT2046_Touchscreen.h>
XPT2046_Touchscreen ts(TOUCHSCREEN_CHIP_SELECT_PIN, TOUCHSCREEN_INTERRUPT_PIN);

void touch_init(int16_t w, int16_t h, uint8_t r) {
  touch_max_x = w - 1;
  touch_max_y = h - 1;
  if (touch_map_x1 == -1) {
    switch (r) {
      case 3:
        touch_swap_xy = true;
        touch_map_x1 = touch_max_x;
        touch_map_x2 = 0;
        touch_map_y1 = 0;
        touch_map_y2 = touch_max_y;
        break;
      case 2:
        touch_swap_xy = false;
        touch_map_x1 = touch_max_x;
        touch_map_x2 = 0;
        touch_map_y1 = touch_max_y;
        touch_map_y2 = 0;
        break;
      case 1:
        touch_swap_xy = true;
        touch_map_x1 = 0;
        touch_map_x2 = touch_max_x;
        touch_map_y1 = touch_max_y;
        touch_map_y2 = 0;
        break;
      default:  // case 0:
        touch_swap_xy = false;
        touch_map_x1 = 0;
        touch_map_x2 = touch_max_x;
        touch_map_y1 = 0;
        touch_map_y2 = touch_max_y;
        break;
    }
  }

  ts.begin();
  ts.setRotation(TOUCH_XPT2046_ROTATION);
}

bool touch_has_signal() {
  return ts.tirqTouched();
}

void translate_touch_raw() {
  if (touch_swap_xy) {
    touch_last_x = map(touch_raw_y, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_x, touch_map_y1, touch_map_y2, 0, touch_max_y);
  } else {
    touch_last_x = map(touch_raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
}

bool touch_touched() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    touch_raw_x = p.x;
    touch_raw_y = p.y;
    int max_z = p.z;
    int count = 0;
    while ((ts.touched()) && (count < TOUCH_XPT2046_SAMPLES)) {
      count++;

      TS_Point p = ts.getPoint();
      if (p.z > max_z) {
        touch_raw_x = p.x;
        touch_raw_y = p.y;
        max_z = p.z;
      }
    }
    translate_touch_raw();
    return true;
  }
  return false;
}

bool touch_released() {
  return true;
}

void setup(void) {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(500000);
  delay(3000);
  Serial.println("Arduino_GFX Touch Calibration example");

  Serial.println("Init display");
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

#ifdef DISPLAY_BACKLIGHT_PIN
  pinMode(DISPLAY_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(DISPLAY_BACKLIGHT_PIN, HIGH);
#endif

  // Init touch device
  w = gfx->width();
  h = gfx->height();
  touch_init(w, h, gfx->getRotation());

  // Top left
  point_x[0] = w / 8;
  point_y[0] = h / 8;
  // Top right
  point_x[1] = point_x[0] * 7;
  point_y[1] = point_y[0];
  // Bottom left
  point_x[2] = point_x[0];
  point_y[2] = point_y[0] * 7;
  // Bottom right
  point_x[3] = point_x[1];
  point_y[3] = point_y[2];

  gfx->setCursor(0, 0);
  gfx->setTextColor(RGB565_RED);
  gfx->setTextSize(2);
  gfx->println("Touch Calibration");
}

void loop() {
  if (current_point != next_point) {
    current_point = next_point;

    gfx->drawLine(
        point_x[current_point] - 5,
        point_y[current_point] - 5,
        point_x[current_point] + 5,
        point_y[current_point] + 5,
        RGB565_RED);
    gfx->drawLine(
        point_x[current_point] + 5,
        point_y[current_point] - 5,
        point_x[current_point] - 5,
        point_y[current_point] + 5,
        RGB565_RED);
  }

  if (touch_touched()) {
    int32_t total_x = touch_raw_x, total_y = touch_raw_y;
    int count = 1;
    while (touch_touched()) {
      total_x += touch_raw_x;
      total_y += touch_raw_y;
      count++;
    }
    touched_x[current_point] = total_x / count;
    touched_y[current_point] = total_y / count;

    if (current_point == 3) {
      Serial.print("touched_x[0]: ");
      Serial.print(touched_x[0]);
      Serial.print(", touched_y[0]: ");
      Serial.println(touched_y[0]);
      Serial.print("touched_x[1]: ");
      Serial.print(touched_x[1]);
      Serial.print(", touched_y[1]: ");
      Serial.println(touched_y[1]);
      Serial.print("touched_x[2]: ");
      Serial.print(touched_x[2]);
      Serial.print(", touched_y[2]: ");
      Serial.println(touched_y[2]);
      Serial.print("touched_x[3]: ");
      Serial.print(touched_x[3]);
      Serial.print(", touched_y[3]: ");
      Serial.println(touched_y[3]);
      uint16_t delta_x = (touched_x[0] > touched_x[1]) ? (touched_x[0] - touched_x[1]) : (touched_x[1] - touched_x[0]);
      uint16_t delta_y = (touched_y[0] > touched_y[1]) ? (touched_y[0] - touched_y[1]) : (touched_y[1] - touched_y[0]);

      if (delta_x > delta_y) {
        touch_swap_xy = false;
        touch_map_x1 = (touched_x[0] + touched_x[2]) / 2;
        touch_map_x2 = (touched_x[1] + touched_x[3]) / 2;
        touch_map_y1 = (touched_y[0] + touched_y[1]) / 2;
        touch_map_y2 = (touched_y[2] + touched_y[3]) / 2;
      } else {
        touch_swap_xy = true;
        touch_map_x1 = (touched_y[0] + touched_y[2]) / 2;
        touch_map_x2 = (touched_y[1] + touched_y[3]) / 2;
        touch_map_y1 = (touched_x[0] + touched_x[1]) / 2;
        touch_map_y2 = (touched_x[2] + touched_x[3]) / 2;
      }

      if (touch_map_x1 > touch_map_x2) {
        delta_x = (touch_map_x1 - touch_map_x2) / 6;
        touch_map_x1 += delta_x;
        touch_map_x2 -= delta_x;
      } else {
        delta_x = (touch_map_x2 - touch_map_x1) / 6;
        touch_map_x1 -= delta_x;
        touch_map_x2 += delta_x;
      }

      if (touch_map_y1 > touch_map_y2) {
        delta_y = (touch_map_y1 - touch_map_y2) / 6;
        touch_map_y1 += delta_y;
        touch_map_y2 -= delta_y;
      } else {
        delta_y = (touch_map_y2 - touch_map_y1) / 6;
        touch_map_y1 -= delta_y;
        touch_map_y2 += delta_y;
      }

      Serial.print("bool touch_swap_xy = ");
      Serial.println(touch_swap_xy ? "true" : "false");
      Serial.print("int16_t touch_map_x1 = ");
      Serial.println(touch_map_x1);
      Serial.print("int16_t touch_map_x2 = ");
      Serial.println(touch_map_x2);
      Serial.print("int16_t touch_map_y1 = ");
      Serial.println(touch_map_y1);
      Serial.print("int16_t touch_map_y2 = ");
      Serial.println(touch_map_y2);

      gfx->setCursor(0, point_y[0] + 10);
      gfx->setTextColor(RGB565_WHITE);
      gfx->setTextSize(1);
      gfx->print("bool touch_swap_xy = ");
      gfx->println(touch_swap_xy ? "true" : "false");
      gfx->print("int16_t touch_map_x1 = ");
      gfx->println(touch_map_x1);
      gfx->print("int16_t touch_map_x2 = ");
      gfx->println(touch_map_x2);
      gfx->print("int16_t touch_map_y1 = ");
      gfx->println(touch_map_y1);
      gfx->print("int16_t touch_map_y2 = ");
      gfx->println(touch_map_y2);

      // wait next touch to continue
      while (!touch_touched());

      gfx->fillScreen(RGB565_BLACK);
      gfx->setCursor(0, 0);
      gfx->setTextColor(RGB565_RED);
      gfx->setTextSize(2);
      gfx->println("Touch Calibration");
    }

    next_point = (current_point == 3) ? 0 : (current_point + 1);
  }

  delay(100);
}
