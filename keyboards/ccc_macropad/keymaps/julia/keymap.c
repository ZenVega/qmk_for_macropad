// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
//
#include "analog.h"
#include QMK_KEYBOARD_H

#define SLIDER_PIN 26
#define SLIDER_SENSITIVITY 12

static int16_t last_val = -1;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /*
     * ┌────┬────┬────┬────┐
     * │Sh1 │Sh2 │ ←  │ →  │   Layer 0 (default)
     * ├────┼────┼────┴────┘
     */
    [0] = LAYOUT(
        MO(1), MO(2),
        LGUI(LALT(KC_LEFT)), LGUI(LALT(KC_RIGHT))
    ),

    /*
     * ┌────┬────┬────┬────┐
     * │Sh1 │Sh2 │ ⇧← │ ⇧→ │   Layer 1 (shifted arrows)
     * ├────┼────┼────┴────┘
     */
    [1] = LAYOUT(
        MO(1), MO(2),
        LGUI(LSFT(LALT(KC_LEFT))), LGUI(LSFT(LALT(KC_RIGHT)))
    ),

    /*
     * ┌────┬────┬────┬────┐
     * │Sh1 │Sh2 │ ⌘⇥ │ ⌘⇧⇥│   Layer 2 (window switching)
     * ├────┼────┼────┴────┘
     */
    [2] = LAYOUT(
        MO(1), MO(2),
        LGUI(KC_TAB), LGUI(LSFT(KC_TAB))
    )
};


    void matrix_scan_user(void) {
    int16_t slider_val = analogReadPin(SLIDER_PIN);

    // Map to a smaller range so it’s not too sensitive
    int16_t step = slider_val / SLIDER_SENSITIVITY;

    // compare with last value and call (VOLUME_UP/VOLUME_DOWN) on change;
    if (step != last_val) {
        if (step > last_val) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
        last_val = step;
    }
}