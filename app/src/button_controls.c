#include "button_controls.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "hal/R5.h"
#include "camera_controls.h"
#include "lcd.h"
#include "nightLight.h"

static bool is_initialized = false;
static bool stop_sampling = false;
pthread_t buttoncontrols_thread;
static bool joystick_currently_pressed = false, joystick_was_pressed = false;
static bool encoder_currently_pressed = false, encoder_was_pressed = false;

void* ButtonControls_Thread(void* args) {
    (void) args;
    assert(is_initialized);

    while (!stop_sampling) {
        // Toggle patrol mode on release of joystick press
        bool joystick_currently_pressed = R5_getJoystickButtonState();
        if (!joystick_currently_pressed && joystick_was_pressed) {
            joystick_was_pressed = false;
            CameraControls_togglePatrolMode();
        } else if (joystick_currently_pressed){
            if (!joystick_was_pressed) {
                lcd_wakeScreen();
            }
            joystick_was_pressed = true;
        }

        // Toggle nightlight mode on release of encoder press
        bool encoder_currently_pressed = R5_getEncButtonState();
        if (!encoder_currently_pressed && encoder_was_pressed) {
            encoder_was_pressed = false;
            nightLight_toggleLightMode();
        } else if (encoder_currently_pressed){
            if (!encoder_was_pressed) {
                lcd_wakeScreen();
            }
            encoder_was_pressed = true;
        }
    }
}

void ButtonControls_init(void) {
    assert(!is_initialized);
    is_initialized = true;

    if (pthread_create(&buttoncontrols_thread, NULL, &ButtonControls_Thread, NULL) != 0) {
        perror("Unable to initialize button control thread");
        exit(EXIT_FAILURE);
    }
}

void ButtonControls_cleanup(void) {
    stop_sampling = true;
    pthread_join(buttoncontrols_thread, NULL);

    assert(is_initialized);
    is_initialized = false;
}