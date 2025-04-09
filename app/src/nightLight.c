#include "nightLight.h"
#include "utils.h"
#include "hal/motionSensor.h"
#include "hal/lightSensor.h"
#include "hal/led.h"
#include "hal/R5.h"
#include "utils.h"

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define LIT_ROOM 25
#define LIGHT_ON_LENGTH 5000  // ms
#define DEBOUNCE_TIME_MS 150  // ms

static _Atomic bool enableLight = false;
static _Atomic bool motionDetected = false;
static _Atomic bool stop = false;

static pthread_t thread_id;
static bool is_initialized = false;

static long long lightOffTime = -1;
static long long lastMotionEvent = -1;

static enum Colour on_arr[8] = {
    RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT,
    RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT
};
static enum Colour off_arr[8] = {
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF
};

static void* nightLight_loop(void* arg) {
    (void)arg;

    while (!stop) {
        sleepForMs(100);
        if (!enableLight) continue;

        long long time_now = getTimeInMs();
        if (motionDetected) {
            motionDetected = false;

            if (time_now - lastMotionEvent > DEBOUNCE_TIME_MS) {
                lastMotionEvent = time_now;

                // Turn on nightlight
                if (lightSensor_getReading() < LIT_ROOM) {
                    R5_setLEDs(on_arr);
                    lightOffTime = time_now + LIGHT_ON_LENGTH;
                }
            }
        }
        // Handle when light needs to turn off from previous event
        if (lightOffTime != -1 && time_now >= lightOffTime) {
            R5_setLEDs(off_arr);
            lightOffTime = -1;
        }
    }

    return NULL;
}

void nightLight_processEvent(bool isRising) {
    assert(is_initialized);
    if (enableLight && isRising) {
        motionDetected = true;
    }
}

bool nightLight_setLightMode(bool val) {
    assert(is_initialized);
    enableLight = val;

    if (!val) {
        R5_setLEDs(off_arr);
        lightOffTime = -1;
    }
    return val;
}

void nightLight_toggleLightMode(void) {
    assert(is_initialized);
    nightLight_setLightMode(!enableLight);
}

bool nightLight_getLightMode(void) {
    assert(is_initialized);
    return enableLight;
}

void nightLight_init(void) {
    assert(!is_initialized);
    motionSensor_addSubscriber(nightLight_processEvent);
    R5_setLEDs(off_arr);
    is_initialized = true;

    if (pthread_create(&thread_id, NULL, nightLight_loop, NULL) != 0) {
        perror("failed to create nightLight thread");
        exit(EXIT_FAILURE);
    }
}

void nightLight_cleanup(void) {
    assert(is_initialized);
    enableLight = false;
    stop = true;

    if (pthread_join(thread_id, NULL) != 0) {
        perror("failed to join nightLight thread");
        exit(EXIT_FAILURE);
    }

    R5_setLEDs(off_arr);
    is_initialized = false;
}