/*
 * ============================================================
 *  Boiler Assistant – EEPROM Storage Module (v1.3)
 *  ------------------------------------------------------------
 *  File: EEPROMStorage.h
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Declares the EEPROM storage interface for the Boiler
 *      Assistant firmware. This module provides functions for:
 *
 *          • Initializing EEPROM versioning
 *          • Loading all persisted configuration values
 *          • Saving individual parameters (setpoint, clamps, PID, etc.)
 *          • Saving all parameters at once
 *
 *      EEPROM versioning ensures safe upgrades:
 *          • If version mismatch → defaults are written
 *          • If version matches → values are loaded normally
 *
 *  Notes for Contributors:
 *      • All EEPROM addresses are defined in EEPROMStorage.cpp.
 *      • eeprom_init() MUST be called before any other module.
 *      • All functions declared here are safe to call externally.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

void eeprom_init();
void eeprom_save_all();
void eeprom_save_setpoint();
void eeprom_save_burnlogic();
void eeprom_save_boost();
void eeprom_save_deadband();
void eeprom_save_clamps();
void eeprom_save_pid();
