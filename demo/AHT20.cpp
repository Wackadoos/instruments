#include <Adafruit_AHTX0.h>
#include <Arduino.h>

Adafruit_AHTX0 ath20;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("ATH20 test"));

  if (!ath20.begin()) {
    Serial.println(F("Could not find AHT? Check wiring"));
    while (true);
  }
}

void loop() {
  sensors_event_t humidity, temp;
  ath20.getEvent(&humidity,
                 &temp);  // populate temp and humidity objects with fresh data
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degrees C");
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println("% rH");

  delay(500);
}
