#include "camera_controls.h"
#include "utils.h"
#include "hal/joystick.h"
#include "hal/panTilt.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define MS_BETWEEN_SAMPLES 1

static pthread_t cameracontrols_thread;
static bool stop_sampling = false;
static bool is_initialized = false;

void* CameraControls_Thread(void* args) {
    (void) args;
    assert(is_initialized);
    
    while (!stop_sampling) {
        struct joystickState state = joystick_getState();
        // servo higher number = left for x, down for y
        panTilt_setPercent(PAN, -state.X);
        panTilt_setPercent(TILT, -state.Y);
        sleepForMs(MS_BETWEEN_SAMPLES);
    }
    return NULL;
}

void CameraControls_pan(int direction) {
    assert(is_initialized);
    // servo higher number = left
    panTilt_setPercent(PAN, -direction * 100);
}

void CameraControls_tilt(int direction) {
    assert(is_initialized);
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
