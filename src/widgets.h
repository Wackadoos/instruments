#pragma once

#include <Arduino.h>

#include "hardware.h"
#include "modules/display.h"
#include "settings.h"
#include "state.h"

//* Definitions in src/widgets.cpp (single TU so no inline-variable init guards)

//* Centre
extern Widget<char[12]> BATT_WIDGET;
extern Widget<float> SPEED_WIDGET;
extern SetpointWidget<float> POWER_WIDGET;

//* Left column (race page)
extern SetpointWidget<float> AVG_WATTS_WIDGET;
extern SetpointWidget<float> VOLTAGE_WIDGET;

//* Bottom Row
extern StaticWidget AMBIENT_TEMP_TEXT_WIDGET;
extern SetpointWidget<float> TEMP1_WIDGET;
extern SetpointWidget<float> TEMP2_WIDGET;
extern StaticWidget ESC_TEMP_TEXT_WIDGET;
extern SetpointWidget<float> ESC_TEMP_WIDGET;
extern StaticWidget MOTOR_TEMP_TEXT_WIDGET;
extern SetpointWidget<float> MOTOR_TEMP1_WIDGET;
extern SetpointWidget<float> MOTOR_TEMP2_WIDGET;

//* Top Row
extern Widget<char[8]> TIME_WIDGET;
extern SetpointWidget<uint8_t> GPS_WIDGET;

extern Button SETTINGS_BUTTON;
struct RaceButton {
  // Label/colour reflect the current AppMode; updated via State::enterMode()
  static Button button;
};

extern Button HOME_PAGE_BUTTON;
extern Button DEBUG_BUTTON;

extern DebugText DEBUG_PAGE_TEXT;
extern Button DEBUG_PREV_BUTTON;
extern Button DEBUG_NEXT_BUTTON;
extern Button DEBUG_HOME_BUTTON;

extern Page RACE_PAGE;
extern Page SETTINGS_PAGE;
extern Page DEBUG_PAGE;
