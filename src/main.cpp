// #include <Arduino.h>
// #include <NMEAGPS.h>
// #include <GPSport.h>

// //------------------------------------------------------------
// // Check configuration

// #ifndef NMEAGPS_RECOGNIZE_ALL
// #error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
// #endif

// #ifdef NMEAGPS_INTERRUPT_PROCESSING
// #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
// #endif

// static NMEAGPS gps;            // This parses received characters
// static uint32_t last_rx = 0L;  // The last millis() time a character was
//                                // received from GPS.  This is used to
//                                // determine when the GPS quiet time begins.

// //------------------------------------------------------------

// static NMEAGPS::nmea_msg_t last_sentence_in_interval = NMEAGPS::NMEA_UNKNOWN;
// static uint8_t prev_sentence_count = 0;
// static uint8_t sentence_count = 0;
// static const uint8_t MAX_SENTENCES = 20;  // per second
// static NMEAGPS::nmea_msg_t sentences[MAX_SENTENCES];

// static void recordSentenceTypes() {
//   // Always save the last sentence, even if we're full
//   sentences[sentence_count] = gps.nmeaMessage;
//   if (sentence_count < MAX_SENTENCES - 1)
//     sentence_count++;

// }  // recordSentenceTypes

// //-----------------------------------------------------------

// static void printSentenceOrder() {
//   DEBUG_PORT.println(F("\nSentence order in each 1-second interval:"));

//   for (uint8_t i = 0; i < sentence_count; i++) {
//     DEBUG_PORT.print(F("  "));
//     DEBUG_PORT.println(gps.string_for(sentences[i]));
//   }

//   if (sentences[sentence_count - 1] == LAST_SENTENCE_IN_INTERVAL) {
//     DEBUG_PORT.print(F("\nSUCCESS: LAST_SENTENCE_IN_INTERVAL is correctly set to NMEAGPS::NMEA_"));
//   } else {
//     DEBUG_PORT.print(F("\nERROR: LAST_SENTENCE_IN_INTERVAL is incorrectly set to NMEAGPS::NMEA_"));
//     DEBUG_PORT.print(gps.string_for(LAST_SENTENCE_IN_INTERVAL));
//     DEBUG_PORT.print(F(
//         "!\n  You must change this line in NMEAGPS_cfg.h:\n"
//         "     #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_"));
//   }
//   DEBUG_PORT.println(gps.string_for(sentences[sentence_count - 1]));
//   DEBUG_PORT.println();

// }  // printSentenceOrder

// //------------------------------------

// static void GPSloop() {
//   while (gpsPort.available()) {
//     last_rx = millis();

//     if (gps.decode(gpsPort.read()) == NMEAGPS::DECODE_COMPLETED) {
//       if (last_sentence_in_interval == NMEAGPS::NMEA_UNKNOWN) {
//         // Still building the list
//         recordSentenceTypes();
//         DEBUG_PORT.print('.');
//       }
//     }
//   }
// }  // GPSloop

// //----------------------------------------------------------------
// //  Determine whether the GPS quiet time has started.
// //
// //  This is only needed in the example programs, which must work
// //  for *any* GPS device.
// //
// //  It also "pretends" to have a quiet time once per
// //  second so that some debug messages are emitted.  This allows
// //  beginners to see whether the GPS device is correctly
// //  connected and functioning.

// static bool quietTimeStarted() {
//   uint32_t current_ms = millis();
//   uint32_t ms_since_last_rx = current_ms - last_rx;

//   if (ms_since_last_rx > 5) {
//     // The GPS device has not sent any characters for at least 5ms.
//     //   See if we've been getting chars sometime during the last second.
//     //   If not, the GPS may not be working or connected properly.

//     bool getting_chars = (ms_since_last_rx < 500UL);
//     static uint32_t last_quiet_time = 0UL;
//     bool just_went_quiet =
//         (((int32_t)(last_rx - last_quiet_time)) > 10L);
//     bool next_quiet_time =
//         ((current_ms - last_quiet_time) >= 1000UL);

//     if ((getting_chars && just_went_quiet) ||
//         (!getting_chars && next_quiet_time)) {
//       last_quiet_time = current_ms;  // Remember for next loop

//       //  If we're not getting good data, make some suggestions.

//       bool allDone = false;

//       if (!getting_chars) {
//         DEBUG_PORT.println(F("\nCheck GPS device and/or connections.  No characters received.\n"));

//         allDone = true;

