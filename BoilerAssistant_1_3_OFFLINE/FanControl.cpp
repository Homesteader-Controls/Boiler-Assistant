/*
 * ============================================================
 *  Boiler Assistant – Fan Control Module (v1.3)
 *  ------------------------------------------------------------
 *  File: FanControl.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Applies all post‑processing to the raw fan percentage:
 *
 *          • BOOST override
 *          • Clamp min/max
 *          • Deadzone mode
 *          • Constant‑airflow mode (deadzone OFF)
 *          • PWM output
 *          • Damper relay control
 *
 *      Behavior Notes:
 *          • When deadzoneFanMode == 1:
 *                Fan stays OFF until fanPercent ≥ clampMin + 5
 *
 *          • When deadzoneFanMode == 0:
 *                Fan NEVER turns off
 *                Fan ALWAYS runs at clampMinPercent or higher
 *                Even if burn logic outputs 0%
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "FanControl.h"
#include "SystemState.h"
#include "Pinout.h"
#include <Arduino.h>

// ============================================================
//  fancontrol_init()
// ============================================================
void fancontrol_init() {
    pinMode(PIN_FAN_PWM, OUTPUT);
    pinMode(PIN_DAMPER_RELAY, OUTPUT);

    digitalWrite(PIN_DAMPER_RELAY, LOW);  // damper OPEN at boot (active‑low)
    
}


// ============================================================
//  BOOST Mode Handler
// ============================================================
void fancontrol_updateBoost() {
    if (burnState == BURN_BOOST) {
        if (millis() - burnBoostStart >= (unsigned long)boostTimeSeconds * 1000UL) {
            burnState = (burnLogicMode == 0) ? BURN_ADAPTIVE : BURN_PID;
        }
    }
}


// ============================================================
//  fancontrol_apply()
// ============================================================
int fancontrol_apply(int rawFanPercent) {

    int fanPercent = rawFanPercent;

    // --------------------------------------------------------
    // BOOST override
    // --------------------------------------------------------
    if (burnState == BURN_BOOST) {
        fanPercent = 100;
    }

    // --------------------------------------------------------
    // Clamp min/max
    // --------------------------------------------------------
    if (fanPercent < clampMinPercent) fanPercent = clampMinPercent;
    if (fanPercent > clampMaxPercent) fanPercent = clampMaxPercent;

    // --------------------------------------------------------
    // CONSTANT AIRFLOW MODE (Deadzone OFF)
    // Fan NEVER turns off, even if burn logic outputs 0
    // --------------------------------------------------------
    if (!deadzoneFanMode) {
        if (fanPercent < clampMinPercent)
            fanPercent = clampMinPercent;

        fanIsOff = false;   // fan is always considered ON
    }

    // --------------------------------------------------------
    // Deadzone mode (Deadzone ON)
    // --------------------------------------------------------
    else {
        if (fanPercent < clampMinPercent + 5) {
            fanPercent = 0;
            fanIsOff = true;
        } else {
            fanIsOff = false;
        }
    }

    // --------------------------------------------------------
    // PWM Output
    // --------------------------------------------------------
    int pwmValue = map(fanPercent, 0, 100, 0, 255);
    analogWrite(PIN_FAN_PWM, pwmValue);


    return fanPercent;
}

