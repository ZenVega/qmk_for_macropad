// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┐
     * │ 1 │ 2 │ 3 │
     * ├───┼───┼───┤
     * │ 5 │ 6 │ 7 │
     * └───┴───┴───┘
     */
    [0] = LAYOUT(KC_1, KC_2, KC_3,   // ROW1
                 KC_4, KC_5, KC_6)}; // ROW2
                                     //

//in order to read the analog input,
//analogReadPin(GP26);
