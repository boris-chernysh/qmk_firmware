#include "analog.h"
#include "pointing_device.h"
#include "math.h"
#include QMK_KEYBOARD_H

#define ONES(x) (fmax(fmin(x, 1.0), -1.0))

#define BASE 0
#define FN 1

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_ortho_5x14 (
            KC_AUDIO_VOL_UP, KC_GRAVE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL,
            KC_MEDIA_PLAY_PAUSE, KC_ESCAPE, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_RBRACKET,
            KC_AUDIO_VOL_DOWN, KC_LCTRL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, KC_BSLASH,
            KC_AUDIO_MUTE, KC_LALT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, KC_BSLASH, KC_DELETE,
            MO(FN), KC_LGUI, LSFT_T(KC_ESCAPE), LCTL_T(KC_TAB), KC_MS_BTN1, KC_ENTER, MO(FN), RSFT_T(KC_SPACE), KC_RCTRL, KC_BSPACE
            ),
    [FN] = LAYOUT_ortho_5x14 (
            _______, KC_FN1, KC_FN2, KC_FN3, KC_FN4, KC_FN5, KC_FN6, KC_FN7, KC_FN8, KC_FN9, KC_FN10, KC_FN11, KC_FN12, _______,
            _______,_______,_______,_______,_______,_______,_______,_______,KC_MS_BTN1, KC_MS_BTN2, KC_MS_BTN3,_______,_______,_______,
            RESET,_______,_______,_______,_______,_______,_______, KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT,_______,_______,_______,
            _______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,_______,
            _______,_______,_______,_______, KC_MS_BTN2, KC_PSCREEN,_______,_______,_______,_______
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
float get_speed(float multiplicator, int8_t xMove, int8_t yMove) {
    return multiplicator * (xMove * xMove + yMove * yMove) / 2 / square(127);
}

// by default: left joystick for scroll, right - for pointer
void pointing_device_task(void) {
    if (timer_elapsed(lastCursorTimer) < cursorTimeout) return;
    lastCursorTimer = timer_read();

    report_mouse_t report = pointing_device_get_report();

    int16_t leftXDelta = analogReadPin(leftXPin) - leftXCenter;
    int16_t leftYDelta = analogReadPin(leftYPin) - leftYCenter;
    if ( abs(leftXDelta) > leftxDead || abs(leftYDelta) > leftyDead ) {
        float leftxScale = leftXDelta > 0 ? 1023 - leftXCenter : leftXCenter;
        float leftyScale = leftYDelta > 0 ? 1023 - leftYCenter : leftYCenter;
        float xperc = ONES( (float)leftXDelta / leftxScale);
        float yperc = ONES( (float)leftYDelta / leftyScale);
        int8_t leftxMove = (int8_t)(xperc * 127.0);
        int8_t leftyMove = (int8_t)(yperc * 127.0);

        float scrollSpeed = get_speed(0.3, leftxMove, leftyMove);

        report.h = leftXPolarity * leftxMove * scrollSpeed;
        report.v = leftYPolarity * leftyMove * scrollSpeed;
    }

    int16_t rightXDelta = analogReadPin(rightXPin) - rightXCenter;
    int16_t rightYDelta = analogReadPin(rightYPin) - rightYCenter;
    if ( abs(rightXDelta) > rightxDead || abs(rightYDelta) > rightyDead ) {
        float rightxScale = rightXDelta > 0 ? 1023 - rightXCenter : rightXCenter;
        float rightyScale = rightYDelta > 0 ? 1023 - rightYCenter : rightYCenter;
        float xperc = ONES( (float)rightXDelta / rightxScale);
        float yperc = ONES( (float)rightYDelta / rightyScale);

        int8_t rightxMove = (int8_t)(xperc * 127.0);
        int8_t rightyMove = (int8_t)(yperc * 127.0);

        float cursorSpeed = get_speed(1.7, rightxMove, rightyMove);

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
