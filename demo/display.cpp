//! ILI9488 based display module + xpt2046 touchscreen
#include <Arduino.h>

void setup() {
}

void loop() {
}

// TODO Frame buffers?
// TODO  Display class should use widgets with local state of last update (to update only if different - after decimal precision cutoff etc) and all widgets implement an erase function that redraws value in black to erase from screen using just those pixels. Will this cause problems with antialiasing on text?
// TODO Widgets get enabled/disabled per page. Page class calls initial draw and erase on exit. Widgets keep updating during when modified
// TODO Render one widget at a time and return. Avoid long contiguous render blocks as this will block sensor data acquisition. Have a selector that iterates through widgets yielding after one renders, to avoid prioritization and starving later widgets from rendering even under heavy contention.
