/*
 * ============================================================
 *  Boiler Assistant – Burn Logic Module (v1.3)
 *  ------------------------------------------------------------
 *  File: BurnLogic.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Declares the combustion control interface for the Boiler
 *      Assistant firmware. BurnLogic is responsible for computing
 *      the *raw* fan percentage based on:
 *
 *          • Adaptive combustion mode
 *          • PID mode (Below / Normal / Above profiles)
 *          • Exhaust temperature error
 *          • BOOST override (handled externally in FanControl)
 *
 *      BurnLogic does NOT:
 *          • Apply clamp limits
 *          • Apply deadzone rules
 *          • Output PWM
 *          • Control the damper
 *          • Use BME280 or DS18B20 sensors in v1.3
 *
 *      FanControl.cpp handles all shaping and hardware output.
 *
 *  Notes for Contributors:
 *      • burnlogic_compute() is the ONLY public function that
 *        produces a fan percentage.
 *      • Adaptive and PID internals are exposed only for UI
 *        diagnostics and should not be used elsewhere.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ============================================================
//  Initialization
// ============================================================

/**
 * burnlogic_init()
 * ------------------------------------------------------------
 * Initializes internal state for both Adaptive and PID modes.
 *
 * Notes:
 *      • Must be called once during system startup.
 *      • Resets adaptive tracking variables.
 */
void burnlogic_init();


// ============================================================
//  Raw Fan Percentage Computation
// ============================================================

/**
 * burnlogic_compute()
 * ------------------------------------------------------------
 * Computes the *raw* fan percentage (0–100) based on the
 * currently selected burn logic mode.
 *
 * Parameters:
 *      exhaustF  - Current exhaust temperature (°F)
 *
 * Returns:
 *      int - Raw fan percentage (0–100) BEFORE:
 *              • Clamping
 *              • Deadzone
 *              • BOOST override
 *              • PWM output
 *
 * Notes:
 *      • FanControl.cpp applies all shaping and hardware output.
 */
int burnlogic_compute(double exhaustF);


// ============================================================
//  Adaptive Combustion Internals (v1.3)
//  Exposed for UI diagnostics only.
// ============================================================

/**
 * adaptive_reset()
 * ------------------------------------------------------------
 * Resets adaptive combustion tracking variables.
 *
 * Notes:
 *      • Called when switching modes or resetting diagnostics.
 */
void adaptive_reset();

/**
 * adaptive_update()
 * ------------------------------------------------------------
 * Updates adaptive combustion slope tracking based on the
 * current exhaust temperature.
 *
 * Parameters:
 *      exhaustF  - Current exhaust temperature (°F)
 *
 * Notes:
 *      • Used internally by burnlogic_compute() when in
 *        Adaptive mode.
 */
void adaptive_update(double exhaustF);


// ============================================================
//  PID Mode Internals (v1.3)
// ============================================================

/**
 * pid_compute()
 * ------------------------------------------------------------
 * Computes PID output based on temperature error.
 *
 * Parameters:
 *      errorF  - (setpoint – current exhaust temperature)
 *
 * Returns:
 *      double - PID output (0–100 range expected)
 *
 * Notes:
 *      • Uses the currently selected PID profile (Below/Normal/Above).
 *      • burnlogic_compute() handles profile selection.
 */
double pid_compute(double errorF);
