#include "analog.h"
#include "pointing_device.h"
#include "math.h"
#include QMK_KEYBOARD_H

#define BASE 0
#define FN 1
#define NAV 2
#define LMOD 3
#define RMOD 4

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = layout_two_halfs (
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, KC_GRAVE, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET,
            XXXXXXX, XXXXXXX, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, XXXXXXX,
            XXXXXXX, XXXXXXX, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, XXXXXXX, XXXXXXX,
            LT(LMOD, KC_SPACE), LT(FN, KC_BSPACE), LT(NAV, KC_BSPACE), LT(RMOD, KC_SPACE)
            ),
    [FN] = layout_two_halfs (
            XXXXXXX, KC_FN1, KC_FN2, KC_FN3, KC_FN4, KC_FN5, KC_FN6, KC_FN7, KC_FN8, KC_FN9, KC_FN10, KC_FN11, KC_FN12, XXXXXXX,
            XXXXXXX, XXXXXXX, KC_AUDIO_MUTE, XXXXXXX, KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            RESET, XXXXXXX, XXXXXXX, KC_MEDIA_PREV_TRACK, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK, XXXXXXX, KC_PSCREEN, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_MS_BTN3, KC_MS_BTN2, KC_MS_BTN1, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
            ),
    [NAV] = layout_right_half (
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT, XXXXXXX, XXXXXXX, XXXXXXX,
            KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX
            ),
    [RMOD] = layout_right_half (
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL,
            XXXXXXX, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, KC_BSLASH, XXXXXXX,
            KC_ENTER, KC_ESC, KC_CAPS, KC_DEL, KC_TAB, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX
            ),
    [LMOD] = layout_left_half (
            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
            XXXXXXX, XXXXXXX, KC_1, KC_2, KC_3, KC_4, KC_5,
            XXXXXXX, XXXXXXX, KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, XXXXXXX,
            XXXXXXX, XXXXXXX, KC_TAB, KC_DEL, KC_CAPS, KC_ESC, KC_ENTER,
            XXXXXXX, XXXXXXX
            )
};

const pin_t leftXPin = F0;
const pin_t leftYPin = F1;
int8_t leftXPolarity = -1;
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

        if (layer_state_is(RMOD)) {
            float scrollSpeed = 0.1 * get_speed(leftxMove, leftyMove);
            report.h = leftXPolarity * leftxMove * scrollSpeed;
            report.v = leftYPolarity * leftyMove * scrollSpeed;
        } else {
            float cursorSpeed = 1.7 * get_speed(leftxMove, leftyMove);
            report.x = leftXPolarity * leftxMove * cursorSpeed;
            report.y = leftYPolarity * leftyMove * cursorSpeed;
        }
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

        if (layer_state_is(LMOD)) {
            float cursorSpeed = 0.1 * get_speed(rightxMove, rightyMove);
            report.h = rightXPolarity * rightxMove * cursorSpeed;
            report.v = rightYPolarity * rightyMove * cursorSpeed;
        } else {
            float scrollSpeed = 1.7 * get_speed(rightxMove, rightyMove);
            report.x = rightXPolarity * rightxMove * scrollSpeed;
            report.y = rightYPolarity * rightyMove * scrollSpeed;
        }
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
