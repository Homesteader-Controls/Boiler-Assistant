/*
 * ============================================================
 *  Boiler Assistant – Burn Logic Module (v1.3)
 *  ------------------------------------------------------------
 *  File: BurnLogic.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Computes the *raw* fan percentage (0–100) using:
 *
 *          • Adaptive combustion mode
 *          • PID mode (three profiles: BELOW / NORMAL / ABOVE)
 *
 *      BurnLogic does NOT:
 *          • Apply clamp limits
 *          • Apply deadzone rules
 *          • Apply BOOST override
 *          • Output PWM
 *          • Control the damper
 *
 *      These behaviors are handled in FanControl.cpp.
 *
 *      Adaptive tracking functions are implemented in Adaptive.cpp.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "BurnLogic.h"
#include "SystemState.h"
#include "Adaptive.h"
#include <Arduino.h>


// ============================================================
//  SECTION: Internal PID State
// ============================================================

/**
 * pidIntegral   - Accumulated integral term
 * pidLastError  - Previous loop error
 * pidLastTime   - Timestamp of last PID update
 */
static double pidIntegral = 0;
static double pidLastError = 0;
static unsigned long pidLastTime = 0;


// ============================================================
//  burnlogic_init()
// ============================================================

/**
 * burnlogic_init()
 * ------------------------------------------------------------
 * Initializes internal state for both Adaptive and PID modes.
 *
 * Behavior:
 *      • Resets PID integrator and timing
 *      • Resets adaptive tracking (via Adaptive.cpp)
 *
 * Notes:
 *      • Must be called once during system startup.
 */
void burnlogic_init() {
    pidIntegral = 0;
    pidLastError = 0;
    pidLastTime = millis();

    adaptive_reset();   // correctly calls Adaptive.cpp
}


// ============================================================
//  pid_compute()
// ============================================================

/**
 * pid_compute()
 * ------------------------------------------------------------
 * Computes PID output based on temperature error.
 *
 * Parameters:
 *      errorF  - (setpoint – current exhaust temperature)
 *
 * Behavior:
 *      • Selects PID profile based on error magnitude:
 *            error < -10 → BELOW profile
 *            error >  10 → ABOVE profile
 *            otherwise   → NORMAL profile
 *
 *      • Computes:
 *            P = kp * error
 *            I = ki * ∫error dt
 *            D = kd * d(error)/dt
 *
 *      • Clamps output to 0–100%
 *
 * Returns:
 *      double - PID output (0–100)
 */
double pid_compute(double errorF) {

    unsigned long now = millis();
    double dt = (now - pidLastTime) / 1000.0;
    if (dt <= 0) dt = 0.001;

    pidLastTime = now;

    // Select PID profile based on error sign
    float kp, ki, kd;

    if (errorF < -10) {
        kp = pidBelowKp;
        ki = pidBelowKi;
        kd = pidBelowKd;
    }
    else if (errorF > 10) {
        kp = pidAboveKp;
        ki = pidAboveKi;
        kd = pidAboveKd;
    }
    else {
        kp = pidNormKp;
        ki = pidNormKi;
        kd = pidNormKd;
    }

    // PID terms
    pidIntegral += errorF * dt;
    double derivative = (errorF - pidLastError) / dt;
    pidLastError = errorF;

    double output = (kp * errorF) + (ki * pidIntegral) + (kd * derivative);

    // Clamp to 0–100%
    if (output < 0) output = 0;
    if (output > 100) output = 100;

    return output;
}


// ============================================================
//  burnlogic_compute()
// ============================================================

/**
 * burnlogic_compute()
 * ------------------------------------------------------------
 * Computes the *raw* fan percentage (0–100) based on the
 * selected burn logic mode.
 *
 * Parameters:
 *      exhaustF  - Current exhaust temperature (°F)
 *
 * Behavior:
 *      • Computes error = setpoint – exhaustF
 *      • Updates adaptive tracking (Adaptive.cpp)
 *      • If Adaptive mode:
 *            - Uses proportional term + slope adjustment
 *      • If PID mode:
 *            - Uses pid_compute()
 *
 * Returns:
 *      int - Raw fan percentage (0–100), BEFORE:
 *              • BOOST override
 *              • Clamp limits
 *              • Deadzone rules
 *              • PWM output
 *
 * Notes:
 *      • FanControl.cpp applies all shaping and hardware output.
 */
int burnlogic_compute(double exhaustF) {

    // --------------------------------------------------------
    // Compute error
    // --------------------------------------------------------
    double errorF = exhaustSetpoint - exhaustF;

    // --------------------------------------------------------
    // Update adaptive tracking (implemented in Adaptive.cpp)
    // --------------------------------------------------------
    adaptive_update(exhaustF);

    // --------------------------------------------------------
    // Mode Selection
    // --------------------------------------------------------
    if (burnLogicMode == 0) {
        // ================================================
        // ADAPTIVE MODE
        // ================================================
        double base = errorF * 0.8;     // proportional
        double slopeAdj = adaptiveSlope * lastRate;

        double out = base + slopeAdj;

        // Clamp to 0–100%
        if (out < 0) out = 0;
        if (out > 100) out = 100;

        return (int)(out + 0.5);
    }
    else {
        // ================================================
        // PID MODE
        // ================================================
        return (int)(pid_compute(errorF) + 0.5);
    }
}
