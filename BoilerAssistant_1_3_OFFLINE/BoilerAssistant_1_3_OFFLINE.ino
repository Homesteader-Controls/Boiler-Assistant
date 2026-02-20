/*
 * ============================================================
 *  Boiler Assistant – Main Firmware (v1.3)
 *  ------------------------------------------------------------
 *  File: BoilerAssistant_1_3.ino
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *    Main entry point for the Boiler Assistant controller.
 *    Coordinates:
 *      - Sensor acquisition
 *      - Burn logic (Adaptive or PID)
 *      - Fan control (clamp, deadzone, BOOST)
 *      - UI rendering + keypad input
 *
 *  Notes:
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include <Arduino.h>
#include "Pinout.h"
#include "SystemState.h"
#include "Sensors.h"
#include "FanControl.h"
#include "BurnLogic.h"
#include "Adaptive.h"
#include "UI.h"
#include "EEPROMStorage.h"


// ============================================================
//  GLOBAL STATE INITIALIZATION
// ============================================================

// UI
UIState uiState = UI_HOME;
bool uiNeedRedraw = true;

// Burn logic
BurnState burnState = BURN_ADAPTIVE;

// EEPROM-backed values (defaults)
int exhaustSetpoint     = 350;
int burnLogicMode       = 0;
int boostTimeSeconds    = 30;
int deadbandF           = 25;

// PID profiles
float pidBelowKp = 0.8, pidBelowKi = 0.02, pidBelowKd = 0.0;
float pidNormKp  = 0.6, pidNormKi  = 0.01, pidNormKd  = 0.0;
float pidAboveKp = 0.4, pidAboveKi = 0.00, pidAboveKd = 0.0;

// Adaptive
float adaptiveSlope = 1.0;

// Clamp limits
int clampMinPercent = 10;
int clampMaxPercent = 100;

// Deadzone
int deadzoneFanMode = 0;
bool fanIsOff = false;

// Adaptive tracking
double lastRate = 0;
double lastT = 0;
unsigned long lastTtime = 0;

// BOOST
unsigned long burnBoostStart = 0;

// Sensor placeholders
float envTempF = NAN;
float envHumidity = NAN;
float envPressure = NAN;
bool envSensorOK = false;

double exhaustTempF = NAN;

int waterSensorCount = 0;
double waterTemps[8] = {NAN,NAN,NAN,NAN,NAN,NAN,NAN,NAN};

// UI edit buffers
String newSetpointValue = "";
String boostTimeEditValue = "";
String deadbandEditValue = "";
String pidEditValue = "";
String clampMinEditValue = "";
String clampMaxEditValue = "";

int burnLogicSelected = 0;
int pidProfileSelected = 1;
int pidParamSelected = 1;


// ============================================================
//  KEYPAD STATE
// ============================================================
char lastKey = 0;


// ============================================================
//  KEYPAD READER
// ============================================================
char readKeypad() {
    static const char keys[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };

    for (int col = 0; col < 4; col++) {
        pinMode(PIN_KEYPAD_COL1 + col, OUTPUT);
        digitalWrite(PIN_KEYPAD_COL1 + col, LOW);

        for (int row = 0; row < 4; row++) {
            pinMode(PIN_KEYPAD_ROW1 + row, INPUT_PULLUP);
            if (digitalRead(PIN_KEYPAD_ROW1 + row) == LOW) {
                digitalWrite(PIN_KEYPAD_COL1 + col, HIGH);
                pinMode(PIN_KEYPAD_COL1 + col, INPUT);
                return keys[row][col];
            }
        }

        digitalWrite(PIN_KEYPAD_COL1 + col, HIGH);
        pinMode(PIN_KEYPAD_COL1 + col, INPUT);
    }

    return 0;
}


// ============================================================
//  setup() — FIXED ORDER
// ============================================================
void setup() {

    // EEPROM MUST LOAD FIRST
    eeprom_init();

    // Now hardware and logic can safely initialize
    sensors_init();
    fancontrol_init();
    burnlogic_init();
    ui_init();

// --------------------------------------------------------
// AUTO-BOOST AT STARTUP
// --------------------------------------------------------
burnState = BURN_BOOST;
burnBoostStart = millis();
}


// ============================================================
//  loop()
// ============================================================
void loop() {

    // --------------------------------------------------------
    // Read exhaust temperature
    // --------------------------------------------------------
    double rawExhaust = exhaust_readF_cached();
    double smoothExh;

    // If the MAX31855 returned NAN, force UI to show ----F
    if (isnan(rawExhaust)) {
    smoothExh = -1;   // UI interprets -1 as "no reading"
    }    
    else {
    smoothExh = smoothExhaustF(rawExhaust);
}

// Store final exhaust temperature for UI and logic
exhaustTempF = smoothExh;


    // --------------------------------------------------------
    // Compute raw fan % (Adaptive or PID)
    // --------------------------------------------------------
    int rawFan = burnlogic_compute(smoothExh);

    // --------------------------------------------------------
    // Apply BOOST, clamp, deadzone, and output PWM
    // --------------------------------------------------------
    fancontrol_updateBoost();
    int finalFan = fancontrol_apply(rawFan);

    // --------------------------------------------------------
    // KEYPAD INPUT
    // --------------------------------------------------------
    char key = readKeypad();

    if (key != 0) {
        if (key != lastKey) {
            ui_handleKey(key, smoothExh, finalFan);
        }
        lastKey = key;
    } else {
        lastKey = 0;
    }

    // --------------------------------------------------------
    // UI Rendering
    // --------------------------------------------------------
    if (uiNeedRedraw) {
        uiNeedRedraw = false;
        ui_showScreen(uiState, smoothExh, finalFan);
    }

    delay(20);
}
