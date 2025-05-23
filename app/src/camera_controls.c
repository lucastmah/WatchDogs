#include "camera_controls.h"
#include "utils.h"
#include "lcd.h"
#include "hal/joystick.h"
#include "hal/panTilt.h"
#include "hal/R5.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdatomic.h>

#define MS_BETWEEN_SAMPLES 1

static pthread_t cameracontrols_thread;
static bool stop_sampling = false;
static bool is_initialized = false;

static atomic_bool is_patrolling = false;
static int patrol_direction = 1;

void* CameraControls_Thread(void* args) {
    (void) args;
    assert(is_initialized);

    while (!stop_sampling) {
        struct joystickState state = joystick_getState();
        if (state.X == 0 && state.Y == 0) {
            // Patrol only if no manual camera movement and set to patrol mode
            if (is_patrolling) {
                panTilt_resetAxis(TILT);
                bool servo_moved = panTilt_setPercent(PAN, patrol_direction * 20);
                // Switch directions once servo reaches max or min pan
                patrol_direction = servo_moved ? patrol_direction : -patrol_direction;
            }
        } else {
            is_patrolling = false;
            lcd_wakeScreen();
            // servo higher number = left for x, down for y
            panTilt_setPercent(PAN, -state.X);
            panTilt_setPercent(TILT, -state.Y);
        }
        sleepForMs(MS_BETWEEN_SAMPLES);
    }
    return NULL;
}

bool CameraControls_setPatrolMode(bool value) {
    is_patrolling = value;
    return value;
}

void CameraControls_togglePatrolMode(void) {
    is_patrolling = !is_patrolling;
}

bool CameraControls_getPatrolMode(void) {
    return is_patrolling;
}

void CameraControls_pan(int direction) {
    assert(is_initialized);
    is_patrolling = false;
    // servo higher number = left
    panTilt_setPercent(PAN, -direction * 100);
}

void CameraControls_tilt(int direction) {
    assert(is_initialized);
    is_patrolling = false;
    // servo higher number = down
    panTilt_setPercent(TILT, -direction * 100);
}

void CameraControls_init(void) {
    assert(!is_initialized);
    is_initialized = true;

    if (pthread_create(&cameracontrols_thread, NULL, &CameraControls_Thread, NULL) != 0) {
        perror("Unable to initialize camera control thread");
        exit(EXIT_FAILURE);
    }
}
void CameraControls_cleanup(void) {
    stop_sampling = true;
    pthread_join(cameracontrols_thread, NULL);

    assert(is_initialized);
    is_initialized = false;
}
