#include <Arduino.h>
#include <SparkFun_External_EEPROM.h>
#include <Wire.h>

ExternalEEPROM eeprom;

void setup() {
  Serial.begin(500000);
  while (!Serial)
    delay(100);  // wait for native usb
  delay(3000);
  Serial.println(F("EEPROM test"));

  Wire.begin();
  eeprom.begin(0x57, Wire);
  // for (int addr = 0; addr <= 80; addr++) {
  //   if (eeprom.begin(addr, Wire)) {
  //     Serial.print("EEPROM detected on address ");
  //     Serial.println(addr);
  //     break;
  //   } else if (addr == 80) {
  //     Serial.println("EEPROM not detected. onCheck wiring.");
  //     while (true);
  //   }
  // }

  Serial.print("Detected number of address bytes: ");
  Serial.println(eeprom.detectAddressBytes());

  uint32_t eepromSizeBytes = eeprom.detectMemorySizeBytes();
  Serial.print("Detected EEPROM size (bytes): ");
  Serial.print(eepromSizeBytes);
  Serial.print(" bytes / ");
  if (eepromSizeBytes < 128) {
    Serial.print(eepromSizeBytes * 8);
    Serial.print(" Bits");
  } else {
    Serial.print(eepromSizeBytes * 8 / 1024);
    Serial.print(" kBits");
  }
  Serial.print(" - 24XX");
  if (eepromSizeBytes == 16)
    Serial.print("00");
  else {
    if ((eepromSizeBytes * 8 / 1024) < 10) Serial.print("0");
    Serial.print(eepromSizeBytes * 8 / 1024);
  }
  Serial.println();

  // AT24C32 = 4096 bytes, 32-byte pages, 2 address bytes
  eeprom.setMemorySizeBytes(4096);
  eeprom.setPageSizeBytes(32);
  eeprom.setAddressBytes(2);

  Serial.print("Set actual values. Continuing:");

  // Yes you can read and write bytes, but you shouldn't!
  byte myValue1a = 200;
  eeprom.write(0, myValue1a);  //(location, data)

  byte myRead1 = eeprom.read(0);
  Serial.print("I read (should be 200): ");
  Serial.println(myRead1);

  // You should use gets and puts. This will automatically and correctly arrange
  // the bytes for larger variable types.
  int myValue2a = -366;
  eeprom.put(10, myValue2a);  //(location, data)
  int myRead2;
  eeprom.get(10, myRead2);  // location to read, thing to put data into
  Serial.print("I read (should be -366): ");
  Serial.println(myRead2);

  float myValue3a = -7.35;
  eeprom.put(20, myValue3a);  //(location, data)
  float myRead3;
  eeprom.get(20, myRead3);  // location to read, thing to put data into
  Serial.print("I read (should be -7.35): ");
  Serial.println(myRead3);

  String myString = "Hi, I am just a simple test string";
  unsigned long nextEEPROMLocation = eeprom.putString(30, myString);
  String myRead4 = "";
  eeprom.getString(30, myRead4);
  Serial.print("I read: ");
  Serial.println(myRead4);
  Serial.print("Next available EEPROM location: ");
  Serial.println(nextEEPROMLocation);

  //! Tests
  unsigned int randomLocation;
  bool allTestsPassed = true;

  // Erase test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.print("Time to erase all EEPROM: ");
  long startTime = millis();
  eeprom.erase();
  long endTime = millis();
  Serial.print(endTime - startTime);
  Serial.println("ms");
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // Byte sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.println();
  Serial.println("8 bit tests");
  byte myValue1 = 200;
  byte myValue2 = 23;
  randomLocation = random(0, eeprom.length() - (sizeof(byte) * 2));

  startTime = micros();
  eeprom.write(randomLocation, myValue1);  //(location, data)
  while (eeprom.isConnected() == false);   // Wait for write to complete
  endTime = micros();
  Serial.print("Time to record byte: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.put(randomLocation + sizeof(byte), myValue2);
  while (eeprom.isConnected() == false);  // Wait for write to complete

  startTime = micros();
  eeprom.write(randomLocation, myValue1);  //(location, data)
  while (eeprom.isConnected() == false);   // Wait for write to complete
  endTime = micros();
  Serial.print("Time to write identical byte to same location (should be ~0ms): ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  startTime = micros();
  byte response1 = eeprom.read(randomLocation);
  endTime = micros();
  Serial.print("Time to read byte: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  byte response2 = eeprom.read(randomLocation + sizeof(byte));
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue1);
  Serial.print(": ");
  Serial.print(response1);
  if (myValue1 == response1 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(byte));
  Serial.print(" should be ");
  Serial.print(myValue2);
  Serial.print(": ");
  Serial.print(response2);
  if (myValue2 == response2 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue1 != response1)
    allTestsPassed = false;
  if (myValue2 != response2)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("16 bit tests");

  // int16_t and uint16_t tests
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  uint16_t myValue3 = 3411;
  int16_t myValue4 = -366;
  randomLocation = random(0, eeprom.length() - (sizeof(int16_t) * 2));

  startTime = micros();
  eeprom.put(randomLocation, myValue3);
  endTime = micros();
  Serial.print("Time to record int16: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.put(randomLocation + sizeof(int16_t), myValue4);

  uint16_t response3;
  int16_t response4;

  startTime = micros();
  eeprom.get(randomLocation, response3);
  endTime = micros();
  Serial.print("Time to read int16: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.get(randomLocation + sizeof(int16_t), response4);
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue3);
  Serial.print(": ");
  Serial.print(response3);
  if (myValue3 == response3 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(int16_t));
  Serial.print(" should be ");
  Serial.print(myValue4);
  Serial.print(": ");
  Serial.print(response4);
  if (myValue4 == response4 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue3 != response3)
    allTestsPassed = false;
  if (myValue4 != response4)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("int tests");

  // int and unsigned int tests
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.print("Size of int: ");
  Serial.println(sizeof(int));  // Uno reports this as 2
  int myValue5 = -2450;
  unsigned int myValue6 = 4001;
  randomLocation = random(0, eeprom.length() - (sizeof(int) * 2));

  startTime = micros();
  eeprom.put(randomLocation, myValue5);
  endTime = micros();
  Serial.print("Time to record int: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");
  eeprom.put(randomLocation + sizeof(int), myValue6);

  int response5;
  unsigned int response6;

  startTime = micros();
  eeprom.get(randomLocation, response5);
  endTime = micros();
  Serial.print("Time to read int: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.get(randomLocation + sizeof(int), response6);
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue5);
  Serial.print(": ");
  Serial.print(response5);
  if (myValue5 == response5 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(int));
  Serial.print(" should be ");
  Serial.print(myValue6);
  Serial.print(": ");
  Serial.print(response6);
  if (myValue6 == response6 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue5 != response5)
    allTestsPassed = false;
  if (myValue6 != response6)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("32 bit tests");

  // int32_t and uint32_t sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  int32_t myValue7 = -341002;
  uint32_t myValue8 = 241544;
  randomLocation = random(0, eeprom.length() - (sizeof(int32_t) * 2));

  eeprom.put(randomLocation, myValue7);
  eeprom.put(randomLocation + sizeof(int32_t), myValue8);

  int32_t response7;
  uint32_t response8;

  startTime = micros();
  eeprom.get(randomLocation, response7);
  endTime = micros();
  Serial.print("Time to read int32: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.get(randomLocation + sizeof(int32_t), response8);
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue7);
  Serial.print(": ");
  Serial.print(response7);
  if (myValue7 == response7 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(int32_t));
  Serial.print(" should be ");
  Serial.print(myValue8);
  Serial.print(": ");
  Serial.print(response8);
  if (myValue8 == response8 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue7 != response7)
    allTestsPassed = false;
  if (myValue8 != response8)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // float (32) sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.print("Size of float: ");
  Serial.println(sizeof(float));
  float myValue9 = -7.35;
  float myValue10 = 5.22;
  randomLocation = random(0, eeprom.length() - (sizeof(float) * 2));

  eeprom.put(randomLocation, myValue9);
  eeprom.put(randomLocation + sizeof(float), myValue10);

  float response9;
  float response10;

  startTime = micros();
  eeprom.get(randomLocation, response9);
  endTime = micros();
  Serial.print("Time to read float: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.get(randomLocation + sizeof(float), response10);
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue9);
  Serial.print(": ");
  Serial.print(response9);
  if (myValue9 == response9 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(float));
  Serial.print(" should be ");
  Serial.print(myValue10);
  Serial.print(": ");
  Serial.print(response10);
  if (myValue10 == response10 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue9 != response9)
    allTestsPassed = false;
  if (myValue10 != response10)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("64 bit tests");

  // double (64) sequential test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  Serial.print("Size of double: ");
  Serial.println(sizeof(double));
  double myValue11 = -290.3485723409857;
  double myValue12 = 384.957;  // 34987;
  double myValue13 = 917.14159;
  double myValue14 = 254.8877;
  randomLocation = random(0, eeprom.length() - (sizeof(double) * 2));

  startTime = micros();
  eeprom.put(randomLocation, myValue11);
  endTime = micros();
  Serial.print("Time to record 64-bits: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.put(randomLocation + sizeof(double), myValue12);
  eeprom.put(eeprom.length() - sizeof(double), myValue13);  // Test end of EEPROM space

  double response11;
  double response12;
  double response13;

  startTime = micros();
  eeprom.get(randomLocation, response11);
  endTime = micros();
  Serial.print("Time to read 64-bits: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  eeprom.get(randomLocation + sizeof(double), response12);
  eeprom.get(eeprom.length() - sizeof(double), response13);
  Serial.print("Location ");
  Serial.print(randomLocation);
  Serial.print(" should be ");
  Serial.print(myValue11);
  Serial.print(": ");
  Serial.print(response11);
  if (myValue11 == response11 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Location ");
  Serial.print(randomLocation + sizeof(double));
  Serial.print(" should be ");
  Serial.print(myValue12);
  Serial.print(": ");
  Serial.print(response12);
  if (myValue12 == response12 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  Serial.print("Edge of EEPROM ");
  Serial.print(eeprom.length() - sizeof(double));
  Serial.print(" should be ");
  Serial.print(myValue13);
  Serial.print(": ");
  Serial.print(response13);
  if (myValue13 == response13 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  double response14;
  eeprom.put(eeprom.length() - sizeof(double), myValue14);  // Test the re-write of a spot
  eeprom.get(eeprom.length() - sizeof(double), response14);
  Serial.print("Rewrite of ");
  Serial.print(eeprom.length() - sizeof(double));
  Serial.print(" should be ");
  Serial.print(myValue14);
  Serial.print(": ");
  Serial.print(response14);
  if (myValue14 == response14 ? Serial.print(" - Success") : Serial.print(" - Fail"))
    ;
  Serial.println();

  if (myValue11 != response11)
    allTestsPassed = false;
  if (myValue12 != response12)
    allTestsPassed = false;
  if (myValue13 != response13)
    allTestsPassed = false;
  if (myValue14 != response14)
    allTestsPassed = false;
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println("");
  Serial.println("Buffer Write Test");

  // Buffer write test
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // char myChars[242] = "Lorem ipsum dolor sit amet, has in verterem accusamus. Nulla viderer inciderint eum at. Quo
  // elit nullam malorum te, agam fuisset detracto an sea, eam ut liber aperiri. Id qui velit facilisi. Mel probatus
  // definitionem id, eu amet vidisse eum.";
  char myChars[88] = "Lorem ipsum dolor sit amet, has in verterem accusamus. Nulla viderer inciderint eum at.";
  randomLocation = random(0, eeprom.length() - sizeof(myChars));

  Serial.print("Calculated time to record array of ");
  Serial.print(sizeof(myChars));
  Serial.print(" characters: ~");
  Serial.print((uint32_t)sizeof(myChars) / eeprom.getPageSizeBytes() * eeprom.getWriteTimeMs());
  Serial.println("ms");

  startTime = micros();
  eeprom.put(randomLocation, myChars);
  endTime = micros();
  Serial.print("Time to record array: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  char readMy[sizeof(myChars)];

  startTime = micros();
  eeprom.get(randomLocation, readMy);
  endTime = micros();
  Serial.print("Time to read array: ");
  Serial.print(endTime - startTime);
  Serial.println(" us");

  //  Serial.print("Location ");
  //  Serial.print(randomLocation);
  //  Serial.print(" string should read:");
  //  Serial.println(myChars);
  //  Serial.println(readMy);
  if (strcmp(myChars, readMy) != 0) {
    Serial.println("String compare failed");
    allTestsPassed = false;
  }

  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  Serial.println();
  Serial.print("Memory Contents:");
  for (uint16_t x = 0; x < 32 * 4; x++) {
    if (x % 16 == 0)
      Serial.println();
    Serial.print(" 0x");
    if (eeprom.read(x) < 0x10)
      Serial.print("0");
    Serial.print(eeprom.read(x), HEX);
  }
  Serial.println();

  if (allTestsPassed == true)
    Serial.println("All tests PASSED!");
  else
    Serial.println("Something went wrong. See output.");
}

void loop() {}
