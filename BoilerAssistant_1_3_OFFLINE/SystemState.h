/*
 * ============================================================
 *  Boiler Assistant – Global System State (v1.3)
 *  ------------------------------------------------------------
 *  File: SystemState.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Declares all global runtime variables used across the
 *      Boiler Assistant firmware. These variables represent:
 *
 *          • UI state machine
 *          • Burn logic mode + BOOST state
 *          • PID profiles
 *          • Adaptive combustion tracking
 *          • Clamp + deadzone configuration
 *          • Environmental sensor placeholders
 *          • Water probe placeholders
 *          • UI edit buffers
 *
 *      Notes:
 *          • This file contains *declarations only*.
 *          • All variables are defined in the main .ino file.
 *          • Modules read/write these globals to coordinate
 *            system behavior.
 *
 *      Contributor Notes:
 *          • Keep this file synchronized with the .ino definitions.
 *          • Avoid adding logic here — declarations only.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ============================================================
//  UI State Machine
// ============================================================

enum UIState {
    UI_HOME,
    UI_SETPOINT,
    UI_BURNLOGIC,
    UI_BOOSTTIME,
    UI_SYSTEM,
    UI_DEADBAND,
    UI_PID_PROFILE,
    UI_PID_PARAM,
    UI_PID_EDIT,
    UI_ADAPTIVE_DIAG,
    UI_CLAMP_MENU,
    UI_CLAMP_MIN,
    UI_CLAMP_MAX
};

extern UIState uiState;
extern bool uiNeedRedraw;


// ============================================================
//  Burn Logic State Machine
// ============================================================

enum BurnState {
    BURN_ADAPTIVE,
    BURN_PID,
    BURN_BOOST
};

extern BurnState burnState;


// ============================================================
//  Core Settings
// ============================================================

extern int exhaustSetpoint;
extern int burnLogicMode;
extern int boostTimeSeconds;
extern int deadbandF;


// ============================================================
//  PID Profiles
// ============================================================

extern float pidBelowKp, pidBelowKi, pidBelowKd;
extern float pidNormKp,  pidNormKi,  pidNormKd;
extern float pidAboveKp, pidAboveKi, pidAboveKd;


// ============================================================
//  Adaptive Combustion
// ============================================================

extern double lastT;
extern unsigned long lastTtime;
extern double lastRate;
extern float adaptiveSlope;


// ============================================================
//  Clamp + Deadzone
// ============================================================

extern int clampMinPercent;
extern int clampMaxPercent;
extern int deadzoneFanMode;
extern bool fanIsOff;


// ============================================================
//  BOOST State
// ============================================================

extern unsigned long burnBoostStart;


// ============================================================
//  Environmental Sensors (placeholders)
// ============================================================

extern float envTempF;
extern float envHumidity;
extern float envPressure;
extern bool envSensorOK;


// ============================================================
//  Water Probes (placeholders)
// ============================================================

extern int waterSensorCount;
extern double waterTemps[8];


// ============================================================
//  UI Edit Buffers
// ============================================================

extern String newSetpointValue;
extern String boostTimeEditValue;
extern String deadbandEditValue;
extern String pidEditValue;
extern String clampMinEditValue;
extern String clampMaxEditValue;

extern int burnLogicSelected;
extern int pidProfileSelected;
extern int pidParamSelected;
