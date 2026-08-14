#pragma once

#include <Arduino.h>

#include "hardware.h"
#include "modules/display.h"
#include "state.h"

//* Centre
static inline auto BATT_WIDGET = Widget(&SensorState::battery_stats, {240, 40, TextAlign::CENTER, RGB565_AZURE, 3});
static inline auto SPEED_WIDGET = Widget(&SensorState::gps_speed, {240, 70, TextAlign::CENTER, RGB565_ORANGE, 14, 0, {"Km/h", 2, 0, 12}});
static inline auto POWER_WIDGET = SetpointWidget(&SensorState::battery_power, {240, 184, TextAlign::CENTER, RGB565_AQUA, 9, 0, {"W", 2, 2, 7}}, &POWER_WARNING_SETPOINT, &POWER_ERROR_SETPOINT, RGB565_AQUA, RGB565_YELLOW, RGB565_RED);

//* Bottom Row
static inline auto AMBIENT_TEMP_TEXT_WIDGET = StaticWidget("Ambient", {0, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto TEMP1_WIDGET = SetpointWidget(&SensorState::ambient_temperature, {0, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto TEMP2_WIDGET = SetpointWidget(&SensorState::ambient_temperature_2, {60, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto ESC_TEMP_TEXT_WIDGET = StaticWidget("ESC", {120, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto ESC_TEMP_WIDGET = SetpointWidget(&SensorState::esc_temp, {120, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &ESC_TEMP_WARNING_SETPOINT, &ESC_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto MOTOR_TEMP_TEXT_WIDGET = StaticWidget("Motor", {180, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto MOTOR_TEMP1_WIDGET = SetpointWidget(&SensorState::temp_motor_1, {180, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto MOTOR_TEMP2_WIDGET = SetpointWidget(&SensorState::temp_motor_2, {240, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);

//* Top Row
static inline auto TIME_WIDGET = Widget(&SensorState::currentTime, {0, 0, TextAlign::LEFT, RGB565_DARKORANGE, 3});
static inline auto LAP_TIME_WIDGET = Widget(&SensorState::splitDiff, {0, 30, TextAlign::LEFT, RGB565_MEDIUMSPRINGGREEN, 2});
static inline auto GPS_WIDGET = SetpointWidget(&SensorState::fix_satellites, {480, 0, TextAlign::RIGHT, RGB565_BLUE, 3, 0, {&SensorState::sat_string, 3, 0, 0}}, &GPS_WARNING_SETPOINT, &GPS_ERROR_SETPOINT, RGB565_BLUE, RGB565_YELLOW, RGB565_RED, true); // TODO

static inline void onSettingsPagePress();
static inline void onRacePagePress();
static inline void onRacePress();

static inline auto SETTINGS_BUTTON = Button("SETTINGS", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0, &onSettingsPagePress);
static inline auto RACE_BUTTON = Button("START\nRACE", {430, 210, TextAlign::CENTER, RGB565_BLUEVIOLET, 2}, 100, 70, 12, 4, &onRacePress);

static inline WidgetBase* RACE_PAGE_WIDGETS[] = {&BATT_WIDGET,
                                                 &SPEED_WIDGET,
                                                 &POWER_WIDGET,
                                                 &AMBIENT_TEMP_TEXT_WIDGET,
                                                 &TEMP1_WIDGET,
                                                 &TEMP2_WIDGET,
                                                 &ESC_TEMP_WIDGET,
                                                 &ESC_TEMP_TEXT_WIDGET,
                                                 &MOTOR_TEMP_TEXT_WIDGET,
                                                 &MOTOR_TEMP1_WIDGET,
                                                 &MOTOR_TEMP2_WIDGET,
                                                 &TIME_WIDGET,
                                                 &LAP_TIME_WIDGET,
                                                 &GPS_WIDGET,
                                                 &SETTINGS_BUTTON,
                                                 &RACE_BUTTON};
static inline Page RACE_PAGE = Page(RACE_PAGE_WIDGETS);

static inline auto HOME_PAGE_BUTTON = Button("HOME", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0,
                                             &onRacePagePress);

static inline WidgetBase* SETTINGS_PAGE_WIDGETS[] = {&HOME_PAGE_BUTTON};
static inline Page SETTINGS_PAGE = Page(SETTINGS_PAGE_WIDGETS);

//* Buttons
static inline void onSettingsPagePress() {
  Display::changePage(&SETTINGS_PAGE);
}

static inline void onRacePagePress() {
  Display::changePage(&RACE_PAGE);
}

static inline void onRacePress() {
}
