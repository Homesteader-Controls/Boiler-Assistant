/*
 * ============================================================
 *  Boiler Assistant – Adaptive Combustion Module (v1.3)
 *  ------------------------------------------------------------
 *  File: Adaptive.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Declares the adaptive combustion interface for v1.3.
 *
 *      Adaptive mode tracks:
 *          • Exhaust temperature slope
 *          • dT/ds (rate of change)
 *
 *      These values are used by:
 *          • Adaptive burn logic (BurnLogic.cpp)
 *          • UI diagnostics screen (UI.cpp)
 *
 *      Notes:
 *          • v1.3 does NOT use BME280 or DS18B20 sensors.
 *          • All adaptive math and state updates are implemented
 *            in Adaptive.cpp.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ============================================================
//  Adaptive State Reset
// ============================================================

/**
 * adaptive_reset()
 * ------------------------------------------------------------
 * Resets all adaptive combustion tracking variables.
 *
 * Behavior:
 *      • Clears slope history
 *      • Clears last temperature sample
 *      • Resets timing state
 *
 * Notes:
 *      • Called during system startup and when switching modes.
 */
void adaptive_reset();


// ============================================================
//  Adaptive Tracking Update
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
 *      • Computes dT/ds (rate of change)
 *      • Updates adaptiveSlope (long‑term trend)
 *      • Stores last sample + timestamp
 *
 * Notes:
 *      • Called once per loop by burnlogic_compute().
 *      • UI diagnostics screen displays adaptiveSlope + lastRate.
 */
void adaptive_update(double exhaustF);
