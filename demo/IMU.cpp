//! MPU9255 Accelerometer Module
#include <Arduino.h>
#include <mpu6500.h>

bfs::Mpu6500 imu;
volatile bool new_data;

void imu_isr() {
  new_data = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("MPU9250 test"));

  /* Start the I2C bus */
  Wire.begin();
  Wire.setClock(400000);
  /* I2C bus,  0x68 address */
  imu.Config(&Wire, bfs::Mpu6500::I2C_ADDR_SEC);
  /* Initialize and configure IMU */
  if (!imu.Begin()) {
    Serial.println("Couldn't initialise IMU");
    while (true);
  }
  /* Set the sample rate divider for 50hz sampling */
  if (!imu.ConfigSrd(19)) {
    Serial.println("Error configured SRD");
    while (true);
  }
  /* Enabled data ready interrupt */
  if (!imu.EnableDrdyInt()) {
    Serial.println("Error enabling data ready interrupt");
    while (true);
  }
  /* Attach data ready interrupt to pin 3 */
  attachInterrupt(3, imu_isr, RISING);
}

void loop() {
  if (new_data && imu.Read()) {
    Serial.print(imu.new_imu_data());
    Serial.print("\t");
    Serial.print(imu.accel_x_mps2());
    Serial.print("\t");
    Serial.print(imu.accel_y_mps2());
    Serial.print("\t");
    Serial.print(imu.accel_z_mps2());
    Serial.print("\t");
    Serial.print(imu.gyro_x_radps());
    Serial.print("\t");
    Serial.print(imu.gyro_y_radps());
    Serial.print("\t");
    Serial.print(imu.gyro_z_radps());
    Serial.print("\t");
    Serial.print(imu.die_temp_c());
    Serial.print("\n");
  }
}

// TODO “WHO_AM_I” Returns Wrong Value? The MPU9250 should return 0x71 when you read register 0x75. If you get 0x70, you likely have an MPU6500 (no magnetometer). If you get 0x73, you have an MPU9255.
