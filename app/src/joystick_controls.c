#include "joystick_controls.h"
#include "utils.h"
#include "hal/joystick.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define DEBOUNCE_TIME_MS 300
#define MAX_JOYSTICK_VAL 100
#define MIN_JOYSTICK_VAL 0
#define JOYSTICK_STEP 5

static pthread_t joystick_thread;
static bool stop_sampling = false;
static bool is_initialized = false;

void* JoystickControls_Thread(void* args) {
    (void) args;
    assert(is_initialized);

    printf("starting joystick sampling thread\n");

    // long long time_now, latest_time_of_event = getTimeInMs();
    // int new_val, old_val;
    
    while (!stop_sampling) {
        struct joystickState state = joystick_getState();
        sleepForMs(500);
        printf("x=%.3f y=%.3f\n", state.X, state.Y);
    }
    return NULL;
}

void JoystickControls_init(void) {
    assert(!is_initialized);
    is_initialized = true;

    if (pthread_create(&joystick_thread, NULL, &JoystickControls_Thread, NULL) != 0) {
        perror("Unable to initialize joystick sampling thread");
        exit(EXIT_FAILURE);
    }
}
void JoystickControls_cleanup(void) {
    stop_sampling = true;
    pthread_join(joystick_thread, NULL);

    assert(is_initialized);
    is_initialized = false;
}
