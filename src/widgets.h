#pragma once

#include <Arduino.h>

#include "hardware.h"
#include "modules/display.h"
#include "settings.h"
#include "state.h"

//* Centre
static inline auto BATT_WIDGET = Widget(&State::battery_stats, {240, 40, TextAlign::CENTER, RGB565_AZURE, 3});
static inline auto SPEED_WIDGET = Widget(&State::gps_speed, {240, 70, TextAlign::CENTER, RGB565_ORANGE, 14, 0, {"Km/h", 2, 0, 12}});
static inline auto POWER_WIDGET = SetpointWidget(&State::battery_power, {240, 184, TextAlign::CENTER, RGB565_AQUA, 9, 0, {"W", 2, 2, 7}}, &POWER_WARNING_SETPOINT, &POWER_ERROR_SETPOINT, RGB565_AQUA, RGB565_YELLOW, RGB565_RED);

//* Bottom Row
static inline auto AMBIENT_TEMP_TEXT_WIDGET = StaticWidget("Ambient", {0, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto TEMP1_WIDGET = SetpointWidget(&State::ambient_temperature, {0, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto TEMP2_WIDGET = SetpointWidget(&State::ambient_temperature_2, {60, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto ESC_TEMP_TEXT_WIDGET = StaticWidget("ESC", {120, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto ESC_TEMP_WIDGET = SetpointWidget(&State::esc_temp, {120, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &ESC_TEMP_WARNING_SETPOINT, &ESC_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto MOTOR_TEMP_TEXT_WIDGET = StaticWidget("Motor", {180, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto MOTOR_TEMP1_WIDGET = SetpointWidget(&State::temp_motor_1, {180, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
static inline auto MOTOR_TEMP2_WIDGET = SetpointWidget(&State::temp_motor_2, {240, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);

//* Top Row
static inline auto TIME_WIDGET = Widget(&State::currentTime, {0, 0, TextAlign::LEFT, RGB565_DARKORANGE, 3});
// static inline auto LAP_TIME_WIDGET = Widget(&State::splitDiff, {0, 30, TextAlign::LEFT, RGB565_MEDIUMSPRINGGREEN, 2});
static inline auto GPS_WIDGET = SetpointWidget(&State::fix_satellites, {480, 0, TextAlign::RIGHT, RGB565_BLUE, 3, 0, {&State::sat_string, 3, 0, 0}}, &GPS_WARNING_SETPOINT, &GPS_ERROR_SETPOINT, RGB565_BLUE, RGB565_YELLOW, RGB565_RED, true);

static inline void onSettingsPagePress();
static inline void onRacePagePress();
static inline void onRacePress();
static inline void onWheelInc();
static inline void onWheelDec();
static inline void onPulsesInc();
static inline void onPulsesDec();
static inline void onTimezoneInc();
static inline void onTimezoneDec();

static inline auto SETTINGS_BUTTON = Button("SETTINGS", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0, &onSettingsPagePress);
struct RaceButton {
  // Label/colour reflect the current AppMode; updated via State::enterMode()
  static inline Button button = Button("START\nRACE", {430, 210, TextAlign::CENTER, RGB565_BLUEVIOLET, 2}, 100, 70, 12, 4, &onRacePress);
};

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
                                                 //  &LAP_TIME_WIDGET,
                                                 &GPS_WIDGET,
                                                 &SETTINGS_BUTTON,
                                                 &RaceButton::button};
static inline Page RACE_PAGE = Page(RACE_PAGE_WIDGETS);

static inline auto HOME_PAGE_BUTTON = Button("HOME", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0,
                                             &onRacePagePress);

//* Settings Page: Wheel circumference (mm)
static inline auto WHEEL_LABEL_WIDGET = StaticWidget("WHEEL", {0, 55, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto WHEEL_VALUE_WIDGET = Widget(&SETTINGS::settings.speed_sensor_wheel_circumference, {130, 55, TextAlign::LEFT, RGB565_WHITE, 3, 0, {"mm", 2, 2, 4}});
static inline auto WHEEL_MINUS_BUTTON = Button("-", {310, 45, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onWheelDec);
static inline auto WHEEL_PLUS_BUTTON = Button("+", {370, 45, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onWheelInc);

//* Settings Page: Pulses per revolution
static inline auto PULSES_LABEL_WIDGET = StaticWidget("PULSES", {0, 125, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto PULSES_VALUE_WIDGET = Widget(&SETTINGS::settings.speed_sensor_pulses_per_revolution, {130, 125, TextAlign::LEFT, RGB565_WHITE, 3});
static inline auto PULSES_MINUS_BUTTON = Button("-", {310, 115, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onPulsesDec);
static inline auto PULSES_PLUS_BUTTON = Button("+", {370, 115, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onPulsesInc);

//* Settings Page: Timezone offset (15-min units, displayed as ±hh:mm)
static inline auto TIMEZONE_LABEL_WIDGET = StaticWidget("TIMEZONE", {0, 195, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
static inline auto TIMEZONE_VALUE_WIDGET = Widget(&State::timezone_string, {130, 195, TextAlign::LEFT, RGB565_WHITE, 3});
static inline auto TIMEZONE_MINUS_BUTTON = Button("-", {310, 185, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onTimezoneDec);
static inline auto TIMEZONE_PLUS_BUTTON = Button("+", {370, 185, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onTimezoneInc);

static inline WidgetBase* SETTINGS_PAGE_WIDGETS[] = {&WHEEL_LABEL_WIDGET,
                                                     &WHEEL_VALUE_WIDGET,
                                                     &WHEEL_MINUS_BUTTON,
                                                     &WHEEL_PLUS_BUTTON,
                                                     &PULSES_LABEL_WIDGET,
                                                     &PULSES_VALUE_WIDGET,
                                                     &PULSES_MINUS_BUTTON,
                                                     &PULSES_PLUS_BUTTON,
                                                     &TIMEZONE_LABEL_WIDGET,
                                                     &TIMEZONE_VALUE_WIDGET,
                                                     &TIMEZONE_MINUS_BUTTON,
                                                     &TIMEZONE_PLUS_BUTTON,
                                                     &HOME_PAGE_BUTTON};
static inline Page SETTINGS_PAGE = Page(SETTINGS_PAGE_WIDGETS);

//* Buttons
static inline void onSettingsPagePress() {
  Display::changePage(&SETTINGS_PAGE);
}

static inline void onRacePagePress() {
  Display::changePage(&RACE_PAGE);
}

static inline void onRacePress() {
  State::enterMode(State::currentMode == AppMode::RACE ? AppMode::IDLE : AppMode::RACE);
}

//* Settings edit helpers (persist via pushSetting -> EEPROM + apply)
static inline void changeWheel(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.speed_sensor_wheel_circumference = constrain((int16_t)s.speed_sensor_wheel_circumference + delta,
                                                 (int16_t)SETTINGS::WHEEL_CIRCUMFERENCE_MIN,
                                                 (int16_t)SETTINGS::WHEEL_CIRCUMFERENCE_MAX);
  SETTINGS::pushSetting(s);
}
static inline void onWheelInc() { changeWheel(1); }
static inline void onWheelDec() { changeWheel(-1); }

static inline void changePulses(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.speed_sensor_pulses_per_revolution = constrain((int16_t)s.speed_sensor_pulses_per_revolution + delta,
                                                   (int16_t)SETTINGS::PULSES_PER_REV_MIN,
                                                   (int16_t)SETTINGS::PULSES_PER_REV_MAX);
  SETTINGS::pushSetting(s);
}
static inline void onPulsesInc() { changePulses(1); }
static inline void onPulsesDec() { changePulses(-1); }

static inline void changeTimezone(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.timezone_offset = constrain(s.timezone_offset + delta, SETTINGS::TIMEZONE_OFFSET_MIN, SETTINGS::TIMEZONE_OFFSET_MAX);
  SETTINGS::pushSetting(s);
}
static inline void onTimezoneInc() { changeTimezone(1); }
static inline void onTimezoneDec() { changeTimezone(-1); }
