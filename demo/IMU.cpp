//! MPU9255 Accelerometer Module
#include <Arduino.h>

void setup()
{
}

void loop()
{
}

// TODO “WHO_AM_I” Returns Wrong Value? The MPU9250 should return 0x71 when you read register 0x75. If you get 0x70, you likely have an MPU6500 (no magnetometer). If you get 0x73, you have an MPU9255.
