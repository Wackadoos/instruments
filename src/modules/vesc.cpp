#include "vesc.h"

#include "modules/battery.h"
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
      State::motor_current = vesc.data.avgMotorCurrent;
      State::battery_current = vesc.data.avgInputCurrent;
      State::duty_cycle = vesc.data.dutyCycleNow;
      State::battery_voltage = vesc.data.inpVoltage + VOLTAGE_CALIBRATION;
      State::watts_used = vesc.data.wattHours;
      State::watts_charged = vesc.data.wattHoursCharged;
      State::esc_temp = vesc.data.tempMosfet;
      State::battery_power = State::battery_voltage * State::battery_current;
      State::battery_power_avg = (State::battery_power_avg == 0)
                                     ? State::battery_power
                                     : 0.025f * State::battery_power + 0.975f * State::battery_power_avg;

      static uint8_t counter = 0;
      if (counter >= 3) {  // every 4th VESC read = 1.0s @ 250ms task, so 60 window samples = 1 min
        BatteryEstimator::updateEWMA(vesc.data.avgInputCurrent);
        State::battery_soc_compensated = BatteryEstimator::estimateSOC(vesc.data.wattHours, vesc.data.wattHoursCharged, State::ambient_temperature_2);
        State::battery_time_remaining_mins = BatteryEstimator::estimateTimeRemainingMinutes();
        char fbuf1[8], fbuf2[8];
        dtostrf(State::battery_soc_compensated, 3, 0, fbuf1);  // width 3, 0 decimal
        dtostrf(State::battery_time_remaining_mins, 3, 0, fbuf2);
        sprintf(State::battery_stats, "%s%% %sm", fbuf1, fbuf2);

        Logging::logDebug(F("VESC Motor Current: "), State::motor_current);
        Logging::logDebug(F("VESC Battery Current: "), State::battery_current);
        Logging::logDebug(F("VESC Duty Cycle: "), State::duty_cycle);
        Logging::logDebug(F("VESC Battery Voltage: "), State::battery_voltage);
        Logging::logDebug(F("VESC Watts Used: "), State::watts_used);
        Logging::logDebug(F("VESC Watts Charged: "), State::watts_charged);
        Logging::logDebug(F("VESC Temp: "), State::esc_temp);
        Logging::logDebug(F("VESC Power: "), State::battery_power);
        counter = 0;
      } else {
        counter++;
      }

      if (vesc.data.error) {  // Errors should be uncommon, avoid unnecessarily setting up jump table
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
