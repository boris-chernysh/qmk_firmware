#include "analog.h"
#include "pointing_device.h"
#include "math.h"
#include QMK_KEYBOARD_H

#define ONES(x) (fmax(fmin(x, 1.0), -1.0))

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    LAYOUT_ortho_5x14 (
        KC_AUDIO_VOL_UP, KC_GRAVE, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINUS, KC_EQUAL,
        KC_MEDIA_PLAY_PAUSE, KC_ESCAPE, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRACKET, KC_LBRACKET,
        KC_AUDIO_VOL_DOWN, KC_LCTRL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON, KC_QUOTE, KC_ENTER,
        KC_AUDIO_MUTE, KC_LALT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH, KC_BSLASH, KC_DELETE,
        KC_NO, KC_LGUI, KC_LSHIFT, KC_TAB, KC_MS_BTN1, KC_MS_BTN2, KC_SPACE, KC_RSHIFT, KC_BSPACE, KC_NO
    )
};

const pin_t leftXPin = F1;
const pin_t leftYPin = F0;
int8_t leftXPolarity = 1;
int8_t leftYPolarity = 1;

const pin_t rightXPin = B4;
const pin_t rightYPin = B5;
int8_t rightXPolarity = 1;
int8_t rightYPolarity = 1;


uint16_t xScale = 128;
uint16_t yScale = 128;
uint16_t xDead = 50;
uint16_t yDead = 50;
int16_t leftXCenter;
int16_t leftYCenter;
int16_t rightXCenter;
int16_t rightYCenter;

float maxCursorSpeed = 0.9;
float minCursorSpeed = 0.2;
float maxScrollSpeed = 0.3;
float minScrollSpeed = 0.1;

uint8_t cursorTimeout = 50;
uint16_t lastCursorTimer = 0;


// by default: left joystick for scroll, right - for pointer
// with GUI button vice versa
void pointing_device_task(void) {
    if (timer_elapsed(lastCursorTimer) < cursorTimeout) return;
    lastCursorTimer = timer_read();

    report_mouse_t report = pointing_device_get_report();

    int16_t leftXDelta = analogReadPin(leftXPin) - leftXCenter;
    int16_t leftYDelta = analogReadPin(leftYPin) - leftYCenter;

    int16_t rightXDelta = analogReadPin(rightXPin) - rightXCenter;
    int16_t rightYDelta = analogReadPin(rightYPin) - rightYCenter;

    if ( abs(leftXDelta) > xDead || abs(leftYDelta) > yDead ) {
        float xperc = ONES( (float)leftXDelta / xScale);
        float yperc = ONES( (float)leftYDelta / yScale);
        int8_t leftxMove = (int8_t)(xperc * 127.0);
        int8_t leftyMove = (int8_t)(yperc * 127.0);

        if (get_mods() & MOD_MASK_GUI) {
            float scroll_speed = (get_mods() & MOD_MASK_SHIFT) ? maxScrollSpeed : minScrollSpeed;
            report.h = leftXPolarity * leftxMove * scroll_speed;
            report.v = rightXPolarity * leftyMove * scroll_speed;
        } else {
            float cursorSpeed = (get_mods() & MOD_MASK_SHIFT) ? maxCursorSpeed : minCursorSpeed;
            report.x = leftXPolarity * leftxMove * cursorSpeed;
            report.y = leftYPolarity * leftyMove * cursorSpeed;
        }
    }

    if ( abs(rightXDelta) > xDead || abs(rightYDelta) > yDead ) {
        float xperc = ONES( (float)rightXDelta / xScale);
        float yperc = ONES( (float)rightYDelta / yScale);
        int8_t rightxMove = (int8_t)(xperc * 127.0);
        int8_t rightyMove = (int8_t)(yperc * 127.0);

        if (get_mods() & MOD_MASK_GUI) {
            float scroll_speed = (get_mods() & MOD_MASK_SHIFT) ? maxScrollSpeed : minScrollSpeed;
            report.h = rightXPolarity * rightxMove * scroll_speed;
            report.v = rightYPolarity * rightyMove * scroll_speed;
        } else {
            float cursorSpeed = (get_mods() & MOD_MASK_SHIFT) ? maxCursorSpeed : minCursorSpeed;
            report.x = rightXPolarity * rightxMove * cursorSpeed;
            report.y = rightYPolarity * rightyMove * cursorSpeed;
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
