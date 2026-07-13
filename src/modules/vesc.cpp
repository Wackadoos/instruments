#include "vesc.h"

#include "utils/errors.h"

VescUart VESC::vesc = VescUart(10);
IntervalMetric VESC::dataProcessTime = IntervalMetric();

void VESC::init(Stream* port, SensorState* state) {
  sensorState = state;
  vesc.setSerialPort(port);

  dataProcessTime.init(F("VESC Proc"), F("Time to process VESC Data"));
  enabled = true;
}

void VESC::update() {
  if (enabled) {
    dataProcessTime.start();
    if (vesc.getVescValues()) {
      sensorState->motor_current = vesc.data.avgMotorCurrent;
      sensorState->battery_current = vesc.data.avgInputCurrent;
      sensorState->duty_cycle = vesc.data.dutyCycleNow;
      sensorState->battery_voltage = vesc.data.inpVoltage;
      sensorState->watts_used = vesc.data.wattHours;
      sensorState->watts_charged = vesc.data.wattHoursCharged;
      sensorState->esc_temp = vesc.data.tempMosfet;

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
