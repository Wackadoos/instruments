#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#define CS_PIN 49
// MOSI=11, MISO=12, SCK=13

// XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN 19
// XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
// XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void setup() {
  pinMode(48, OUTPUT);
  digitalWrite(48, HIGH);
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);

  Serial.begin(115200);
  delay(3000);
  ts.begin();
  // ts.begin(SPI1); // use alternate SPI port
  ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.print(p.y);
    delay(30);
    Serial.println();
  }
}
