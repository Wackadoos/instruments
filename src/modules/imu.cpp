#include "imu.h"

#include "hardware.h"
#include "utils/errors.h"

bfs::Mpu9250 IMU::imu = bfs::Mpu9250();
IntervalMetric IMU::dataProcessTime = IntervalMetric();

void IMU::init(TwoWire* wire, SensorState* state) {
  sensorState = state;
  imu.Config(wire, bfs::Mpu9250::I2C_ADDR_PRIM);

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
      sensorState->imu_accel_x = imu.accel_x_mps2();
      sensorState->imu_accel_y = imu.accel_y_mps2();
      sensorState->imu_accel_z = imu.accel_z_mps2();
      sensorState->imu_die_temp = imu.die_temp_c();
      // TODO sensorState->max_1s_acceleration
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
