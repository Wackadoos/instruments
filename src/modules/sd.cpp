// //! MicroSD card communications & logging
// // Quick hardware test for SPI card access.
// #include <Arduino.h>

// #ifndef DISABLE_FS_H_WARNING
// #define DISABLE_FS_H_WARNING  // Disable warning for type File not defined.
// #endif                        // DISABLE_FS_H_WARNING
// #include "SdFat.h"
// #include "sdios.h"

// #define SPI_SPEED SD_SCK_MHZ(8)
// SdExFat sd;
// ExFile file;

// // SD card chip select
// int chipSelect = 48;

// void clearSerialInput() {
//   uint32_t m = micros();
//   do {
//     if (Serial.read() >= 0) {
//       m = micros();
//     }
//   } while (micros() - m < 10000);
// }

// void setup() {
//   Serial.begin(500000);
//   delay(3000);
// }

// bool firstTry = true;
// void loop() {
//   // Read any existing Serial data.
//   if (!sd.begin(chipSelect, SPI_SPEED)) {
//     if (sd.card()->errorCode()) {
//     //   cout << int(sd.card()->errorCode());
//     //   cout << F(", errorData: ") << int(sd.card()->errorData());
//       return;
//     }
//     if (sd.vol()->fatType() == 0) {
//     //   cout << F("Can't find a valid FAT16/FAT32/exFAT partition.\n");
//       return;
//     }
//     // cout << F("Can't determine error type\n");
//     return;
//   }

//   uint32_t size = sd.card()->sectorCount();
//   if (size == 0) {
//     // cout << F("Can't determine the card size.\n");
//     return;
//   }
//   uint32_t sizeMB = 0.000512 * size + 0.5;
// //   cout << F("Card size: ") << sizeMB;
// //   cout << F(" MB (MB = 1,000,000 bytes)\n");
//   if (sd.fatType() <= 32) {
//     // cout << F("\nVolume is FAT") << int(sd.fatType());
//   } else {
//     // cout << F("\nVolume is exFAT");
//   }
// //   cout << F(", Cluster size (bytes): ") << sd.vol()->bytesPerCluster();

// //   cout << F("Files found (date time size name):\n");
// //   sd.ls(LS_R | LS_DATE | LS_SIZE);

//   if ((sizeMB > 1100 && sd.vol()->sectorsPerCluster() < 64) ||
//       (sizeMB < 2200 && sd.vol()->fatType() == 32)) {
//     // cout << F("\nThis card should be reformatted for best performance.\n");
//     // cout << F("Use a cluster size of 32 KB for cards larger than 1 GB.\n");
//     // cout << F("Only cards larger than 2 GB should be formatted FAT32.\n");
//     return;
//   }
// }

// // TODO Preallocate sd card space for race mode
// // TODO Do we need to flush sd to not corrupt in power down failure? How does this work?
// // TODO Should we also log which driver is currently driving along with the data?

// // SD whenever needed, smaller chunks ideal avoid stutter
// // New file per driver?
// // Timestamps with sensor readings?
