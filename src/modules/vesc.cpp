#include "vesc.h"

#include "state.h"
#include "utils/errors.h"
#include "utils/logging.h"

VescUart VESC::vesc = VescUart(10);
IntervalMetric VESC::dataProcessTime = IntervalMetric();

void VESC::init(Stream* port) {
  vesc.setSerialPort(port);

  dataProcessTime.init(F("VESC Proc"), F("Time to process VESC Data"));
  enabled = true;
}

void VESC::update() {
  if (enabled) {
    dataProcessTime.start();
    if (vesc.getVescValues()) {
      SensorState::motor_current = vesc.data.avgMotorCurrent;
      Logging::logDebug(F("VESC Motor Current: "), vesc.data.avgMotorCurrent);
      SensorState::battery_current = vesc.data.avgInputCurrent;
      Logging::logDebug(F("VESC Battery Current: "), vesc.data.avgInputCurrent);
      SensorState::duty_cycle = vesc.data.dutyCycleNow;
      Logging::logDebug(F("VESC Duty Cycle: "), vesc.data.dutyCycleNow);
      SensorState::battery_voltage = vesc.data.inpVoltage;
      Logging::logDebug(F("VESC Battery Voltage: "), vesc.data.inpVoltage);
      SensorState::watts_used = vesc.data.wattHours;
      Logging::logDebug(F("VESC Watts Used: "), vesc.data.wattHours);
      SensorState::watts_charged = vesc.data.wattHoursCharged;
      Logging::logDebug(F("VESC Watts Charged: "), vesc.data.wattHoursCharged);
      SensorState::esc_temp = vesc.data.tempMosfet;
      Logging::logDebug(F("VESC Temp: "), vesc.data.tempMosfet);

      if (vesc.data.error) {  // Errors should be uncommon, avoid unnecessarily setting up jump table
        // TODO need a way to log additional error data like code
        switch (vesc.data.error) {
          case FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_1:
          case FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_2:
          case FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_3:
          case FAULT_CODE_UNBALANCED_CURRENTS:
          case FAULT_CODE_BRK:
          case FAULT_CODE_RESOLVER_LOT:
          case FAULT_CODE_RESOLVER_DOS:
          case FAULT_CODE_RESOLVER_LOS:
          case FAULT_CODE_DRV:
            Errors::logError(Error::VESC_REPORTED_HARDWARE_FAULT);
            break;
          case FAULT_CODE_OVER_TEMP_FET:
          case FAULT_CODE_OVER_TEMP_MOTOR:
            Errors::logError(Error::VESC_REPORTED_TEMPERATURE_FAULT);
            break;
          case FAULT_CODE_OVER_VOLTAGE:
          case FAULT_CODE_UNDER_VOLTAGE:
          case FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE:
          case FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE:
          case FAULT_CODE_MCU_UNDER_VOLTAGE:
            Errors::logError(Error::VESC_REPORTED_VOLTAGE_FAULT);
            break;
          case FAULT_CODE_FLASH_CORRUPTION:
          case FAULT_CODE_FLASH_CORRUPTION_APP_CFG:
          case FAULT_CODE_FLASH_CORRUPTION_MC_CFG:
            Errors::logError(Error::VESC_REPORTED_FLASH_CORRUPTION);
            break;
          default:
            Errors::logError(Error::VESC_REPORTED_UNCOMMON_ERROR);
            break;
        }
      }
    } else {
      Errors::logError(Error::VESC_DATA_UNAVAILABLE);
    }
    dataProcessTime.stop();
  }
}
