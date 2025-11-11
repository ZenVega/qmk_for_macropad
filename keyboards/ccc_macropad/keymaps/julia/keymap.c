// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "analog.h"
#include QMK_KEYBOARD_H

// ----------------------
// slider setup
// ----------------------
#define SLIDER_PIN 26
#define MAX_VOLUME_STEPS 100
#define SLIDER_DEADBAND 2     // ignore <2 steps of change
static bool volume_init_done = false;
static bool slider_ready = false;
static uint32_t slider_timer = 0;
static uint32_t init_timer = 0;
static int16_t last_val = 0;

// ----------------------
// LEDs
// ----------------------
#define LED1_PIN 29 //left LED
#define LED2_PIN 27
#define LED3_PIN 28 //right LED

// ----------------------
// OS Switch
// ----------------------
#define OS_SWITCH_PIN GP3
static bool is_mac = false;

// ----------------------
// helpers to hold tab
// ----------------------
static bool gui_held = false;
static uint32_t last_tab_time = 0;
#define GUI_HOLD_TIMEOUT 1000 // ms

// ----------------------
// Custom keycodes
// ----------------------
enum custom_keys {
    KC_WS_LEFT,
    KC_WS_RIGHT,
    KC_MOVE_LEFT,
    KC_MOVE_RIGHT
};


// ----------------------
// Keymap
// ----------------------
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* Layer 0: Workspace switching */
    [0] = LAYOUT(
        MO(1), MO(2),
        KC_WS_LEFT, KC_WS_RIGHT
    ),

    /* Layer 1: Move window between workspaces */
    [1] = LAYOUT(
        _______, MO(2),                         // "_______" describes a transparent key 
        KC_MOVE_LEFT, KC_MOVE_RIGHT
    ),

    /* Layer 2: Switch windows */
    [2] = LAYOUT(
        MO(1),_______,
        LGUI(KC_TAB), LGUI(LSFT(KC_TAB))
    ),

    /* Layer 3: sleep*/
    [3] = LAYOUT(
        _______, _______,
        KC_SYSTEM_SLEEP, KC_SYSTEM_SLEEP
    )
};

// ----------------------
//called once at boot
// ----------------------
void matrix_init_user(void) 
{
    //Initialize OS Switch
    setPinInputHigh(OS_SWITCH_PIN);

    // Initialize LEDs
    setPinOutput(LED1_PIN);
    setPinOutput(LED2_PIN);
    setPinOutput(LED3_PIN); 
    
    // initial happy blinking
    int i = 0;
    while (i < 10)
    {
        writePinHigh(LED1_PIN);
        writePinLow(LED2_PIN);
        writePinHigh(LED3_PIN);
        wait_ms(200);
        writePinLow(LED1_PIN);
        writePinHigh(LED2_PIN);
        writePinLow(LED3_PIN);
        wait_ms(200);
        i++;
    }
    writePinLow(LED2_PIN);
}

// ----------------------
// called once after boot is done
// ----------------------
void post_init_user(void)
{
    /*
        to have the intial blinking here does not work,
        checking out the layer state immediately starts after boot
    */
}

//-----------
// called when layer changes
// --------------
layer_state_t layer_state_set_user(layer_state_t state) 
{
    state = update_tri_layer_state(state, 1, 2, 3);

    uint8_t layer = get_highest_layer(state);

    writePinLow(LED1_PIN);
    writePinLow(LED2_PIN);
    writePinLow(LED3_PIN);

    switch(layer) 
    {
        case 0: 
            writePinHigh(LED1_PIN); 
            break;
        case 1: 
            writePinHigh(LED2_PIN);
            break;
        case 2:
            writePinHigh(LED3_PIN);
            break;
        case 3:
            writePinHigh(LED1_PIN);
            writePinHigh(LED2_PIN);
            writePinHigh(LED3_PIN);
            break;

    }
    return (state);
}


