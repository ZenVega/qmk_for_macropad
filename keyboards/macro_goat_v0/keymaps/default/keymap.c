// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#define STARTUP_MS 3000
#define BLINK_MS   150
#define NUM_KEYS   (MATRIX_ROWS * MATRIX_COLS)

static uint32_t startup_timer           = 0;
static bool     startup_done            = false;
static uint32_t key_blink_timer[NUM_KEYS] = {0};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8)
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = {
        ENCODER_CCW_CW(KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP),
        ENCODER_CCW_CW(KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP),
        ENCODER_CCW_CW(KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP),
        ENCODER_CCW_CW(KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP)
    }
};
#endif

void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();
    rgblight_sethsv_noeeprom(0, 255, 180);
    rgblight_mode_noeeprom(RGBLIGHT_MODE_RAINBOW_SWIRL);
    startup_timer = timer_read32();
}

void matrix_scan_user(void) {
    if (!startup_done && timer_elapsed32(startup_timer) >= STARTUP_MS) {
        rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
        for (uint8_t i = 0; i < RGBLIGHT_LED_COUNT; i++) {
            rgblight_sethsv_at(0, 0, 0, i);
        }
        startup_done = true;
    }

    if (startup_done) {
        for (uint8_t i = 0; i < NUM_KEYS; i++) {
            if (key_blink_timer[i] != 0 && timer_elapsed32(key_blink_timer[i]) >= BLINK_MS) {
                rgblight_sethsv_at(0, 0, 0, i);
                key_blink_timer[i] = 0;
            }
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed && startup_done) {
        uint8_t key_idx = record->event.key.row * MATRIX_COLS + record->event.key.col;
        if (key_idx < NUM_KEYS) {
            rgblight_sethsv_at(0, 0, 200, key_idx);
            key_blink_timer[key_idx] = timer_read32();
        }
    }
    return true;
}
