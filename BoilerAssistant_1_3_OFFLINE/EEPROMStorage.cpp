/*
 * ============================================================
 *  Boiler Assistant – EEPROM Storage Module (v1.3)
 *  ------------------------------------------------------------
 *  File: EEPROMStorage.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Handles persistent storage of all user‑configurable
 *      parameters for the Boiler Assistant firmware.
 *
 *      Stored values include:
 *          • Exhaust setpoint (16‑bit)
 *          • Burn logic mode (Adaptive / PID)
 *          • BOOST duration
 *          • Deadband (°F)
 *          • Clamp limits (min/max fan %)
 *          • Deadzone mode
 *          • PID profiles (Below / Normal / Above)
 *
 *      EEPROM versioning ensures safe upgrades:
 *          • If version mismatch → defaults are written
 *          • If version matches → values are loaded normally
 *
 *  Notes for Contributors:
 *      • All EEPROM addresses are defined at the top of this file.
 *      • eeprom_save_all() writes every parameter.
 *      • eeprom_init() MUST be called before any other module.
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "EEPROMStorage.h"
#include "SystemState.h"
#include <EEPROM.h>

#define EEPROM_VERSION 0x15   // bump version after layout fix

// EEPROM layout
#define ADDR_VERSION        0

// store setpoint as 16‑bit (2 bytes)
#define ADDR_SETPOINT       1   // uses bytes 1–2

#define ADDR_BURNLOGIC      3
#define ADDR_BOOSTTIME      4
#define ADDR_DEADBAND       6
#define ADDR_CLAMP_MIN      8
#define ADDR_CLAMP_MAX      9
#define ADDR_DEADZONE       10

// PID profiles (floats)
#define ADDR_PID_BELOW_KP   20
#define ADDR_PID_BELOW_KI   24
#define ADDR_PID_BELOW_KD   28

#define ADDR_PID_NORM_KP    32
#define ADDR_PID_NORM_KI    36
#define ADDR_PID_NORM_KD    40

#define ADDR_PID_ABOVE_KP   44
#define ADDR_PID_ABOVE_KI   48
#define ADDR_PID_ABOVE_KD   52


void eeprom_init() {
    uint8_t ver = EEPROM.read(ADDR_VERSION);

    if (ver != EEPROM_VERSION) {
        EEPROM.write(ADDR_VERSION, EEPROM_VERSION);
        eeprom_save_all();
        return;
    }

    // setpoint as 16‑bit
    int16_t sp16 = 0;
    EEPROM.get(ADDR_SETPOINT, sp16);
    exhaustSetpoint = sp16;

    burnLogicMode    = EEPROM.read(ADDR_BURNLOGIC);
    boostTimeSeconds = EEPROM.read(ADDR_BOOSTTIME);
    deadbandF        = EEPROM.read(ADDR_DEADBAND);

    clampMinPercent  = EEPROM.read(ADDR_CLAMP_MIN);
    clampMaxPercent  = EEPROM.read(ADDR_CLAMP_MAX);
    deadzoneFanMode  = EEPROM.read(ADDR_DEADZONE);

    EEPROM.get(ADDR_PID_BELOW_KP, pidBelowKp);
    EEPROM.get(ADDR_PID_BELOW_KI, pidBelowKi);
    EEPROM.get(ADDR_PID_BELOW_KD, pidBelowKd);

    EEPROM.get(ADDR_PID_NORM_KP, pidNormKp);
    EEPROM.get(ADDR_PID_NORM_KI, pidNormKi);
    EEPROM.get(ADDR_PID_NORM_KD, pidNormKd);

    EEPROM.get(ADDR_PID_ABOVE_KP, pidAboveKp);
    EEPROM.get(ADDR_PID_ABOVE_KI, pidAboveKi);
    EEPROM.get(ADDR_PID_ABOVE_KD, pidAboveKd);
}


// SAVE FUNCTIONS
void eeprom_save_setpoint() {
    int16_t sp16 = (int16_t)exhaustSetpoint;
    EEPROM.put(ADDR_SETPOINT, sp16);
}

void eeprom_save_burnlogic() {
    EEPROM.write(ADDR_BURNLOGIC, burnLogicMode);
}

void eeprom_save_boost() {
    EEPROM.write(ADDR_BOOSTTIME, boostTimeSeconds);
}

void eeprom_save_deadband() {
    EEPROM.write(ADDR_DEADBAND, deadbandF);
}

void eeprom_save_clamps() {
    EEPROM.write(ADDR_CLAMP_MIN, clampMinPercent);
    EEPROM.write(ADDR_CLAMP_MAX, clampMaxPercent);
    EEPROM.write(ADDR_DEADZONE, deadzoneFanMode);
}

void eeprom_save_pid() {
    EEPROM.put(ADDR_PID_BELOW_KP, pidBelowKp);
    EEPROM.put(ADDR_PID_BELOW_KI, pidBelowKi);
    EEPROM.put(ADDR_PID_BELOW_KD, pidBelowKd);

    EEPROM.put(ADDR_PID_NORM_KP, pidNormKp);
    EEPROM.put(ADDR_PID_NORM_KI, pidNormKi);
    EEPROM.put(ADDR_PID_NORM_KD, pidNormKd);

    EEPROM.put(ADDR_PID_ABOVE_KP, pidAboveKp);
    EEPROM.put(ADDR_PID_ABOVE_KI, pidAboveKi);
    EEPROM.put(ADDR_PID_ABOVE_KD, pidAboveKd);
}

void eeprom_save_all() {
    eeprom_save_setpoint();
    eeprom_save_burnlogic();
    eeprom_save_boost();
    eeprom_save_deadband();
    eeprom_save_clamps();
    eeprom_save_pid();
}
