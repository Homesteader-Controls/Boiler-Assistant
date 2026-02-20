/*
 * ============================================================
 *  Boiler Assistant – Keypad Interface Module (v1.3)
 *  ------------------------------------------------------------
 *  File: Keypad.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Public interface for the 4x4 matrix keypad scanner used
 *      by the Boiler Assistant firmware. This module provides:
 *
 *          • Initialization of keypad GPIO pins
 *          • Debounced, non‑blocking ASCII key retrieval
 *
 *      The keypad is used for all UI navigation and numeric
 *      entry. Implementation is provided in Keypad.cpp.
 *
 *  Notes for Contributors:
 *      • getKey() is non‑blocking and returns 0 when no key is pressed.
 *      • Debouncing is handled internally in Keypad.cpp.
 *      • UI.cpp consumes keypresses and drives the state machine.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once

// ============================================================
//  SECTION: Initialization
// ============================================================

/**
 * keypad_init()
 * ------------------------------------------------------------
 * Configures all GPIO pins required for the 4x4 matrix keypad.
 *
 * Notes:
 *      • Must be called once during system startup.
 *      • Sets row pins as outputs and column pins as inputs.
 */
void keypad_init();


// ============================================================
//  SECTION: Key Retrieval
// ============================================================

/**
 * getKey()
 * ------------------------------------------------------------
 * Returns a single debounced ASCII keypress from the keypad.
 *
 * Behavior:
 *      • Non‑blocking — returns immediately.
 *      • Returns 0 when no key is pressed.
 *      • Returns ASCII characters for digits, A–D, *, #.
 *
 * Returns:
 *      char - ASCII key or 0 if idle.
 *
 * Notes:
 *      • UI.cpp calls this once per loop.
 *      • All keypad logic is implemented in Keypad.cpp.
 */
char getKey();
