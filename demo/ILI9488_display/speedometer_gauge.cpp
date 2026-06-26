// =============================================================
//  Speedometer Gauge Demo — Arduino_GFX + ILI9488 on Mega
//  Libraries needed:
//    - GFX Library for Arduino (moononournation/Arduino_GFX)
//    - XPT2046_Touchscreen (PaulStoffregen) if using touch
// =============================================================

#include <Arduino_GFX_Library.h>
#include <math.h>  // for sin(), cos(), M_PI

// -------------------------------------------------------------
//  Pin definitions — adjust to your wiring on the Mega
// -------------------------------------------------------------
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
// Hardware SPI pins on Mega: MOSI=51, MISO=50, SCK=52 (fixed)

// -------------------------------------------------------------
//  Arduino_GFX setup — two objects, one for bus, one for display
//  Arduino_HWSPI(DC, CS)  — uses hardware SPI automatically
//  Arduino_ILI9488(bus, RST, rotation, ips)
//    rotation: 0=portrait, 1=landscape, 2=portrait flip, 3=landscape flip
// -------------------------------------------------------------
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);
Arduino_GFX    *gfx = new Arduino_ILI9488(bus, TFT_RST, 1 /*landscape*/);

// -------------------------------------------------------------
//  Color helpers
//  Arduino_GFX uses 16-bit RGB565 color values.
//  The macro RGB565(r,g,b) packs 5 red bits, 6 green bits, 5 blue bits.
//  Common colors are pre-defined as constants.
// -------------------------------------------------------------
#define COL_BG        0x0000  // BLACK
#define COL_FACE      0x1082  // Dark grey (custom: R=2,G=4,B=2 in 565)
#define COL_RIM       0x8410  // Mid grey
#define COL_RED_ZONE  0xF800  // RED
#define COL_YELLOW    0xFFE0  // YELLOW
#define COL_GREEN     0x07E0  // GREEN
#define COL_WHITE     0xFFFF
#define COL_NEEDLE    0xF800  // RED needle
#define COL_HUB       0xC618  // Light grey hub

// -------------------------------------------------------------
//  Gauge geometry — all derived from centre + radius
//  The gauge is centred on the screen, occupying most of the height.
//  The arc sweeps from 225° to -45° (i.e. 270° total sweep),
//  so 0 km/h is bottom-left and max is bottom-right.
// -------------------------------------------------------------
#define SCREEN_W     480
#define SCREEN_H     320
#define CX           240        // centre X
#define CY           185        // centre Y (shifted down to leave room for label)
#define RADIUS       145        // outer radius of gauge face
#define ARC_OUTER    140        // outer edge of coloured arc band
#define ARC_INNER    115        // inner edge of coloured arc band
#define TICK_OUTER   130        // outer end of tick marks
#define TICK_INNER   110        // inner end of major ticks
#define TICK_INNER_M 118        // inner end of minor ticks
#define LABEL_R       88        // radius at which speed numbers are drawn
#define NEEDLE_LEN   120        // how far the needle reaches
#define NEEDLE_TAIL   25        // how far needle goes behind the pivot

// Speed range
#define SPEED_MIN      0
#define SPEED_MAX    200

// Angle mapping: 0 km/h = 225°, 200 km/h = -45° (= 315°)
// We work in degrees throughout; trig converts to radians as needed.
#define ANGLE_START  225.0f   // degrees, measured clockwise from 3-o'clock
#define ANGLE_END    315.0f   // same convention, goes via 360/0


// -------------------------------------------------------------
//  Utility: degrees → radians (Arduino has no built-in degToRad)
// -------------------------------------------------------------
float degToRad(float deg) {
  return deg * M_PI / 180.0f;
}

// -------------------------------------------------------------
//  Map a speed value to an angle (degrees, clockwise from east)
// -------------------------------------------------------------
float speedToAngle(float speed) {
  // Clamp
  speed = constrain(speed, SPEED_MIN, SPEED_MAX);
  // Linear interpolation across the 270° sweep
  float fraction = (speed - SPEED_MIN) / (float)(SPEED_MAX - SPEED_MIN);
  // Going clockwise: start at ANGLE_START, sweep 270° to ANGLE_END
  // Angles cross 360° boundary so we handle wrap-around
  float sweep = 270.0f;  // total arc in degrees
  return ANGLE_START - fraction * sweep;  // decreasing = clockwise on screen
}

