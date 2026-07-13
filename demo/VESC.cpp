//! FLIPSKY 75100 ESC Communications
#include <Arduino.h>
#include <VescUart.h>

VescUart VESC = VescUart(10);

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);  // wait for native usb
  Serial.println(F("ESC test"));

  //! NOTE apparently 115200 baud on vesc to arduino isn't reliable (due to clock), use 250000. (see timings table for 16mhz clock https://wormfood.net/avrbaudcalc.php?bitrate=300%2C600%2C1200%2C2400%2C4800%2C9600%2C14.4k%2C19.2k%2C28.8k%2C38.4k%2C57.6k%2C76.8k%2C115.2k%2C230.4k%2C250k%2C460.8k%2C.5m%2C921.6k%2C1m&clock=16&databits=8)
  Serial2.begin(250000);

  VESC.setDebugPort(&Serial);
  VESC.setSerialPort(&Serial2);
}

void loop() {
  if (VESC.getVescValues()) {
    // Serial.println(VESC.data.rpm);
    // Serial.println(VESC.data.inpVoltage);
    // Serial.println(VESC.data.ampHours);
    // Serial.println(VESC.data.tachometerAbs);
    VESC.printVescValues();
  } else {
    Serial.println("Failed to get data!");
  }

  delay(500);
}
