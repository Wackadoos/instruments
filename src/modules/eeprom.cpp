#include "eeprom.h"

#include "hardware.h"
#include "utils/errors.h"

ExternalEEPROM EEPROM::eeprom = ExternalEEPROM();

void EEPROM::init(TwoWire* wire) {
  if (!eeprom.begin(EEPROM_I2C_ADDR, *wire)) {
    Errors::logError(Error::EEPROM_UNINITIALISED);
    return;
  }

  // AT24C32 = 4096 bytes, 32-byte pages, 2 address bytes
  eeprom.setMemorySizeBytes(4096);
  eeprom.setPageSizeBytes(32);
  eeprom.setAddressBytes(2);

  enabled = true;
}

// TODO Make sure battery capacity and current discharge metrics are persisted in short power outages. Actually just make race mode not disengage unless more than 60 mins passed. How to deal with no rtc on reboot?
