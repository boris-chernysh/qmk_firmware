#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID 0xFEED
#define PRODUCT_ID 0x5D5E
#define DEVICE_VER 0x0001
#define MANUFACTURER Boris Chernysh
#define PRODUCT BigThumb
#define DESCRIPTION Cool keeb for good guys

/* key matrix size */
#define MATRIX_ROWS 5
#define MATRIX_COLS 14

/*
 * Keyboard Matrix Assignments
 *
 * Change this to how you wired your keyboard
 * COLS: AVR pins used for columns, left to right
 * ROWS: AVR pins used for rows, top to bottom
 * DIODE_DIRECTION: COL2ROW = COL = Anode (+), ROW = Cathode (-, marked on diode)
 *                  ROW2COL = ROW = Anode (+), COL = Cathode (-, marked on diode)
 *
 */
#define MATRIX_ROW_PINS { D1, D2, B3, B2, B1 }
#define MATRIX_COL_PINS { D5, C7, C6, D4, D0, E6, F0, F1, B6, D7, F6, F7, D3, D6 }
#define UNUSED_PINS

/* COL2ROW, ROW2COL*/
#define DIODE_DIRECTION COL2ROW

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION
