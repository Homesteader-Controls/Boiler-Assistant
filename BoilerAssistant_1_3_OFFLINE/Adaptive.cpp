/*
 * ============================================================
 *  Boiler Assistant – Adaptive Combustion Module (v1.3)
 *  ------------------------------------------------------------
 *  File: Adaptive.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Implements the adaptive combustion tracking functions.
 *      Adaptive mode tracks:
 *
 *          • Exhaust temperature slope (long‑term trend)
 *          • dT/ds (rate of change)
 *
 *      These values are used by:
 *          • Adaptive burn logic (BurnLogic.cpp)
 *          • UI diagnostics screen (UI.cpp)
 *
 *      Notes:
 *          • v1.3 uses ONLY exhaust temperature for adaptive logic.
 *          • No BME280 or DS18B20 data is used.
 *          • BurnLogic.cpp performs the actual adaptive fan math.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "Adaptive.h"
#include "SystemState.h"
#include <Arduino.h>


// ============================================================
//  adaptive_reset()
// ============================================================

/**
 * adaptive_reset()
 * ------------------------------------------------------------
 * Resets all adaptive combustion tracking variables.
 *
 * Behavior:
 *      • Clears lastRate (dT/ds)
 *      • Clears lastT (previous exhaust temperature)
 *      • Resets lastTtime to current millis()
 *
 * Notes:
 *      • Called during system startup and when switching modes.
 */
void adaptive_reset() {
    lastRate = 0;
    lastT = 0;
    lastTtime = millis();
}


// ============================================================
//  adaptive_update()
// ============================================================

/**
 * adaptive_update()
 * ------------------------------------------------------------
 * Updates adaptive combustion tracking based on the current
 * exhaust temperature.
 *
 * Parameters:
 *      exhaustF  - Current exhaust temperature (°F)
 *
 * Behavior:
 *      • Computes dt (seconds since last update)
 *      • Computes dT (temperature delta)
 *      • Computes lastRate = dT/dt (°F per second)
 *      • Updates lastT and lastTtime
 *
 * Notes:
 *      • Called once per loop by burnlogic_compute().
 *      • adaptiveSlope is updated in BurnLogic.cpp using lastRate.
 */
void adaptive_update(double exhaustF) {

    unsigned long now = millis();
    double dt = (now - lastTtime) / 1000.0;

    if (dt <= 0) dt = 0.001;

    double dT = exhaustF - lastT;
    lastRate = dT / dt;

    lastT = exhaustF;
    lastTtime = now;
}
