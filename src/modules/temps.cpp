#include "temps.h"

#include "utils/errors.h"

// TODO have interface for enumerating and selecting and storing which temp probes are available and which is which

void TEMPS::init(OneWire* oneWire, SensorState* state) {
  sensorState = state;
  sensors.setOneWire(oneWire);
  sensors.begin();
  auto numberOfDevices = sensors.getDeviceCount();

  if (numberOfDevices == EXPECTED_SENSOR_COUNT) {
    enabled = true;
    for (int i = 0; i < EXPECTED_SENSOR_COUNT; i++) {
      if (sensors.getAddress(addresses[i], i)) {
        sensors.setResolution(addresses[i], SENSOR_RESOLUTION);
        if (sensors.getResolution(addresses[i]) != SENSOR_RESOLUTION) {
          Errors::logError(Error::TEMP_RESOLUTION_INCORRECT);  // Not a reason to disable the module, just to notify
        }
        sensors.setWaitForConversion(false);  // Enable async mode
      } else {
        enabled = false;
        Errors::logError(Error::GHOST_TEMP_PROBE);
        return;
      }
    }
    if (!sortAddresses(addresses, EXPECTED_SENSOR_COUNT)) {
      enabled = false;
      Errors::logError(Error::DUPLICATE_TEMP_SENSOR_IDS);
    }
  } else if (numberOfDevices == 0) {
    Errors::logError(Error::NO_TEMPS_FOUND);
  } else {
    Errors::logError(Error::INCORRECT_NUM_TEMPS);
  }
}

void TEMPS::update() {
  if (enabled) {
    sensors.requestTemperaturesByAddress(addresses[current_address]);
  }
}

void TEMPS::run() {
  if (enabled && sensors.isConversionComplete()) {  // isConversionComplete reads i2c bit which has micros waits. Could check this less often for efficiency!
    float tempC = sensors.getTempC(addresses[current_address]);

    if (tempC == DEVICE_DISCONNECTED_C) {
      Errors::logError(Error::TEMP_UNREADABLE);
    } else {
      sensorState->temp_motor_front = tempC;  // TODO set according to current_address + mapping setting
      // sensorState->temp_motor_back = tempC;
    }

    current_address++;
    if (current_address >= EXPECTED_SENSOR_COUNT) {
      current_address = 0;
    }
  }
}

int8_t TEMPS::compare(DeviceAddress a, DeviceAddress b) {
  for (uint8_t i = 0; i < sizeof(DeviceAddress); i++) {
    if (a[i] > b[i]) return 1;
    if (a[i] < b[i]) return -1;
  }
  return 0;
}

bool TEMPS::sortAddresses(DeviceAddress* addresses, uint8_t count) {  // Insertion Sort
  for (uint8_t i = 1; i < count; i++) {
    uint8_t key[sizeof(DeviceAddress)];
    for (uint8_t k = 0; k < sizeof(DeviceAddress); k++) key[k] = addresses[i][k];

    int8_t j = i - 1;
    while (j >= 0) {
      int8_t cmp = compare(addresses[j], key);
      if (cmp <= 0) break;
      for (uint8_t k = 0; k < sizeof(DeviceAddress); k++) addresses[j + 1][k] = addresses[j][k];
      j--;
    }
    for (uint8_t k = 0; k < sizeof(DeviceAddress); k++) addresses[j + 1][k] = key[k];
  }

  for (uint8_t i = 0; i + 1 < count; i++) {
    uint8_t same = 1;
    for (uint8_t k = 0; k < sizeof(DeviceAddress); k++) {
      if (addresses[i][k] != addresses[i + 1][k]) {
        same = 0;
        break;
      }
    }
    if (same) return false;
  }

  return true;
}
