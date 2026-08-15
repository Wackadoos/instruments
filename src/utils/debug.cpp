#include "utils/debug.h"

#include "state.h"
#include "utils/errors.h"
#include "utils/metrics.h"

#define DEBUG_STATE_ITEMS 32   // Items 0-31
#define DEBUG_TIMING_START 32  // Items 32-41 = the registered IntervalMetrics

static void append(char* buf, size_t len, const char* text) {
  size_t used = strlen(buf);
  if (used >= len - 1) {
    return;
  }
  strncat(buf + used, text, len - used - 1);
  buf[len - 1] = '\0';
}

static void appendFlash(char* buf, size_t len, const __FlashStringHelper* text) {
  size_t used = strlen(buf);
  if (used >= len - 1) {
    return;
  }
  strncpy_P(buf + used, reinterpret_cast<const char*>(text), len - used - 1);
  buf[len - 1] = '\0';
}

static void appendFloat(char* buf, size_t len, float value) {
  char tmp[12];
  dtostrf(value, 0, 2, tmp);
  append(buf, len, tmp);
}

static void appendInt(char* buf, size_t len, long value) {
  char tmp[12];
  ltoa(value, tmp, 10);
  append(buf, len, tmp);
}

static void append2(char* buf, size_t len, uint8_t value) {
  char tmp[3] = {(char)('0' + value / 10), (char)('0' + value % 10), '\0'};
  append(buf, len, tmp);
}

uint8_t debugPageCount() {
  return 3;
}

static bool debugStateItem(uint8_t idx, DebugLine& out, char* v, size_t len) {
  v[0] = '\0';
  switch (idx) {
    case 0:  // MODE
      out.label = F("MODE");
      switch (State::currentMode) {
        case AppMode::IDLE:
          append(v, len, "IDLE");
          break;
        case AppMode::RACE:
          append(v, len, "RACE");
          break;
        case AppMode::DEBUG:
          append(v, len, "DEBUG");
          break;
      }
      break;
    case 1:  // TIME
      out.label = F("TIME");
      append(v, len, State::currentTime);
      break;
    case 2:
      out.label = F("MOTOR1");
      appendFloat(v, len, State::temp_motor_1);
      break;
    case 3:
      out.label = F("MOTOR2");
      appendFloat(v, len, State::temp_motor_2);
      break;
    case 4:
      out.label = F("BMP AMB");
      appendFloat(v, len, State::ambient_temperature);
      break;
    case 5:
      out.label = F("AHT AMB");
      appendFloat(v, len, State::ambient_temperature_2);
      break;
    case 6:
      out.label = F("ESC");
      appendFloat(v, len, State::esc_temp);
      break;
    case 7:
      out.label = F("HUM");
      appendFloat(v, len, State::relative_humidity);
      break;
    case 8:
      out.label = F("ALT");
      appendFloat(v, len, State::uncalibrated_altitude);
      break;
    case 9:  // IMU X/Y/Z combined line
      out.label = F("IMU X/Y/Z");
      appendFloat(v, len, State::imu_accel_x);
      append(v, len, " ");
      appendFloat(v, len, State::imu_accel_y);
      append(v, len, " ");
      appendFloat(v, len, State::imu_accel_z);
      break;
    case 10:
      out.label = F("IMU TMP");
      appendFloat(v, len, State::imu_die_temp);
      break;
    case 11:
      out.label = F("IMU MAX1S");
      appendFloat(v, len, State::max_1s_acceleration);
      break;
    case 12:
      out.label = F("GPS SPD");
      appendFloat(v, len, State::gps_speed);
      break;
    case 13:
      out.label = F("OPT SPD");
      appendFloat(v, len, State::kilometers_per_hour);
      break;
    case 14:
      out.label = F("V");
      appendFloat(v, len, State::battery_voltage);
      break;
    case 15:
      out.label = F("I BAT");
      appendFloat(v, len, State::battery_current);
      break;
    case 16:
      out.label = F("I MOT");
      appendFloat(v, len, State::motor_current);
      break;
    case 17:
      out.label = F("PWR");
      appendFloat(v, len, State::battery_power);
      break;
    case 18:
      out.label = F("DUTY");
      appendFloat(v, len, State::duty_cycle);
      break;
    case 19:
      out.label = F("WH OUT");
      appendFloat(v, len, State::watts_used);
      break;
    case 20:
      out.label = F("WH IN");
      appendFloat(v, len, State::watts_charged);
      break;
    case 21:
      out.label = F("LAT");
      appendFloat(v, len, (float)State::latitude / 10000000.0f);
      break;
    case 22:
      out.label = F("LON");
      appendFloat(v, len, (float)State::longitude / 10000000.0f);
      break;
    case 23:
      out.label = F("GPS ALT");
      appendFloat(v, len, (float)State::altitude / 100.0f);
      break;
    case 24:
      out.label = F("HDG");
      appendFloat(v, len, State::heading);
      break;
    case 25:  // SAT fix/visible
      out.label = F("SAT");
      appendInt(v, len, State::fix_satellites);
      append(v, len, "/");
      appendInt(v, len, State::visible_satellites);
      break;
    case 26:
      out.label = F("SOC");
      appendFloat(v, len, State::battery_soc_compensated);
      break;
    case 27:
      out.label = F("TIME REMAIN");
      appendFloat(v, len, State::battery_time_remaining_mins);
      break;
    case 28:
      out.label = F("BATT");
      append(v, len, State::battery_stats);
      break;
    case 29:
      out.label = F("TZ");
      append(v, len, State::timezone_string);
      break;
    case 30:
      out.label = F("RAM");
      appendInt(v, len, State::ram_free_bytes);
      break;
    case 31:
      out.label = F("RAM MIN");
      appendInt(v, len, State::ram_free_bytes_minimum);
      break;
  }
  return true;
}

static bool debugTimingItem(uint8_t idx, DebugLine& out, char* v, size_t len) {
  IntervalMetric* m = METRICS::get(idx);
  if (m == nullptr) {
    return false;
  }
  out.label = m->name();
  v[0] = '\0';
  append(v, len, "min=");
  appendInt(v, len, m->shortest());
  append(v, len, " avg=");
  appendInt(v, len, m->average());
  append(v, len, " max=");
  appendInt(v, len, m->longest());
  return true;
}

static bool debugErrorItem(uint8_t line, DebugLine& out, char* v, size_t len) {
  ErrorEvent e = Errors::newestError(line);
  if (e.type == Error::UNKNOWN) {
    return false;
  }
  out.wholeLine = true;
  unsigned long secs = e.seconds_epoch_time % 86400UL;
  v[0] = '\0';
  append2(v, len, (uint8_t)(secs / 3600));
  append(v, len, ":");
  append2(v, len, (uint8_t)((secs / 60) % 60));
  append(v, len, ":");
  append2(v, len, (uint8_t)(secs % 60));
  append(v, len, " ");
  appendFlash(v, len, Errors::errorDescription(e.type));
  return true;
}

bool debugItem(uint8_t page, uint8_t line, DebugLine& out, char* valueBuf, size_t len) {
  if (page == 2) {
    return debugErrorItem(line, out, valueBuf, len);
  }
  uint8_t idx = page * DEBUG_ITEMS_PER_PAGE + line;
  if (idx >= DEBUG_TIMING_START) {
    if (idx >= DEBUG_TIMING_START + METRICS::count()) {
      return false;
    }
    return debugTimingItem(idx - DEBUG_TIMING_START, out, valueBuf, len);
  }
  if (idx >= DEBUG_STATE_ITEMS) {
    return false;
  }
  return debugStateItem(idx, out, valueBuf, len);
}
