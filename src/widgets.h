#pragma once

#include <Arduino.h>

#include "modules/display.h"
#include "state.h"

//* Centre
static inline auto BATT_WIDGET = Widget(&SensorState::battery_stats, {240, 40, TextAlign::CENTER, RGB565_AZURE, 3});
static inline auto SPEED_WIDGET = Widget(&SensorState::temp_motor_back, {240, 70, TextAlign::CENTER, RGB565_ORANGE, 14, 0, {"Km/h", 2, 0, 12}});
static inline auto POWER_WIDGET = Widget(&SensorState::battery_power, {240, 180, TextAlign::CENTER, RGB565_GOLDENROD, 9, 0, {"W", 2, 2, 7}});

//* Bottom Row
static inline auto AMBIENT_TEMP_TEXT_WIDGET = StaticWidget("Ambient", {0, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto TEMP1_WIDGET = Widget(&SensorState::ambient_temperature, {0, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3});
static inline auto TEMP2_WIDGET = Widget(&SensorState::ambient_temperature_2, {60, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3});
static inline auto ESC_TEMP_TEXT_WIDGET = StaticWidget("ESC", {120, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto ESC_TEMP_WIDGET = Widget(&SensorState::esc_temp, {120, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3});
static inline auto MOTOR_TEMP_TEXT_WIDGET = StaticWidget("Motor", {180, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto MOTOR_TEMP1_WIDGET = Widget(&SensorState::temp_motor_front, {180, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3});
static inline auto MOTOR_TEMP2_WIDGET = Widget(&SensorState::temp_motor_back, {240, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3});

//* Top Row
static inline auto TIME_WIDGET = Widget(&SensorState::currentTime, {0, 0, TextAlign::LEFT, RGB565_DARKORANGE, 3});
static inline auto LAP_TIME_WIDGET = Widget(&SensorState::splitDiff, {0, 30, TextAlign::LEFT, RGB565_GREEN, 2});
static inline auto GPS_WIDGET = Widget(&SensorState::visible_satellites, {480, 0, TextAlign::RIGHT, RGB565_BLUE, 3});

//* Buttons
const inline char SETTINGS_TEXT[] = "SETTINGS";
const inline char* SETTINGS_TEXT_PTR = SETTINGS_TEXT;
static inline auto SETTINGS_BUTTON = Button(&SETTINGS_TEXT_PTR, {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12);
const inline char RACE_TEXT[] = "";
const inline char* RACE_TEXT_PTR = RACE_TEXT;
static inline auto RACE_BUTTON = Button(&RACE_TEXT_PTR, {425, 215, TextAlign::CENTER, RGB565_BLUEVIOLET, 2}, 100, 80, 12);
static inline auto RACE_BUTTON2 = StaticWidget("START", {425, 200, TextAlign::CENTER, RGB565_BLUEVIOLET, 2});
static inline auto RACE_BUTTON3 = StaticWidget("RACE", {425, 220, TextAlign::CENTER, RGB565_BLUEVIOLET, 2});

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
                                                       &RACE_BUTTON,
                                                       &RACE_BUTTON2,
                                                       &RACE_BUTTON3};
static inline Page RACE_PAGE = Page(RACE_PAGE_WIDGETS);
