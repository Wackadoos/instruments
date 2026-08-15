#pragma once
#include <Arduino.h>

class BatteryEstimator {
 private:
  // ---- Tunable constants ----
  static constexpr float RATED_CAPACITY_AH = 36.0;
  static constexpr float RATED_HOURS = 20.0;
  static constexpr float NOMINAL_VOLTAGE = 12.0;  // for Wh -> Ah conversion
  static constexpr float PEUKERT_K = 1.24;
  static constexpr float CHARGE_EFF = 0.92;
  static constexpr float TEMP_COEFF = 0.006;                         // 0.6%/°C, pinned at 25°C
  static constexpr float TAU_HOURS = 20.0 / 60.0;                    // ~20 min recovery constant
  static constexpr float I_RATED = RATED_CAPACITY_AH / RATED_HOURS;  // 1.8A

  // ---- Persistent state ----
  static inline unsigned long lastUpdateMicros = 0;

  // ---- Class calculated stored stats ----
  static inline float currentSOCPercent = 100;    // Start assuming 100% charge
  static inline float C_eff = RATED_CAPACITY_AH;  // Start assuming 100% efficiency

  // ---- Rolling window ----
  static constexpr uint16_t WINDOW_MAX_SAMPLES = 60;  // 60 samples @ 1s cadence = 1-min window
  static inline float windowCurrent[WINDOW_MAX_SAMPLES];
  static inline uint16_t windowHead = 0;
  static inline uint16_t windowCount = 0;
  static inline float windowCurrentSumA = 0.0f;

  // Passover calibration
  static inline float totalDischargedWh_calibration = 0;
  static inline float totalChargedWh_calibration = 0;

 public:
  // ---- Persistent state ----
  static inline float I_avg = I_RATED;  // seeded at rated rate

  static void init(float totalDischargedWh, float totalChargedWh);

  // ---- FAST: call as often as you like (e.g. every current sample) ----
  static void updateEWMA(float instCurrentA);

  // ---- SLOW: call whenever you want a fresh SOC% (e.g. display refresh) ----
  static float estimateSOC(float totalDischargedWh, float totalChargedWh, float ambientTempC);

  // Time remaining estimate, based on 1-min rolling average (60 samples @ 1s)
  static float estimateTimeRemainingMinutes();
};
