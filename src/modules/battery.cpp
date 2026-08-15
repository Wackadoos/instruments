#include "battery.h"

void BatteryEstimator::init(float totalDischargedWh = 0, float totalChargedWh = 0) {
  totalDischargedWh_calibration = totalDischargedWh;
  totalChargedWh_calibration = totalChargedWh;
}

void BatteryEstimator::updateEWMA(float instCurrentA) {
  unsigned long now = micros();
  float dt_h = (lastUpdateMicros == 0) ? 0.0 : (now - lastUpdateMicros) / 3600000000.0;  // us -> h
  lastUpdateMicros = now;

  float I_inst = max(instCurrentA, 0.0f);
  if (dt_h > 0) {  // first call only seeds the clock
    float alpha = 1.0 - exp(-dt_h / TAU_HOURS);
    I_avg += alpha * (I_inst - I_avg);
  }

  // Push window sample (raw, so regen shows up as negative)
  windowCurrentSumA -= windowCurrent[windowHead];
  windowCurrentSumA += instCurrentA;
  windowCurrent[windowHead] = instCurrentA;
  windowHead = (windowHead + 1) % WINDOW_MAX_SAMPLES;
  if (windowCount < WINDOW_MAX_SAMPLES) windowCount++;
}

float BatteryEstimator::estimateSOC(float totalDischargedWh, float totalChargedWh, float ambientTempC) {
  // --- Wh -> Ah (nominal voltage approximation) ---
  float totalDischargedAh = (totalDischargedWh + totalDischargedWh_calibration) / NOMINAL_VOLTAGE;
  float totalChargedAh = (totalChargedWh + totalChargedWh_calibration) / NOMINAL_VOLTAGE;

  // --- Temperature derate ---
  float tempFactor = 1.0 + TEMP_COEFF * (ambientTempC - 25.0);
  tempFactor = constrain(tempFactor, 0.5, 1.15);

  // --- Peukert rate derate (uses whatever I_avg currently is) ---
  float I_eff = max(I_avg, I_RATED * 0.2f);
  float rateFactor = pow(I_RATED / I_eff, PEUKERT_K - 1.0);
  rateFactor = constrain(rateFactor, 0.3, 1.15);

  // --- Effective capacity ceiling ---
  C_eff = RATED_CAPACITY_AH * tempFactor * rateFactor;

  // --- Net charge ledger ---
  float net_used_Ah = totalDischargedAh - CHARGE_EFF * totalChargedAh;
  net_used_Ah = max(net_used_Ah, 0.0f);

  // --- SOC ---
  float remaining_Ah = C_eff - net_used_Ah;
  float soc = 100.0 * remaining_Ah / C_eff;
  currentSOCPercent = constrain(soc, -99.0f, 100.0f);
  return currentSOCPercent;
}

// Time remaining estimate, based on the 1-min rolling average current and current effective SOC
float BatteryEstimator::estimateTimeRemainingMinutes() {
  if (windowCount == 0) return 0;  // no samples yet (defensive)
  float avgCurrent = windowCurrentSumA / windowCount;
  if (avgCurrent <= 0.0f) return 999;  // parked / regen: not net-draining
  float remaining_Ah = (currentSOCPercent / 100.0) * C_eff;
  return constrain((remaining_Ah / avgCurrent) * 60.0f, 0, 999);  // Ah / A * 60 = minutes
}