// -------------------------------------------------------------
//  Utility: draw a line from centre-offset outward to r at angle
//  Angles are in standard math convention (0=right, CCW positive)
//  but we negate Y because screen Y grows downward.
// -------------------------------------------------------------
void drawRadialLine(int16_t cx, int16_t cy,
                    int16_t r_inner, int16_t r_outer,
                    float angleDeg, uint16_t color, uint8_t thickness = 1) {
  float rad = degToRad(angleDeg);
  float cosA = cos(rad);
  float sinA = -sin(rad);  // negate because screen Y is inverted

  int16_t x0 = cx + r_inner * cosA;
  int16_t y0 = cy + r_inner * sinA;
  int16_t x1 = cx + r_outer * cosA;
  int16_t y1 = cy + r_outer * sinA;

  gfx->drawLine(x0, y0, x1, y1, color);

  // For thicker lines, draw offset copies (simple but effective on AVR)
  if (thickness > 1) {
    gfx->drawLine(x0+1, y0,   x1+1, y1,   color);
    gfx->drawLine(x0,   y0+1, x1,   y1+1, color);
  }
  if (thickness > 2) {
    gfx->drawLine(x0-1, y0,   x1-1, y1,   color);
    gfx->drawLine(x0,   y0-1, x1,   y1-1, color);
  }
}

// -------------------------------------------------------------
//  Draw the static gauge face (called once in setup)
// -------------------------------------------------------------
void drawGaugeFace() {

  // --- Background ---
  gfx->fillScreen(COL_BG);

  // --- Gauge face circle ---
  // fillCircle(x, y, radius, color)
  gfx->fillCircle(CX, CY, RADIUS, COL_FACE);

  // --- Rim ring (draw concentric circles for a thick rim effect) ---
  for (int r = RADIUS; r > RADIUS - 6; r--) {
    gfx->drawCircle(CX, CY, r, COL_RIM);
  }

  // --- Coloured arc band (green / yellow / red zones) ---
  //
  //  fillArc(x, y, r_outer, r_inner, angle_start, angle_end, color)
  //  Angles in Arduino_GFX fillArc are in DEGREES,
  //  measured CLOCKWISE from the top (12 o'clock = 0°).
  //  So we need to convert our math angles to this convention:
  //    screen_angle = 90 - math_angle  (then mod 360 if needed)
  //
  //  Our gauge:
  //    0 km/h  → math 225° → screen 90-225 = -135 → +225° screen
  //  200 km/h  → math -45° → screen 90-(-45) = 135° screen
  //
  //  Green:  0–100  → screen 225° to 315°   (going clockwise = smaller screen angle... wait)
  //
  //  Actually the simplest approach: test with a helper.
  //  Arduino_GFX fillArc treats 0° = right (east), going clockwise.
  //  That matches our math convention if we flip Y. Let's use that directly.
  //
  //  Zone extents in our math-angle system (0=right, going clockwise = decreasing angle):
  //    0  km/h → 225°
  //    80 km/h → 225 - (80/200)*270 = 225 - 108 = 117°
  //   140 km/h → 225 - (140/200)*270 = 225 - 189 = 36°
  //   200 km/h → 225 - 270 = -45° = 315°
  //
  //  fillArc(cx, cy, r_outer, r_inner, start_angle, end_angle, color)
  //  where angles go COUNTER-clockwise from east in Arduino_GFX.

  // Green zone: 0–80 km/h  (225° down to 117°, CCW = from 117 to 225)
  gfx->fillArc(CX, CY, ARC_OUTER, ARC_INNER, 117.0f, 225.0f, COL_GREEN);

  // Yellow zone: 80–140 km/h  (117° down to 36°)
  gfx->fillArc(CX, CY, ARC_OUTER, ARC_INNER, 36.0f, 117.0f, COL_YELLOW);

  // Red zone: 140–200 km/h  (36° down to -45° = crossing zero)
  // Split at 0° to handle the wrap-around
  gfx->fillArc(CX, CY, ARC_OUTER, ARC_INNER, 0.0f,   36.0f, COL_RED_ZONE);
  gfx->fillArc(CX, CY, ARC_OUTER, ARC_INNER, 315.0f, 360.0f, COL_RED_ZONE);

  // --- Tick marks ---
  // Major ticks every 20 km/h, minor ticks every 10 km/h
  for (int spd = SPEED_MIN; spd <= SPEED_MAX; spd += 10) {
    float angle = speedToAngle(spd);
    bool isMajor = (spd % 20 == 0);

    drawRadialLine(CX, CY,
                   isMajor ? TICK_INNER : TICK_INNER_M,
                   TICK_OUTER,
                   angle,
                   COL_WHITE,
                   isMajor ? 2 : 1);
  }

  // --- Speed labels (every 40 km/h to avoid crowding) ---
  //  setTextSize(n) sets character size: 1 = 6x8px, 2 = 12x16px, etc.
  //  setTextColor(fg, bg) — use a bg matching the face so text is clean
  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE, COL_FACE);

  for (int spd = 0; spd <= SPEED_MAX; spd += 40) {
    float angle = speedToAngle(spd);
    float rad   = degToRad(angle);
    // Position text centre approximately at LABEL_R
    int16_t tx = CX + LABEL_R * cos(rad);
    int16_t ty = CY - LABEL_R * sin(rad);  // negate Y for screen coords

    // Crude centering: each char is 12px wide at size 2
    String label = String(spd);
    int16_t offsetX = (label.length() * 12) / 2;
    int16_t offsetY = 8;  // half of 16px char height

    gfx->setCursor(tx - offsetX, ty - offsetY);
    gfx->print(label);
  }

  // --- "km/h" unit label at bottom of face ---
  gfx->setTextSize(2);
  gfx->setTextColor(COL_WHITE, COL_FACE);
  gfx->setCursor(CX - 28, CY + 55);
  gfx->print("km/h");

  // --- Title at top of screen ---
  gfx->setTextSize(3);
  gfx->setTextColor(COL_WHITE, COL_BG);
  gfx->setCursor(160, 8);
  gfx->print("SPEED");
}

