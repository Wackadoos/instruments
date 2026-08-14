#include "GPS.h"

#include "GPSport.h"
#include "Streamers.h"
#include "hardware.h"
#include "state.h"
#include "utils/errors.h"

NMEAGPS GPS::gps = NMEAGPS();
gps_fix GPS::fix = gps_fix();
IntervalMetric GPS::dataProcessTime = IntervalMetric();

void GPS::GPSisr(uint8_t c) {
  gps.handle(c);
}

void GPS::PPSisr() {
  gps.UTCsecondStart();
  setSubSecond = true;
}

void GPS::init() {
  gpsPort.begin(9600);  // module's default baud
  delay(100);
  gpsPort.print(F("$PCAS01,4*18\r\n"));  // switch module to 57600
  delay(100);
  gpsPort.end();

  gpsPort.begin(57600);  // match host to new baud

  gpsPort.print(F("$PCAS02,500*1A\r\n"));                        // 2 Hz
  gpsPort.print(F("$PCAS03,1,0,1,1,1,0,1,0,0,0,,,0,0*03\r\n"));  // Enable only the specific sentences we want
  gpsPort.print(F("$PCAS00*01\r\n"));                            // Optional - Persist config to flash

  gpsPort.attachInterrupt(GPS::GPSisr);
  attachInterrupt(digitalPinToInterrupt(GPS_PULSE_PER_SECOND_PIN), PPSisr, RISING);

  dataProcessTime.init(F("GPS Proc"), F("Time to process GPS Data"));
  enabled = true;
}

void GPS::run() {
  if (gps.available()) {
    dataProcessTime.start();
    // Print all the things!
    fix = gps.read();
    if (fix.valid.location) {
      SensorState::latitude = fix.latitudeL();
      SensorState::longitude = fix.longitudeL();
    } else {
      SensorState::latitude = 0;
      SensorState::longitude = 0;
    }
    if (fix.valid.altitude) {
      SensorState::altitude = fix.altitude_cm();
    } else {
      SensorState::altitude = 0;
    }
    if (fix.valid.speed) {
      SensorState::gps_speed = fix.speed_kph();
    } else {
      SensorState::gps_speed = 0;
    }
    if (fix.valid.heading) {
      SensorState::heading = fix.heading();
    } else {
      SensorState::heading = 0;
    }
    if (fix.valid.date && fix.valid.time && setSubSecond) {
      dateTimeCalibrated = true;
    } else {
      dateTimeCalibrated = false;
    }
    if (fix.valid.satellites) {
      SensorState::visible_satellites = gps.sat_count;
      SensorState::fix_satellites = fix.satellites;
    } else {
      SensorState::visible_satellites = 0;
      SensorState::fix_satellites = 0;
    }
    char buf[16];
    sprintf(buf, " / %d", SensorState::visible_satellites);
    SensorState::sat_string = buf;
#ifdef DEBUG_LOGGING
    trace_all(DEBUG_PORT, gps, fix);
#endif
    dataProcessTime.stop();
  }

  if (gps.overrun()) {
    gps.overrun(false);
    Errors::logError(Error::GPS_OVERRUN);
  }
}
