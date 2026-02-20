/*
 * ============================================================
 *  Boiler Assistant – User Interface Module (v1.3)
 *  ------------------------------------------------------------
 *  File: UI.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Public interface for the Boiler Assistant LCD + keypad
 *      user interface. This module provides:
 *
 *          • UI initialization
 *          • Screen rendering for all UI states
 *          • Key handling for navigation + numeric entry
 *
 *      The UI is responsible for presenting system information,
 *      editing configuration values, and driving the UI state
 *      machine defined in SystemState.h.
 *
 *  Notes for Contributors:
 *      • ui_showScreen() must be called only when uiNeedRedraw is true.
 *      • ui_handleKey() is non‑blocking and processes one key at a time.
 *      • All UI state variables live in SystemState.h.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>
#include "SystemState.h"

// ============================================================
//  UI Initialization
// ============================================================
void ui_init();

// ============================================================
//  Screen Rendering
// ============================================================
void ui_showScreen(int state, double exhaustF, int fanPercent);

// ============================================================
//  Key Handling
// ============================================================
void ui_handleKey(char k, double exhaustF, int fanPercent);
