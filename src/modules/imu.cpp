#include "imu.h"

#include "hardware.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

bfs::Mpu6500 IMU::imu = bfs::Mpu6500();
IntervalMetric IMU::dataProcessTime = IntervalMetric();

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

      // TODO SensorState::max_1s_acceleration
      //  imu.gyro_x_radps();
      //  imu.gyro_y_radps();
      //  imu.gyro_z_radps();
      //  imu.mag_x_ut();
      //  imu.mag_y_ut();
      //  imu.mag_z_ut();
    } else {
      Errors::logError(Error::IMU_DATA_READ_FAILED);
    }
    dataProcessTime.stop();
  }
}

void IMU::imu_isr() {
  dataReady = true;
}
