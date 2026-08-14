#include "GPS.h"

#include "GPSport.h"
#include "Streamers.h"
#include "state.h"

NMEAGPS GPS::gps = NMEAGPS();
IntervalMetric GPS::dataProcessTime = IntervalMetric();

void GPS::GPSisr(uint8_t c) {
  gps.handle(c);
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

  dataProcessTime.init(F("GPS Proc"), F("Time to process GPS Data"));
  enabled = true;
}

void GPS::run() {
  if (gps.available()) {
    dataProcessTime.start();
    // Print all the things!
    auto fix = gps.read();
    trace_all(DEBUG_PORT, gps, fix);
    SensorState::visible_satellites = gps.sat_count;
    SensorState::fix_satellites = fix.satellites;
    char buf[16];
    sprintf(buf, "%d / %d", fix.satellites, gps.sat_count);
    SensorState::sat_string = buf;
    // Logging::logDebug(F("ATH Humidity: "), humidity.relative_humidity);
    dataProcessTime.stop();
  }

  if (gps.overrun()) {
    gps.overrun(false);
    DEBUG_PORT.println(F("DATA OVERRUN: took too long to print GPS data!"));
  }
}
