//! FLIPSKY 75100 ESC Communications
#include <Arduino.h>

void setup() {
}

void loop() {
}

// TODO apparently 115200 baud on vesc to arduino isn't reliable, use 250000. (see timings table for 16mhz clock https://wormfood.net/avrbaudcalc.php?bitrate=300%2C600%2C1200%2C2400%2C4800%2C9600%2C14.4k%2C19.2k%2C28.8k%2C38.4k%2C57.6k%2C76.8k%2C115.2k%2C230.4k%2C250k%2C460.8k%2C.5m%2C921.6k%2C1m&clock=16&databits=8)
// TODO Get VESC errors via serial
// TODO do Estimated completion battery percentage (based on expected AH in settings and current usage & time)
