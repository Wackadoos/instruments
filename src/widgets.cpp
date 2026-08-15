#include "widgets.h"

//* Centre
Widget<char[12]> BATT_WIDGET(&State::battery_stats, {240, 40, TextAlign::CENTER, RGB565_AZURE, 3});
Widget<float> SPEED_WIDGET(&State::gps_speed, {240, 70, TextAlign::CENTER, RGB565_ORANGE, 14, 0, {"Km/h", 2, 0, 12}});
SetpointWidget<float> POWER_WIDGET(&State::battery_power, {240, 184, TextAlign::CENTER, RGB565_AQUA, 9, 0, {"W", 2, 2, 7}}, &POWER_WARNING_SETPOINT, &POWER_ERROR_SETPOINT, RGB565_AQUA, RGB565_YELLOW, RGB565_RED);

//* Bottom Row
StaticWidget AMBIENT_TEMP_TEXT_WIDGET("Ambient", {0, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
SetpointWidget<float> TEMP1_WIDGET(&State::ambient_temperature, {0, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
SetpointWidget<float> TEMP2_WIDGET(&State::ambient_temperature_2, {60, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &TEMP1_WARNING_SETPOINT, &TEMP1_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
StaticWidget ESC_TEMP_TEXT_WIDGET("ESC", {120, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
SetpointWidget<float> ESC_TEMP_WIDGET(&State::esc_temp, {120, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &ESC_TEMP_WARNING_SETPOINT, &ESC_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
StaticWidget MOTOR_TEMP_TEXT_WIDGET("Motor", {180, 275, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
SetpointWidget<float> MOTOR_TEMP1_WIDGET(&State::temp_motor_1, {180, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);
SetpointWidget<float> MOTOR_TEMP2_WIDGET(&State::temp_motor_2, {240, 295, TextAlign::LEFT, RGB565_LIGHTGREEN, 3}, &MOTOR_TEMP_WARNING_SETPOINT, &MOTOR_TEMP_ERROR_SETPOINT, RGB565_LIGHTGREEN, RGB565_YELLOW, RGB565_RED);

//* Top Row
Widget<char[8]> TIME_WIDGET(&State::currentTime, {0, 0, TextAlign::LEFT, RGB565_DARKORANGE, 3});
// Widget<char[8]> LAP_TIME_WIDGET(&State::splitDiff, {0, 30, TextAlign::LEFT, RGB565_MEDIUMSPRINGGREEN, 2});
SetpointWidget<uint8_t> GPS_WIDGET(&State::fix_satellites, {480, 0, TextAlign::RIGHT, RGB565_BLUE, 3, 0, {State::sat_string, 3, 0, 0}}, &GPS_WARNING_SETPOINT, &GPS_ERROR_SETPOINT, RGB565_BLUE, RGB565_YELLOW, RGB565_RED, true);

//* Buttons
static void onSettingsPagePress() {
  Display::changePage(&SETTINGS_PAGE);
}

static void onRacePagePress() {
  Display::changePage(&RACE_PAGE);
}

static void onRacePress() {
  State::enterMode(State::currentMode == AppMode::RACE ? AppMode::IDLE : AppMode::RACE);
}

//* Settings edit helpers (persist via pushSetting -> EEPROM + apply)
static void changeWheel(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.speed_sensor_wheel_circumference = constrain((int16_t)s.speed_sensor_wheel_circumference + delta,
                                                 (int16_t)SETTINGS::WHEEL_CIRCUMFERENCE_MIN,
                                                 (int16_t)SETTINGS::WHEEL_CIRCUMFERENCE_MAX);
  SETTINGS::pushSetting(s);
}
static void onWheelInc() { changeWheel(1); }
static void onWheelDec() { changeWheel(-1); }

static void changePulses(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.speed_sensor_pulses_per_revolution = constrain((int16_t)s.speed_sensor_pulses_per_revolution + delta,
                                                   (int16_t)SETTINGS::PULSES_PER_REV_MIN,
                                                   (int16_t)SETTINGS::PULSES_PER_REV_MAX);
  SETTINGS::pushSetting(s);
}
static void onPulsesInc() { changePulses(1); }
static void onPulsesDec() { changePulses(-1); }

static void changeTimezone(int16_t delta) {
  auto s = SETTINGS::getSettings();
  s.timezone_offset = constrain(s.timezone_offset + delta, SETTINGS::TIMEZONE_OFFSET_MIN, SETTINGS::TIMEZONE_OFFSET_MAX);
  SETTINGS::pushSetting(s);
}
static void onTimezoneInc() { changeTimezone(1); }
static void onTimezoneDec() { changeTimezone(-1); }

Button SETTINGS_BUTTON("SETTINGS", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0, &onSettingsPagePress);
Button RaceButton::button = Button("START\nRACE", {430, 210, TextAlign::CENTER, RGB565_BLUEVIOLET, 2}, 100, 70, 12, 4, &onRacePress);
Button HOME_PAGE_BUTTON("HOME", {410, 285, TextAlign::CENTER, RGB565_BLUE, 2}, 140, 50, 12, 0, &onRacePagePress);

//* Settings Page: Wheel circumference (mm)
StaticWidget WHEEL_LABEL_WIDGET("WHEEL", {0, 55, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
Widget<uint16_t> WHEEL_VALUE_WIDGET(&SETTINGS::settings.speed_sensor_wheel_circumference, {130, 55, TextAlign::LEFT, RGB565_WHITE, 3, 0, {"mm", 2, 2, 4}});
Button WHEEL_MINUS_BUTTON("-", {310, 45, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onWheelDec);
Button WHEEL_PLUS_BUTTON("+", {370, 45, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onWheelInc);

//* Settings Page: Pulses per revolution
StaticWidget PULSES_LABEL_WIDGET("PULSES", {0, 125, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
Widget<uint8_t> PULSES_VALUE_WIDGET(&SETTINGS::settings.speed_sensor_pulses_per_revolution, {130, 125, TextAlign::LEFT, RGB565_WHITE, 3});
Button PULSES_MINUS_BUTTON("-", {310, 115, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onPulsesDec);
Button PULSES_PLUS_BUTTON("+", {370, 115, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onPulsesInc);

//* Settings Page: Timezone offset (15-min units, displayed as ±hh:mm)
StaticWidget TIMEZONE_LABEL_WIDGET("TIMEZONE", {0, 195, TextAlign::LEFT, RGB565_WHITESMOKE, 2});
Widget<char[8]> TIMEZONE_VALUE_WIDGET(&State::timezone_string, {130, 195, TextAlign::LEFT, RGB565_WHITE, 3});
Button TIMEZONE_MINUS_BUTTON("-", {310, 185, TextAlign::CENTER, RGB565_RED, 3}, 40, 40, 8, 0, &onTimezoneDec);
Button TIMEZONE_PLUS_BUTTON("+", {370, 185, TextAlign::CENTER, RGB565_LIGHTGREEN, 3}, 40, 40, 8, 0, &onTimezoneInc);

//* Pages
static WidgetBase* RACE_PAGE_WIDGETS[] = {&BATT_WIDGET,
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
                                          &GPS_WIDGET,
                                          &SETTINGS_BUTTON,
                                          &RaceButton::button};
Page RACE_PAGE = Page(RACE_PAGE_WIDGETS);

static WidgetBase* SETTINGS_PAGE_WIDGETS[] = {&WHEEL_LABEL_WIDGET,
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
Page SETTINGS_PAGE = Page(SETTINGS_PAGE_WIDGETS);
