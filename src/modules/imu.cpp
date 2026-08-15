#include "imu.h"

#include "hardware.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

bfs::Mpu6500 IMU::imu = bfs::Mpu6500();
IntervalMetric IMU::dataProcessTime = IntervalMetric();

float IMU::accelBuf[IMU::AXES][IMU::SAMPLES] = {};
uint8_t IMU::accelMinIdx[IMU::AXES] = {};
uint8_t IMU::accelHead = 0;
uint16_t IMU::accelCount = 0;

void IMU::init(TwoWire* wire) {
  imu.Config(wire, IMU_I2C_ADDR);

  if (!imu.Begin()) {
    Errors::logError(Error::IMU_UNINITIALISED);
    return;
  }
  if (!imu.ConfigSrd(19)) {  // Set sample rate to 50hz
    Errors::logError(Error::IMU_SAMPLE_RATE_ERR);
    return;
  }
  if (!imu.EnableDrdyInt()) {  // Enable interrupt on new sample
    Errors::logError(Error::IMU_INTERRUPT_ERR);
    return;
  }

  attachInterrupt(digitalPinToInterrupt(ACCELEROMETER_INTERRUPT_PIN), imu_isr, RISING);

  dataProcessTime.init(F("IMU Proc"), F("Time to process IMU Data"));
  enabled = true;
}

void IMU::run() {
  if (enabled && dataReady) {
    dataReady = false;
    dataProcessTime.start();
    if (imu.Read()) {
      State::imu_accel_x = imu.accel_x_mps2();
      State::imu_accel_y = imu.accel_y_mps2();
      State::imu_accel_z = imu.accel_z_mps2();
      State::imu_die_temp = imu.die_temp_c();

#ifdef DEBUG_LOGGING
      static uint8_t counter = 0;
      if (counter >= 24) {
        Logging::logDebug(F("IMU x: "), imu.accel_x_mps2());
        Logging::logDebug(F("IMU y: "), imu.accel_y_mps2());
        Logging::logDebug(F("IMU z: "), imu.accel_z_mps2());
        Logging::logDebug(F("IMU Temp: "), imu.die_temp_c());
        counter = 0;
      } else {
        counter++;
      }
#endif

      updateMax1s(0, fabsf(imu.accel_x_mps2()));
      updateMax1s(1, fabsf(imu.accel_y_mps2()));
      updateMax1s(2, fabsf(imu.accel_z_mps2()));
    } else {
      Errors::logError(Error::IMU_DATA_READ_FAILED);
    }
    dataProcessTime.stop();
  }
}

// Tracks the minimum of the last SAMPLES samples per axis (the 1s dwell value) using an
// incremental running minimum: the whole window is only re-scanned when the evicted sample
// is the current minimum. The window minimum is promoted to the per-axis peak once the
// window is full.
void IMU::updateMax1s(uint8_t axis, float value) {
  uint8_t evict = accelHead;
  accelBuf[axis][evict] = value;
  accelHead = (uint8_t)((accelHead + 1) % SAMPLES);

  if (accelCount < SAMPLES) {
    accelCount++;
    if (accelCount == 1 || value < accelBuf[axis][accelMinIdx[axis]]) {
      accelMinIdx[axis] = evict;
    }
    if (accelCount == SAMPLES) {
      promote(axis, accelBuf[axis][accelMinIdx[axis]]);
    }
    return;
  }

  if (accelMinIdx[axis] == evict) {
    // The running minimum was evicted: re-scan the whole window.
    uint8_t minIdx = 0;
    for (uint8_t i = 1; i < SAMPLES; i++) {
      if (accelBuf[axis][i] < accelBuf[axis][minIdx]) {
        minIdx = i;
      }
    }
    accelMinIdx[axis] = minIdx;
  } else if (value < accelBuf[axis][accelMinIdx[axis]]) {
    accelMinIdx[axis] = evict;
  }
  promote(axis, accelBuf[axis][accelMinIdx[axis]]);
}

// Promotes a window minimum into the per-axis peak when it beats the stored value, keeping
// the combined State::max_1s_acceleration in sync as the max of the three peaks.
void IMU::promote(uint8_t axis, float windowMin) {
  switch (axis) {
    case 0:
      if (windowMin > State::max_1s_accel_x) State::max_1s_accel_x = windowMin;
      break;
    case 1:
      if (windowMin > State::max_1s_accel_y) State::max_1s_accel_y = windowMin;
      break;
    case 2:
      if (windowMin > State::max_1s_accel_z) State::max_1s_accel_z = windowMin;
      break;
  }
  float m = State::max_1s_accel_x;
  if (State::max_1s_accel_y > m) m = State::max_1s_accel_y;
  if (State::max_1s_accel_z > m) m = State::max_1s_accel_z;
  State::max_1s_acceleration = m;
}

void IMU::imu_isr() {
  dataReady = true;
}
