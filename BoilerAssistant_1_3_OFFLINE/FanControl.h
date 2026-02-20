/*
 * ============================================================
 *  Boiler Assistant – Fan Control Module (v1.3)
 *  ------------------------------------------------------------
 *  File: FanControl.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Public interface for the fan shaping and damper relay
 *      control module. FanControl applies:
 *
 *          • BOOST override
 *          • Clamp min/max
 *          • Deadzone mode
 *          • PWM output
 *          • Damper relay logic
 *
 *      BurnLogic computes the *raw* fan %, and FanControl
 *      applies all shaping and hardware output.
 *
 *  Notes for Contributors:
 *      • fancontrol_apply() is the ONLY function that outputs
 *        the final PWM and damper relay state.
 *      • BOOST timing is updated via fancontrol_updateBoost().
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
void fancontrol_init();

// ============================================================
//  BOOST Mode Handler
// ============================================================
void fancontrol_updateBoost();

// ============================================================
//  Apply shaping + PWM + damper relay
//  Returns final applied fan percentage (0–100)
// ============================================================
int fancontrol_apply(int rawFanPercent);