// -------------------------------------------------------------
//  Draw (or erase) the needle for a given speed
//  Call with COL_NEEDLE to draw, COL_FACE to erase.
//
//  The needle is a triangle for a realistic look:
//    - tip at NEEDLE_LEN from centre
//    - base is two points at NEEDLE_TAIL behind centre,
//      offset slightly perpendicular for width
// -------------------------------------------------------------
void drawNeedle(float speed, uint16_t color) {
  float angle = speedToAngle(speed);
  float rad   = degToRad(angle);
  float cosA  =  cos(rad);
  float sinA  = -sin(rad);  // screen Y flip

  // Perpendicular direction (for needle width at base)
  float perpCos = -sinA;
  float perpSin =  cosA;
  int   halfBase = 5;  // half-width of needle base in pixels

  // Tip point
  int16_t tipX = CX + NEEDLE_LEN * cosA;
  int16_t tipY = CY + NEEDLE_LEN * sinA;

  // Two base points (behind pivot, spread perpendicular)
  int16_t b1x = CX - NEEDLE_TAIL * cosA + halfBase * perpCos;
  int16_t b1y = CY - NEEDLE_TAIL * sinA + halfBase * perpSin;
  int16_t b2x = CX - NEEDLE_TAIL * cosA - halfBase * perpCos;
  int16_t b2y = CY - NEEDLE_TAIL * sinA - halfBase * perpSin;

  // fillTriangle(x0,y0, x1,y1, x2,y2, color)
  gfx->fillTriangle(tipX, tipY, b1x, b1y, b2x, b2y, color);

  // Hub cap (small filled circle at pivot, drawn on top)
  gfx->fillCircle(CX, CY, 10, COL_HUB);
  gfx->drawCircle(CX, CY, 10, COL_WHITE);
}

// -------------------------------------------------------------
//  Update the digital speed readout at the bottom
// -------------------------------------------------------------
void drawSpeedReadout(float speed) {
  gfx->setTextSize(4);
  gfx->setTextColor(COL_WHITE, COL_BG);
  // Fixed-width: always print 3 digits so old value is overwritten cleanly
  char buf[8];
  snprintf(buf, sizeof(buf), "%3d", (int)speed);
  gfx->setCursor(200, 270);
  gfx->print(buf);
}

// -------------------------------------------------------------
//  Setup
// -------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // gfx->begin() initialises the display and clears it.
  // Optionally pass a desired SPI frequency: gfx->begin(20000000UL)
  if (!gfx->begin()) {
    Serial.println("Display init failed!");
    while (1);
  }

  drawGaugeFace();
  drawNeedle(0, COL_NEEDLE);
  drawSpeedReadout(0);
}

// -------------------------------------------------------------
//  Loop — animate the needle sweeping up and back down
//  In a real project you'd replace `simulatedSpeed` with a
//  value from a sensor or CAN bus.
// -------------------------------------------------------------
float currentSpeed = 0;
float targetSpeed  = 0;
bool  increasing   = true;

void loop() {
  // Simulate acceleration / deceleration
  if (increasing) {
    targetSpeed += 1.5f;
    if (targetSpeed >= SPEED_MAX) increasing = false;
  } else {
    targetSpeed -= 1.5f;
    if (targetSpeed <= 0) increasing = true;
  }

  // Smooth the needle: move current 20% toward target each frame
  // This gives a natural damped-needle feel without any delay()
  float diff = targetSpeed - currentSpeed;
  currentSpeed += diff * 0.2f;

  // Erase old needle by redrawing in face colour, then draw new
  drawNeedle(currentSpeed, COL_FACE);
  drawNeedle(currentSpeed, COL_NEEDLE);
  drawSpeedReadout(currentSpeed);

  // Small delay to control animation speed (~30 fps target)
  delay(33);
}
