#pragma once

#include <Arduino.h>

#include "modules/display.h"
#include "state.h"

//* Centre
static inline auto SPEED_WIDGET = Widget(&SensorState::temp_motor_back, 160, 70, RGB565_ORANGE, 14);
static inline auto POWER_WIDGET = Widget(&SensorState::battery_power, 160, 180, RGB565_GOLDENROD, 9);

//* Bottom Row
const inline char MOTOR_TEMP_TEXT[] = "Motor";
const inline char* MOTOR_TEMP_TEXT_PTR = MOTOR_TEMP_TEXT;
static inline auto MOTOR_TEMP_TEXT_WIDGET = StaticWidget(&MOTOR_TEMP_TEXT_PTR, 0, 275, RGB565_WHITESMOKE, 2);
static inline auto MOTOR_TEMP1_WIDGET = Widget(&SensorState::temp_motor_front, 0, 295, RGB565_LIGHTGREEN, 3);
static inline auto MOTOR_TEMP2_WIDGET = Widget(&SensorState::temp_motor_back, 60, 295, RGB565_LIGHTGREEN, 3);
const inline char ESC_TEMP_TEXT[] = "ESC";
const inline char* ESC_TEMP_TEXT_PTR = ESC_TEMP_TEXT;
static inline auto ESC_TEMP_TEXT_WIDGET = StaticWidget(&ESC_TEMP_TEXT_PTR, 120, 275, RGB565_WHITESMOKE, 2);
static inline auto ESC_TEMP_WIDGET = Widget(&SensorState::esc_temp, 120, 295, RGB565_LIGHTGREEN, 3);
const inline char AMBIENT_TEMP_TEXT[] = "Ambient";
const inline char* AMBIENT_TEMP_TEXT_PTR = AMBIENT_TEMP_TEXT;
static inline auto AMBIENT_TEMP_TEXT_WIDGET = StaticWidget(&AMBIENT_TEMP_TEXT_PTR, 180, 275, RGB565_WHITESMOKE, 2);
static inline auto TEMP1_WIDGET = Widget(&SensorState::ambient_temperature, 180, 295, RGB565_LIGHTGREEN, 3);
static inline auto TEMP2_WIDGET = Widget(&SensorState::ambient_temperature_2, 240, 295, RGB565_LIGHTGREEN, 3);

//* Top Row
static inline auto ALTITUDE_WIDGET = Widget(&SensorState::uncalibrated_altitude, 10, 10, RGB565_BLUE, 3);
static inline auto GPS_WIDGET = Widget(&SensorState::visible_satellites, 440, 0, RGB565_BLUE, 3);

const static inline WidgetBase* RACE_PAGE_WIDGETS[] = {&SPEED_WIDGET, &POWER_WIDGET, &MOTOR_TEMP_TEXT_WIDGET, &ESC_TEMP_TEXT_WIDGET, &AMBIENT_TEMP_TEXT_WIDGET, &MOTOR_TEMP1_WIDGET, &MOTOR_TEMP2_WIDGET, &ESC_TEMP_WIDGET, &TEMP1_WIDGET, &TEMP2_WIDGET, &ALTITUDE_WIDGET, &GPS_WIDGET};
static inline Page RACE_PAGE = Page(RACE_PAGE_WIDGETS);