//       } else if (sentence_count == 0) {
//         DEBUG_PORT.println(F(
//             "No valid sentences, but characters are being received.\n"
//             "Check baud rate or device protocol configuration.\n"));

//         allDone = true;
//       }

//       if (allDone) {
//         DEBUG_PORT.print(F("\nEND.\n"));
//         for (;;);  // hang!
//       }

//       // No problem, just a real GPS quiet time.
//       return true;
//     }
//   }

//   return false;

// }  // quietTimeStarted

// //----------------------------------------------------------------
// //  Figure out what sentence the GPS device sends
// //  as the last sentence in each 1-second interval.

// static void watchForLastSentence() {
//   if (quietTimeStarted()) {
//     if (prev_sentence_count != sentence_count) {
//       // We have NOT received two full intervals of sentences with
//       //    the same number of sentences in each interval.  Start
//       //    recording again.
//       prev_sentence_count = sentence_count;
//       sentence_count = 0;

//     } else if (sentence_count > 0) {
//       // Got it!
//       last_sentence_in_interval = sentences[sentence_count - 1];
//     }
//   }

// }  // watchForLastSentence

// //--------------------------

// void setup() {
//   DEBUG_PORT.begin(500000);
//   delay(3000);

//   DEBUG_PORT.print(F("NMEAorder.INO: started\n"));
//   DEBUG_PORT.print(F("fix object size = "));
//   DEBUG_PORT.println(sizeof(gps.fix()));
//   DEBUG_PORT.print(F("NMEAGPS object size = "));
//   DEBUG_PORT.println(sizeof(gps));
//   DEBUG_PORT.println(F("Looking for GPS device on " GPS_PORT_NAME));
//   DEBUG_PORT.flush();

//   gpsPort.begin(115200);
// }

// //--------------------------

// void loop() {
//   GPSloop();

//   if (last_sentence_in_interval == NMEAGPS::NMEA_UNKNOWN)
//     watchForLastSentence();
//   else {
//     printSentenceOrder();
//     for (;;);  // All done!
//   }
// }



#include "main.h"

#include "hardware.h"
#include "modules/GPS.h"
#include "modules/ath20.h"
#include "modules/bmp280.h"
#include "modules/display.h"
#include "modules/imu.h"
#include "modules/rtc.h"
#include "modules/speed.h"
#include "modules/temps.h"
#include "modules/vesc.h"
#include "settings.h"
#include "state.h"
#include "utils/errors.h"
#include "utils/scheduler.h"

AppState currentState = AppState::IDLE;
IntervalMetric mainLoopTime = IntervalMetric();

extern char* __brkval;
extern char __heap_start;

int freeMemory() {
  char top;
  return __brkval ? &top - __brkval : &top - &__heap_start;
}

void logData();
void reportRam() {
  Serial.print(F("Free RAM = "));  // F function does the same and is now a built in library, in IDE > 1.0.0
  Serial.println(freeMemory());    // print how much RAM is available in bytes.
}

ScheduledTask tasks[] = {
    // ScheduledTask(100, 0, readGPS),
    // ScheduledTask(500, 0, displayBattStats),
    ScheduledTask(200, 0, reportRam),
    ScheduledTask(100, 5, SPEED::update),
    ScheduledTask(250, 10, VESC::update),
    ScheduledTask(500, 100, ATH::update),
    ScheduledTask(500, 200, BMP::update),
    ScheduledTask(500, 300, TEMPS::update),
    ScheduledTask(1000, 15, RTC::update),
    // ScheduledTask(500, 400, logData),
};

void setup() {
  Errors::init();

  //! Setup SD class first before errors class

  HARDWARE::init();

  SETTINGS::init();

  SPEED::configure(SETTINGS::getSettings().speed_sensor_wheel_circumference, SETTINGS::getSettings().speed_sensor_pulses_per_revolution);

  mainLoopTime.init(F("Main Loop"), F("Interval measurement of main loop"));

  Scheduler::start(tasks);
}

void loop() {
  mainLoopTime.start();  // TODO maybe have a minimum threshold on this? So a busy-wait isn't included
  Scheduler::runTasks();
  HARDWARE::run();
  TEMPS::run();  // Runs once conversion is complete
  IMU::run();    // Internally scheduled via data ready interrupt
  GPS::run();   // Runs when new fix available
  Display::run();

  switch (currentState) {
    case AppState::IDLE:
      break;
    case AppState::RACE:
      break;
    case AppState::DEBUG:
      break;
  }
  // for dev track framerate of display updates for different values?
  mainLoopTime.stop();
}