// ----------------------
// repeated loop, constantly called while running
// ----------------------
void matrix_scan_user(void) {

    // ------ OS_Switch -------- //
    bool new_mode = !readPin(OS_SWITCH_PIN); // HIGH = macOS, LOW = Linux
    if (new_mode != is_mac) {
        is_mac = new_mode;
    }
    
    // ------ hold tab for a while after pressed -------- //
    if (gui_held && timer_elapsed32(last_tab_time) > GUI_HOLD_TIMEOUT) {
        unregister_mods(MOD_BIT(KC_LGUI));
        gui_held = false;
    }

    // ------ volume init -------- //
    if (!volume_init_done) {
        if (!init_timer) {
            init_timer = timer_read32();
        }
        if (timer_elapsed32(init_timer) > 800) {
            for (int i = 0; i < 50; i++) {
                tap_code_delay(KC_VOLD, 5);
            }
            last_val = 0;
            volume_init_done = true;
            slider_timer = timer_read32(); 
        }
        return;
    }

    // ------ start slider delayed -------- //
    if (!slider_ready) {
        if (timer_elapsed32(slider_timer) > 500) {
            slider_ready = true;
        } else {
            return;
        }
    }

    // ------ slider processing -------- //    
    int16_t raw = analogReadPin(SLIDER_PIN);
    int target = (int)(raw * MAX_VOLUME_STEPS / 4095.0f);

    if (target < 0) target = 0;
    if (target > MAX_VOLUME_STEPS) target = MAX_VOLUME_STEPS;

    if (abs(target - last_val) <= SLIDER_DEADBAND) return; //noise filter

    if (target > last_val) {
        for (int i = last_val; i < target; i++)
            tap_code(KC_AUDIO_VOL_UP);
    } else {
        for (int i = target; i < last_val; i++)
            tap_code(KC_AUDIO_VOL_DOWN);
    }

    last_val = target;
}

// ----------------------
// Called on every keypress - Process custom keys
// ----------------------
bool process_record_user(uint16_t keycode, keyrecord_t *record) 
{
    if (!record->event.pressed) 
        return true;
    
    if (record->event.pressed) {
        if (keycode == KC_SYSTEM_SLEEP) {
            // sleep animation
            for (uint8_t i = 0; i < 5; i++) {
                writePinHigh(LED1_PIN);
                writePinHigh(LED2_PIN);
                writePinHigh(LED3_PIN);
                wait_ms(200);

                writePinLow(LED1_PIN);
                writePinLow(LED2_PIN);
                writePinLow(LED3_PIN);
                wait_ms(200);
            }
        }
    }

    switch (keycode) {
        case KC_WS_LEFT:
            tap_code16(is_mac ? LCTL(KC_LEFT)
                              : LGUI(LALT(KC_LEFT)));
            return false;

        case KC_WS_RIGHT:
            tap_code16(is_mac ? LCTL(KC_RIGHT)
                              : LGUI(LALT(KC_RIGHT)));
            return false;

        case KC_MOVE_LEFT:
            // macOS: no moving windows → fallback to switching
            tap_code16(is_mac ? LCTL(KC_LEFT)
                              : LGUI(LSFT(LALT(KC_LEFT))));
            return false;

        case KC_MOVE_RIGHT:
            tap_code16(is_mac ? LCTL(KC_RIGHT)
                              : LGUI(LSFT(LALT(KC_RIGHT))));
            return false;
        
        case LGUI(KC_TAB):
        case LGUI(LSFT(KC_TAB)):
        {
            if (!gui_held) {
                register_mods(MOD_BIT(KC_LGUI)); //hold LGUI / LCMD
                gui_held = true;
            }

            if (keycode == LGUI(KC_TAB)) {
                tap_code(KC_TAB);        // forward
            } else {
                register_mods(MOD_BIT(KC_LSFT));  // hold Shift for reverse
                tap_code(KC_TAB);
                unregister_mods(MOD_BIT(KC_LSFT));
            }

            last_tab_time = timer_read32();
            return false;
        }
    }
    return true;
}

