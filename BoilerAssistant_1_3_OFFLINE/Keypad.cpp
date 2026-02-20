/*
 * ============================================================
 *  Boiler Assistant – Keypad Interface Module (Revised)
 *  ------------------------------------------------------------
 *  File: Keypad.cpp
 *  Author: The Architect Collective
 *  License: CC BY-NC-SA 4.0
 *
 *  Description:
 *    Implements the 4×4 matrix keypad scanner for the Boiler
 *    Assistant firmware. Provides stable, debounced ASCII key
 *    input for the UI state machine.
 *
 *  Responsibilities:
 *    - Configure keypad GPIO pins
 *    - Perform UNO R4–safe matrix scanning with precharge timing
 *    - Debounce keypresses
 *    - Return a single stable ASCII key per press
 *
 *  Architecture Notes:
 *    - scanKeypad() performs raw matrix scanning (no debounce)
 *    - getKey() provides debounced, non-blocking key retrieval
 *    - Returns 0 when no key is pressed
 *    - Keys are normalized to uppercase
 *
 *  This software is open-source for personal and private use.
 *  Commercial use, resale, or inclusion in commercial products
 *  is strictly prohibited under the CC BY-NC-SA 4.0 license.
 *
 *  Contributions welcome.
 *
 *  This header is documentation-only and has ZERO effect on
 *  compiled code, logic, timing, or behavior.
 * ============================================================
 */

#include "Keypad.h"
#include "Pinout.h"
#include <Arduino.h>


// ============================================================
//  SECTION: Key Mapping Table
//  Defines the ASCII characters for each row/column position.
// ============================================================
static const char KEYS[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};


// ============================================================
//  FUNCTION: keypad_init()
//  Initializes all keypad row and column pins as inputs with
//  pull-ups. Actual scanning logic is handled in scanKeypad().
// ============================================================
void keypad_init() {
  pinMode(PIN_KEYPAD_ROW1, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_ROW2, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_ROW3, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_ROW4, INPUT_PULLUP);

  pinMode(PIN_KEYPAD_COL1, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_COL2, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_COL3, INPUT_PULLUP);
  pinMode(PIN_KEYPAD_COL4, INPUT_PULLUP);
}


// ============================================================
//  FUNCTION: scanKeypad()
//  Purpose:
//    Performs a full matrix scan of the 4x4 keypad. Uses UNO R4
//    compatible precharge timing to ensure stable row activation.
//
//  Returns:
//    char - ASCII key pressed, or 0 if none detected.
//
//  Notes:
//    - This function does NOT debounce. Debouncing is handled
//      by getKey().
// ============================================================
static char scanKeypad() {
  const int rows[4] = {PIN_KEYPAD_ROW1, PIN_KEYPAD_ROW2, PIN_KEYPAD_ROW3, PIN_KEYPAD_ROW4};
  const int cols[4] = {PIN_KEYPAD_COL1, PIN_KEYPAD_COL2, PIN_KEYPAD_COL3, PIN_KEYPAD_COL4};

  char found = 0;

  for (int r = 0; r < 4; r++) {

    // Reset all rows to inactive (HIGH, input pull-up)
    for (int i = 0; i < 4; i++) {
      pinMode(rows[i], INPUT_PULLUP);
      digitalWrite(rows[i], HIGH);   // Required for UNO R4
    }

    // PRECHARGE active row HIGH before driving LOW
    digitalWrite(rows[r], HIGH);
    pinMode(rows[r], OUTPUT);
    digitalWrite(rows[r], LOW);

    delayMicroseconds(300);          // settle time

    // Scan columns for LOW (pressed key)
    for (int c = 0; c < 4; c++) {
      if (digitalRead(cols[c]) == LOW) {
        delayMicroseconds(80);       // confirm stable press
        if (digitalRead(cols[c]) == LOW)
          found = KEYS[r][c];
      }
    }
  }

  return found;
}


// ============================================================
//  FUNCTION: getKey()
//  Purpose:
//    Provides debounced, stable keypress detection. Returns a
//    single ASCII key only once per press.
//
//  Returns:
//    char - ASCII key pressed, or 0 if no stable key is detected.
//
//  Notes:
//    - Uses scanKeypad() internally
//    - Debounce time: ~40 ms
//    - Converts keys to uppercase
// ============================================================
char getKey() {
  static char lastKey = 0;
  static char stableKey = 0;
  static unsigned long lastChange = 0;

  char k = scanKeypad();
  unsigned long now = millis();

  // No key pressed
  if (k == 0) {
    stableKey = 0;
    lastKey = 0;
    return 0;
  }

  // Key changed — start debounce timer
  if (k != stableKey) {
    stableKey = k;
    lastChange = now;
    return 0;
  }

  // Debounce window
  if (now - lastChange < 40)
    return 0;

  // Already returned this key
  if (k == lastKey)
    return 0;

  lastKey = k;
  return toupper(k);
}
