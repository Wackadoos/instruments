#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(42);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
uint8_t numberOfDevices;          // Number of temperature devices found
DeviceAddress tempDeviceAddress;  // We'll use this variable to store a found device address
uint32_t start, stop;

#define TEMPERATURE_PRECISION 9

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void setup() {
  Serial.begin(500000);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("DS18B20 test"));

  sensors.begin();

  numberOfDevices = sensors.getDeviceCount();

  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode())
    Serial.println("ON");
  else
    Serial.println("OFF");

  // Loop through each device, print out address
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();

      Serial.print("Setting resolution to ");
      Serial.println(TEMPERATURE_PRECISION, DEC);

      // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

      Serial.print("Resolution actually set to: ");
      Serial.print(sensors.getResolution(tempDeviceAddress), DEC);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress) {
  // method 1 - slower
  // Serial.print("Temp C: ");
  // Serial.print(sensors.getTempC(deviceAddress));
  // Serial.print(" Temp F: ");
  // Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));  // Converts tempC to Fahrenheit
}

uint32_t run(int runs) {
  #pragma clang diagnostic ignored "-Wunused-but-set-variable"
  volatile float t;
  start = millis();
  for (int i = 0; i < runs; i++) {
    sensors.requestTemperatures();
    t = sensors.getTempCByIndex(0);
  }
  stop = millis();
  return stop - start;
}

void loop(void) {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();  // Send the command to get temperatures
  Serial.println("DONE");

  // Loop through each device, print out temperature data
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      // Output the device ID
      Serial.print("Temperature for device: ");
      Serial.println(i, DEC);

      // It responds almost immediately. Let's print out the data
      printTemperature(tempDeviceAddress);  // Use a simple function to print out the data
    }
    // else ghost device! Check your power requirements and cabling
  }

  //! Test Speeds
  float ti[4] = {94, 188, 375, 750};

  Serial.println();
  Serial.println("Test takes about 30 seconds for 4 resolutions");
  Serial.println("RES\tTIME\tACTUAL\tGAIN");
  for (int r = 9; r < 13; r++) {
    start = micros();
    sensors.setResolution(r);
    Serial.println(micros() - start);

    start = micros();
    sensors.setResolution(r);
    Serial.println(micros() - start);

    uint32_t duration = run(20);
    float avgDuration = duration / 20.0;

    Serial.print(r);
    Serial.print("\t");
    Serial.print(ti[r - 9]);
    Serial.print("\t");
    Serial.print(avgDuration, 2);
    Serial.print("\t");
    Serial.print(avgDuration * 100 / ti[r - 9], 1);
    Serial.println("%");
  }
  delay(1000);

  //! Async
  // Request temperature conversion (traditional)
  Serial.println("Before blocking requestForConversion");
  unsigned long start = millis();

  sensors.requestTemperatures();

  unsigned long stop = millis();
  Serial.println("After blocking requestForConversion");
  Serial.print("Time used: ");
  Serial.println(stop - start);

  // get temperature
  Serial.print("Temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  Serial.println("\n");

  // Request temperature conversion - non-blocking / async
  Serial.println("Before NON-blocking/async requestForConversion");
  start = millis();
  sensors.setWaitForConversion(false);  // makes it async
  sensors.requestTemperatures();
  sensors.setWaitForConversion(true);
  stop = millis();
  Serial.println("After NON-blocking/async requestForConversion");
  Serial.print("Time used: ");
  Serial.println(stop - start);

  // NOTE! You can use sensors.isConversionComplete() to check while waiting!

  // 9 bit resolution by default
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 9;
  delay(750 / (1 << (12 - resolution)));

  // get temperature
  Serial.print("Temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  Serial.println("\n\n\n\n");

  delay(1500);
}
