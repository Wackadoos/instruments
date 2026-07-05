#include "eeprom.h"

#include "errors.h"

void EEPROM::init(TwoWire* wire) {
  if (eeprom.begin(0x50, *wire)) {
    enabled = true;
    // AT24C32 = 4096 bytes, 32-byte pages, 2 address bytes
    eeprom.setMemorySizeBytes(4096);
    eeprom.setPageSizeBytes(32);
    eeprom.setAddressBytes(2);
  } else {
    Errors::logError(Error::EEPROM_UNINITIALISED);
  }
}
