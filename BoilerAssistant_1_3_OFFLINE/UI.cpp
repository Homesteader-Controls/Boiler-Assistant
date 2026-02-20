/*
 * ============================================================
 *  Boiler Assistant – User Interface Module (v1.3)
 *  ------------------------------------------------------------
 *  File: UI.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *      Implements the complete LCD + keypad user interface for
 *      the Boiler Assistant controller. This module provides:
 *
 *          • Boot animation + system readiness screen
 *          • Home screen with live exhaust + fan data
 *          • Full configuration menu tree
 *          • PID tuning interface
 *          • Adaptive combustion diagnostics
 *          • Clamp, deadzone, BOOST, and system settings
 *
 *      Responsibilities:
 *          • Render all UI screens (20×4 LCD)
 *          • Handle keypad input and state transitions
 *          • Update EEPROM-backed settings
 *          • Display live system telemetry
 *
 *      Architecture Notes:
 *          • ui_showScreen() dispatches to all screen renderers
 *          • ui_handleKey() drives the UI state machine
 *          • lcd4() performs optimized 4‑line redraw with caching
 *          • All global UI state lives in SystemState.h
 *
 *      This header is documentation-only and has ZERO effect on
 *      compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "UI.h"
#include "SystemState.h"
#include "Pinout.h"
#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>
#include "Adaptive.h"
#include "EEPROMStorage.h"

// ============================================================
//  LCD Instance
// ============================================================
#define LCD_I2C_ADDRESS 0x27
static LiquidCrystal_PCF8574 lcd(LCD_I2C_ADDRESS);

// Forward reference for adaptive diag
extern double lastRate;


// ============================================================
//  Internal LCD 4‑line renderer
// ============================================================
static void lcd4(const char* l1, const char* l2, const char* l3, const char* l4) {
    static String last[4] = {"","","",""};
    const char* lines[4] = { l1, l2, l3, l4 };

    for (int i = 0; i < 4; i++) {
        String cur = String(lines[i]);
        if (cur != last[i]) {
            lcd.setCursor(0, i);
            lcd.print("                    ");
            lcd.setCursor(0, i);
            lcd.print(cur);
            last[i] = cur;
        }
    }
}


// ============================================================
//  Boot Screen
// ============================================================
static void showBootScreen() {
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("  BOILER ASSISTANT  ");
    delay(300);
    lcd.setCursor(0,1); lcd.print("    Initializing    ");
    delay(300);

    const char* bar[] = {
        "                    ","#                   ","##                  ",
        "###                 ","####                ","#####               ",
        "######              ","#######             ","########            ",
        "#########           ","##########          ","###########         ",
        "############        ","#############       ","##############      ",
        "###############     ","################    ","#################   ",
        "##################  ","################### ","********************"
    };

    for (int i = 0; i < 21; i++) {
        lcd.setCursor(0,2);
        lcd.print(bar[i]);
        delay(70);
    }

    lcd.setCursor(0,3);
    lcd.print("System Check OK");
    delay(800);

    lcd.clear();
    lcd.setCursor(0,1); lcd.print("  Ready to Ignite  ");
    delay(1200);
}


// ============================================================
//  ui_init()
// ============================================================
void ui_init() {
    lcd.begin(20,4);
    lcd.setBacklight(255);
    showBootScreen();
}


// ============================================================
//  Home Screen
// ============================================================
void ui_showHome(double exhaustF, int fanPercent) {
    char l1[21], l2[21], l3[21], l4[21];

    snprintf(l1,21,"Exh Set: %3dF", exhaustSetpoint);

    if (exhaustF < 0) snprintf(l2,21,"Exh Cur: ----F");
    else snprintf(l2,21,"Exh Cur: %3dF", (int)(exhaustF + 0.5));

    snprintf(l3,21,"Fan: %3d%%", fanPercent);

    if (burnState == BURN_BOOST)
        snprintf(l4,21,"BOOSTING");
    else if (burnLogicMode == 0)
        snprintf(l4,21,"Mode: ADAPTIVE");
    else
        snprintf(l4,21,"Mode: PID");

    lcd4(l1,l2,l3,l4);
}


// ============================================================
//  Screen Renderers
// ============================================================
static void ui_showSetpoint() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"EXHAUST SET POINT ");
    snprintf(l2,21,"Current: %3dF", exhaustSetpoint);
    snprintf(l3,21,"New: %s", newSetpointValue.c_str());
    snprintf(l4,21,"*=CANCEL   #=SAVE ");
    lcd4(l1,l2,l3,l4);
}

static void ui_showBurnLogic() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"BURN LOGIC MODE ");
    snprintf(l2,21,"1: ADAPTIVE%s", (burnLogicSelected==0)?" <":"");
    snprintf(l3,21,"2: PID%s", (burnLogicSelected==1)?" <":"");
    snprintf(l4,21,"3: BOOST TIME #Save");
    lcd4(l1,l2,l3,l4);
}

static void ui_showBoostTime() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"BOOST TIME (sec) ");
    snprintf(l2,21,"Current: %3d", boostTimeSeconds);
    snprintf(l3,21,"New: %s", boostTimeEditValue.c_str());
    snprintf(l4,21,"*=CANCEL   #=SAVE ");
    lcd4(l1,l2,l3,l4);
}

static void ui_showSystem() {
    lcd4(
        "SYSTEM SETTINGS  ",
        "1: DEADBAND",
        "2: ADAPTIVE DIAG",
        "3: CLAMP  *=BACK"
    );
}

static void ui_showDeadband() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"DEADBAND (F)    ");
    snprintf(l2,21,"Current: %3d", deadbandF);
    snprintf(l3,21,"New: %s", deadbandEditValue.c_str());
    snprintf(l4,21,"*=CANCEL   #=SAVE ");
    lcd4(l1,l2,l3,l4);
}

static void ui_showClampMenu() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"CLAMP SETTINGS   ");
    snprintf(l2,21,"1:Min: %3d%%", clampMinPercent);
    snprintf(l3,21,"  Max: %3d%%", clampMaxPercent);
    snprintf(l4,21,"4:Deadzone: %s", deadzoneFanMode ? "OFF" : "ON");
    lcd4(l1,l2,l3,l4);
}

static void ui_showClampMin() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"EDIT CLAMP MIN (%%)");
    snprintf(l2,21,"Current: %3d", clampMinPercent);
    snprintf(l3,21,"New: %s", clampMinEditValue.c_str());
    snprintf(l4,21,"*=CANCEL   #=SAVE ");
    lcd4(l1,l2,l3,l4);
}

static void ui_showClampMax() {
    char l1[21],l2[21],l3[21],l4[21];
    snprintf(l1,21,"EDIT CLAMP MAX (%%)");
    snprintf(l2,21,"Current: %3d", clampMaxPercent);
    snprintf(l3,21,"New: %s", clampMaxEditValue.c_str());
    snprintf(l4,21,"*=CANCEL   #=SAVE ");
    lcd4(l1,l2,l3,l4);
}

static void ui_showPIDProfile() {
    lcd4(
        "PID TUNING     ",
        "1: BELOW",
        "2: NORMAL",
        "3: ABOVE   *=BACK"
    );
}

static void ui_showPIDParam() {
    char l1[21],l2[21],l3[21],l4[21];
    float kp,ki,kd;
    const char* title;

    if (pidProfileSelected==1) {
        title="PID BELOW";
        kp=pidBelowKp; ki=pidBelowKi; kd=pidBelowKd;
    } else if (pidProfileSelected==2) {
        title="PID NORMAL";
        kp=pidNormKp; ki=pidNormKi; kd=pidNormKd;
    } else {
        title="PID ABOVE";
        kp=pidAboveKp; ki=pidAboveKi; kd=pidAboveKd;
    }

    snprintf(l1,21,"%s",title);
    snprintf(l2,21,"1:KP %.3f",kp);
    snprintf(l3,21,"2:KI %.3f",ki);
    snprintf(l4,21,"3:KD %.3f *=BACK",kd);

    lcd4(l1,l2,l3,l4);
}

static void ui_showPIDEdit() {
    char l1[21],l2[21],l3[21],l4[21];

    const char* prof =
        (pidProfileSelected==1) ? "BELOW" :
        (pidProfileSelected==2) ? "NORMAL" :
                                  "ABOVE";

    const char* param =
        (pidParamSelected==1) ? "KP" :
        (pidParamSelected==2) ? "KI" :
                                "KD";

    float current;

    if (pidProfileSelected==1) {
        current = (pidParamSelected==1) ? pidBelowKp :
                  (pidParamSelected==2) ? pidBelowKi :
                                          pidBelowKd;
    }
    else if (pidProfileSelected==2) {
        current = (pidParamSelected==1) ? pidNormKp :
                  (pidParamSelected==2) ? pidNormKi :
                                          pidNormKd;
    }
    else {
        current = (pidParamSelected==1) ? pidAboveKp :
                  (pidParamSelected==2) ? pidAboveKi :
                                          pidAboveKd;
    }

    snprintf(l1,21,"EDIT %s (%s)",param,prof);
    snprintf(l2,21,"Current: %.3f",current);
    snprintf(l3,21,"New: %s",pidEditValue.c_str());
    snprintf(l4,21,"D=DOT *=CANCEL #=SAVE");

    lcd4(l1,l2,l3,l4);
}

static void ui_showAdaptiveDiag() {
    char l1[21],l2[21],l3[21],l4[21];

    snprintf(l1,21,"ADAPTIVE DIAGNOSTIC");
    snprintf(l2,21,"Slope: %.2f", adaptiveSlope);
    snprintf(l3,21,"dT/ds: %.3f", lastRate);
    snprintf(l4,21,"*=BACK   #=RESET   ");

    lcd4(l1,l2,l3,l4);
}


// ============================================================
//  ui_showScreen()
// ============================================================
void ui_showScreen(int state, double exhaustF, int fanPercent) {
    switch(state) {
        case UI_HOME:          ui_showHome(exhaustF, fanPercent); break;
        case UI_SETPOINT:      ui_showSetpoint(); break;
        case UI_BURNLOGIC:     ui_showBurnLogic(); break;
        case UI_BOOSTTIME:     ui_showBoostTime(); break;
        case UI_SYSTEM:        ui_showSystem(); break;
        case UI_DEADBAND:      ui_showDeadband(); break;
        case UI_PID_PROFILE:   ui_showPIDProfile(); break;
        case UI_PID_PARAM:     ui_showPIDParam(); break;
        case UI_PID_EDIT:      ui_showPIDEdit(); break;
        case UI_ADAPTIVE_DIAG: ui_showAdaptiveDiag(); break;
        case UI_CLAMP_MENU:    ui_showClampMenu(); break;
        case UI_CLAMP_MIN:     ui_showClampMin(); break;
        case UI_CLAMP_MAX:     ui_showClampMax(); break;
        default:               ui_showHome(exhaustF, fanPercent); break;
    }
}


// ============================================================
//  ui_handleKey()
// ============================================================
void ui_handleKey(char k, double exhaustF, int fanPercent) {
    if (!k) return;

    uiNeedRedraw = true;

    switch(uiState) {

        case UI_HOME:
            if (k=='A') { uiState=UI_SETPOINT; newSetpointValue=""; }
            else if (k=='B') { uiState=UI_BURNLOGIC; burnLogicSelected=burnLogicMode; }
            else if (k=='C') { uiState=UI_PID_PROFILE; }
            else if (k=='D') { uiState=UI_SYSTEM; }
            break;

        case UI_SETPOINT:
            if (k>='0' && k<='9' && newSetpointValue.length()<3)
                newSetpointValue += k;
            else if (k=='#') {
                int v = newSetpointValue.length() ? newSetpointValue.toInt() : exhaustSetpoint;
                if (v < 200) v = 200;
                if (v > 999) v = 999;
                exhaustSetpoint = v;

                eeprom_save_setpoint();

                uiState = UI_HOME;
            }
            else if (k=='*') uiState = UI_HOME;
            break;

        case UI_BURNLOGIC:
            if (k=='1') burnLogicSelected=0;
            else if (k=='2') burnLogicSelected=1;
            else if (k=='3') { uiState=UI_BOOSTTIME; boostTimeEditValue=""; break; }
            else if (k=='#') {
                burnLogicMode = burnLogicSelected;
                eeprom_save_burnlogic();
                uiState = UI_HOME;
            }
            else if (k=='*') uiState = UI_HOME;
            break;

        case UI_BOOSTTIME:
            if (k>='0' && k<='9' && boostTimeEditValue.length()<3)
                boostTimeEditValue += k;
            else if (k=='#') {
                int v = boostTimeEditValue.length() ? boostTimeEditValue.toInt() : boostTimeSeconds;
                if (v < 10) v = 10;
                if (v > 300) v = 300;
                boostTimeSeconds = v;

                eeprom_save_boost();

                uiState = UI_BURNLOGIC;
            }
            else if (k=='*') uiState = UI_BURNLOGIC;
            break;

        case UI_SYSTEM:
            if (k=='1') { uiState=UI_DEADBAND; deadbandEditValue=""; }
            else if (k=='2') { uiState=UI_ADAPTIVE_DIAG; }
            else if (k=='3') { uiState=UI_CLAMP_MENU; }
            else if (k=='*') uiState = UI_HOME;
            break;

        case UI_DEADBAND:
            if (k>='0' && k<='9' && deadbandEditValue.length()<3)
                deadbandEditValue += k;
            else if (k=='#') {
                int v = deadbandEditValue.length() ? deadbandEditValue.toInt() : deadbandF;
                if (v < 10) v = 10;
                if (v > 200) v = 200;
                deadbandF = v;

                eeprom_save_deadband();

                uiState = UI_SYSTEM;
            }
            else if (k=='*') uiState = UI_SYSTEM;
            break;

        case UI_CLAMP_MENU:
            if (k=='1') { uiState=UI_CLAMP_MIN; clampMinEditValue=""; }
            else if (k=='2') { uiState=UI_CLAMP_MAX; clampMaxEditValue=""; }
            else if (k=='4') {
                deadzoneFanMode = !deadzoneFanMode;
                eeprom_save_clamps();
            }
            else if (k=='*') uiState = UI_SYSTEM;
            break;

        case UI_CLAMP_MIN:
            if (k>='0' && k<='9' && clampMinEditValue.length()<3)
                clampMinEditValue += k;
            else if (k=='#') {
                int v = clampMinEditValue.length() ? clampMinEditValue.toInt() : clampMinPercent;
                if (v < 0) v = 0;
                if (v > 100) v = 100;

                clampMinPercent = v;
                if (clampMinPercent > clampMaxPercent)
                    clampMaxPercent = clampMinPercent;

                eeprom_save_clamps();

                uiState = UI_CLAMP_MAX;
                clampMaxEditValue = "";
            }
            else if (k=='*') uiState = UI_CLAMP_MENU;
            break;

        case UI_CLAMP_MAX:
            if (k>='0' && k<='9' && clampMaxEditValue.length()<3)
                clampMaxEditValue += k;
            else if (k=='#') {
                int v = clampMaxEditValue.length() ? clampMaxEditValue.toInt() : clampMaxPercent;
                if (v < 0) v = 0;
                if (v > 100) v = 100;

                clampMaxPercent = v;
                if (clampMinPercent > clampMaxPercent)
                    clampMinPercent = clampMaxPercent;

                eeprom_save_clamps();

                uiState = UI_CLAMP_MENU;
            }
            else if (k=='*') uiState = UI_CLAMP_MENU;
            break;

                case UI_PID_PROFILE:
            if (k=='1') { pidProfileSelected=1; uiState=UI_PID_PARAM; }
            else if (k=='2') { pidProfileSelected=2; uiState=UI_PID_PARAM; }
            else if (k=='3') { pidProfileSelected=3; uiState=UI_PID_PARAM; }
            else if (k=='*') uiState = UI_HOME;
            break;

        case UI_PID_PARAM:
            if (k=='1') { pidParamSelected=1; pidEditValue=""; uiState=UI_PID_EDIT; }
            else if (k=='2') { pidParamSelected=2; pidEditValue=""; uiState=UI_PID_EDIT; }
            else if (k=='3') { pidParamSelected=3; pidEditValue=""; uiState=UI_PID_EDIT; }
            else if (k=='*') uiState = UI_PID_PROFILE;
            break;

        case UI_PID_EDIT:
            if (k>='0' && k<='9') {
                if (pidEditValue.length() < 6)
                    pidEditValue += k;
            }
            else if (k=='D') {
                if (pidEditValue.indexOf('.') == -1 && pidEditValue.length() < 6)
                    pidEditValue += '.';
            }
            else if (k=='#') {
                float v = pidEditValue.length() ? pidEditValue.toFloat() : 0.0f;

                if (pidProfileSelected==1) {
                    if (pidParamSelected==1) pidBelowKp = v;
                    else if (pidParamSelected==2) pidBelowKi = v;
                    else pidBelowKd = v;
                }
                else if (pidProfileSelected==2) {
                    if (pidParamSelected==1) pidNormKp = v;
                    else if (pidParamSelected==2) pidNormKi = v;
                    else pidNormKd = v;
                }
                else {
                    if (pidParamSelected==1) pidAboveKp = v;
                    else if (pidParamSelected==2) pidAboveKi = v;
                    else pidAboveKd = v;
                }

                eeprom_save_pid();

                uiState = UI_PID_PARAM;
            }
            else if (k=='*') uiState = UI_PID_PARAM;
            break;

        case UI_ADAPTIVE_DIAG:
            if (k=='*') uiState = UI_SYSTEM;
            else if (k=='#') adaptive_reset();
            break;

        default:
            uiState = UI_HOME;
            break;
    }
}
