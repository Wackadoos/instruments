//! ILI9488 based display module + xpt2046 touchscreen
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define GFX_BL 4    // backlight
#define TFT_CS 53   // chip select
#define TFT_DC 47   // data/command
#define TFT_RST 43  // reset

// Uses default HW SPI (SCK/MOSI/MISO pins for your board)
Arduino_DataBus* bus = new Arduino_HWSPI(TFT_DC,TFT_CS);
Arduino_GFX* gfx = new Arduino_ILI9488_18bit(bus, TFT_RST, 1 /* rotation */, false /* IPS */);

void setup(void) {
  Serial.begin(115200);
  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);
  pinMode(49, OUTPUT);
  digitalWrite(49, HIGH);
  delay(3000);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin(8000000)) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_RED);
  gfx->println("Hello World!");
  Serial.println("gfx->initialised!");

  delay(2000);  // 5 seconds
}

void loop() {
  gfx->setCursor(random(gfx->width()), random(gfx->height()));
  gfx->setTextColor(0xffffff, random(0xffff));
  gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
  gfx->println("Hello World!");
  Serial.println("Hello World!");

  delay(100);  // 1 second
}

// TODO Frame buffers?
// TODO  Display class should use widgets with local state of last update (to update only if different - after decimal precision cutoff etc) and all widgets implement an erase function that redraws value in black to erase from screen using just those pixels. Will this cause problems with antialiasing on text?
// TODO Widgets get enabled/disabled per page. Page class calls initial draw and erase on exit. Widgets keep updating during when modified
// TODO Render one widget at a time and return. Avoid long contiguous render blocks as this will block sensor data acquisition. Have a selector that iterates through widgets yielding after one renders, to avoid prioritization and starving later widgets from rendering even under heavy contention.
