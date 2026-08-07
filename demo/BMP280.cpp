#include <Adafruit_BMP280.h>
#include <Arduino.h>

Adafruit_BMP280 bmp280;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("BMP280 test"));

  if (!bmp280.begin()) {
    Serial.println(F("BMP280 connection failed!"));
    while (true);
  }

  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,    /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,    /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X8,    /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_X8,      /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_63); /* Standby time. */
}

void loop() {
  Serial.print(F("Temperature = "));
  Serial.print(bmp280.readTemperature());
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(bmp280.readPressure());
  Serial.println(" Pa");

  Serial.print(F("Approx altitude = "));
  Serial.print(bmp280.readAltitude(1013.25)); /* Adjusted to local forecast! */
  Serial.println(" m");

  Serial.println();
  delay(500);
}
