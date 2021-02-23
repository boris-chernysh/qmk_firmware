#include "analog.h"
#include "pointing_device.h"
#include "math.h"
#include QMK_KEYBOARD_H

#define BASE 0
#define FN 1
#define NAV 2
#define LMOD 3
#define RMOD 4
#define MOUSE 5

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_ortho_5x14 (
            KC_NO, KC_NO, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL,
            KC_NO, KC_GRAVE, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET,
            KC_NO, KC_NO, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, KC_BSLASH,
            KC_NO, KC_NO, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, KC_BSLASH, KC_NO,
            KC_BSPACE, LT(FN, KC_ESC), LT(LMOD, KC_SPACE), LT(MOUSE, KC_TAB), KC_ENTER, KC_ENTER, LT(MOUSE, KC_TAB), LT(RMOD, KC_SPACE), LT(NAV, KC_ESC), KC_BSPACE
            ),
    [FN] = LAYOUT_ortho_5x14 (
            _______, KC_FN1, KC_FN2, KC_FN3, KC_FN4, KC_FN5, KC_FN6, KC_FN7, KC_FN8, KC_FN9, KC_FN10, KC_FN11, KC_FN12, _______,
            _______, _______, _______, _______, _______, KC_AUDIO_VOL_UP, _______,  _______, _______, _______, _______, _______, _______, _______,
            RESET, _______, KC_MEDIA_PREV_TRACK, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK, KC_AUDIO_MUTE, _______, _______, KC_PSCREEN, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, KC_AUDIO_VOL_DOWN, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
            ),
    [NAV] = LAYOUT_ortho_5x14 (
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
            ),
    [RMOD] = LAYOUT_ortho_5x14 (
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
            ),
    [LMOD] = LAYOUT_ortho_5x14 (
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
            ),
    [MOUSE] = LAYOUT_ortho_5x14 (
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, KC_MS_BTN3, KC_MS_BTN3, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, KC_MS_BTN2, KC_MS_BTN2, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, KC_MS_BTN1, KC_MS_BTN1, _______, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
            )
};

const pin_t leftXPin = F0;
const pin_t leftYPin = F1;
int8_t leftXPolarity = 1;
int8_t leftYPolarity = -1;
uint16_t leftxDead = 40;
uint16_t leftyDead = 40;
int16_t leftXCenter;
int16_t leftYCenter;

const pin_t rightXPin = B4;
const pin_t rightYPin = B5;
int8_t rightXPolarity = 1;
int8_t rightYPolarity = 1;
uint16_t rightxDead = 40;
uint16_t rightyDead = 40;
int16_t rightXCenter;
int16_t rightYCenter;

uint8_t cursorTimeout = 40;
uint16_t lastCursorTimer = 0;

// used to calculate exponential speed
float get_speed(int8_t xMove, int8_t yMove) {
    return (xMove * xMove + yMove * yMove) / 2 / square(127);
}

// by default: left joystick for scroll, right - for pointer
void pointing_device_task(void) {
    if (timer_elapsed(lastCursorTimer) < cursorTimeout) return;
    lastCursorTimer = timer_read();

    report_mouse_t report = pointing_device_get_report();

    int16_t leftXDelta = analogReadPin(leftXPin) - leftXCenter;
    int16_t leftYDelta = analogReadPin(leftYPin) - leftYCenter;
    if ( abs(leftXDelta) > leftxDead || abs(leftYDelta) > leftyDead ) {
        float leftxScale = leftXDelta >= 0 ? 1023 - leftXCenter : leftXCenter;
        float leftyScale = leftYDelta >= 0 ? 1023 - leftYCenter : leftYCenter;
        float xperc =  (float)leftXDelta / leftxScale;
        float yperc = (float)leftYDelta / leftyScale;
        int8_t leftxMove = (int8_t)(xperc * 127.0);
        int8_t leftyMove = (int8_t)(yperc * 127.0);

        float scrollSpeed = 0.1 * get_speed(leftxMove, leftyMove);

        report.h = leftXPolarity * leftxMove * scrollSpeed;
        report.v = leftYPolarity * leftyMove * scrollSpeed;
    }

    int16_t rightXDelta = analogReadPin(rightXPin) - rightXCenter;
    int16_t rightYDelta = analogReadPin(rightYPin) - rightYCenter;
    if ( abs(rightXDelta) > rightxDead || abs(rightYDelta) > rightyDead ) {
        float rightxScale = rightXDelta >= 0 ? 1023 - rightXCenter : rightXCenter;
        float rightyScale = rightYDelta >= 0 ? 1023 - rightYCenter : rightYCenter;
        float xperc = (float)rightXDelta / rightxScale;
        float yperc = (float)rightYDelta / rightyScale;

        int8_t rightxMove = (int8_t)(xperc * 127.0);
        int8_t rightyMove = (int8_t)(yperc * 127.0);

        float cursorSpeed = 1.7 * get_speed(rightxMove, rightyMove);

        report.x = rightXPolarity * rightxMove * cursorSpeed;
        report.y = rightYPolarity * rightyMove * cursorSpeed;
    }

    pointing_device_set_report(report);
    pointing_device_send();
}

void keyboard_post_init_user(void) {
    leftXCenter = analogReadPin(leftXPin);
    leftYCenter = analogReadPin(leftYPin);
    rightXCenter = analogReadPin(rightXPin);
    rightYCenter = analogReadPin(rightYPin);
}
