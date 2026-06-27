# instruments

Software to run in-vehicle instruments

## FLIPSKY 75100 Pro v1 Settings

Unless specified below, leave settings as default!

### Firmware

Stock ESC comes with VESC v5.02. Download and flash the latest (should be v7.0 or higher). Notes:

- The hardware is classified incorrectly as 75_300_R2, this is okay as it is compatible.
- All settings will be reset with the update
- Phase Filters will be enabled by default, but are **NOT COMPATIBLE** with this hardware. This needs to be turned off before running a motor!

### Motor Settings

#### General

General:

- Motor Type: DC

Current:

- Motor Current Max: 45.00 A
- Motor Current Max Brake: -45.00 A
- Absolute Maximum Current: 420.00 A
- Battery Current Max: 36.00 A
- Battery Current Max Regen: -36.00 A
- DRV8301 OC Mode: Disabled

Voltage:

- Battery Voltage Cutoff Start: 20.0 V
- Battery Voltage Cutoff End: 16.0 V
- Battery Voltage Regen Cutoff Start: 28.0 V
- Battery Voltage Regen Cutoff End: 30.0 V

Temperature:

- Motor Temperature Sensor Type: Disabled

BMS:

- BMS type: None

Advanced:

- Minimum Input Voltage: 11.0 V
- Maximum Input Voltage: 36.0 V
- Fault Stop Time: 3000 ms

#### DC

- Current Controller Gain: 0.00460
- Current Controller Ramp Step Max: 0.0400
- Current Backoff Gain: 0.5000
- Switching Frequency: 25.00 kHz

#### Additional Info

Setup:

- Battery Type: BATTERY_TYPE_LEAD_ACID
- Battery Cells Series: 12
- Battery Capacity: 36 AH
- Motor No-load Current: 1.0 A

Motor General:

- Motor Brand: Greenpower Motor
- Motor Model: 105ZDY08

### App Settings

#### General

General:

- APP to Use: ADC and UART
- Timeout Brake Current: 5.0 A
- CAN Mode: Unused
- Kill Switch Mode: PPM High

#### ADC

General:

- Control Type: Current
- Use Filter: True
- Safe Start: Regular
- Update Rate: 500 Hz
- Positive Ramping Time: 0.3 s
- Negative Ramping Time: 0.1 s
- Multiple VESCs Over CAN: FALSE

Mapping:

- Input Deadband: 15%
- ADC1 Start Voltage: 0.1 V
- ADC1 End Voltage: 3.2 V
- ADC1 Centre Voltage: 0.1 V
- Invert ADC1 Voltage: False
- ADC1 Abs Min Voltage: 0.0 V
- ADC1 Abs Max Voltage: 3.5 V
- ADC2 Start Voltage: 0.1 V
- ADC2 End Voltage: 3.2 V
- Invert ADC2 Voltage: False

#### UART

Baudrate: 250000 bps

#### IMU

IMU Type: IMU_TYPE_OFF

### SWD

To program an NRF51822 for Bluetooth connectivity to the ESC, connect to the swd pins (DIO -> SDO & CLK -> SCLK) and press connect in VESC. Then, select the firmware with RX: 11 TX: 9 and flash it. After, you can connect the pins P0.11 -> TX2 and P0.9 -> RX2 to allow the bluetooth module to talk to the ESC on it's secondary serial port (leaving the primary for app usage ie connecting with arduino etc.)
