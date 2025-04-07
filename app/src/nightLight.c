#include "nightLight.h"
#include "utils.h"
#include "sharedMem.h"
#include "hal/motionSensor.h"
#include "hal/gpio.h"
#include "hal/timeout.h"
#include "hal/lightSensor.h"
#include "hal/led.h"
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#define LIT_ROOM 25
#define LIGHT_ON_LENGTH 5000 // Represented in ms

static _Atomic bool enableLight = false;
static _Atomic bool lightsEvent = false;
static _Atomic bool eventLockout = false;
static pthread_mutex_t motion_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t thread_id;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static long long start_countdown = -1;

static int on_arr[8] = {RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT, RED_BRIGHT};
static int off_arr[8] = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

static void* nightLight_loop() {
    while (enableLight) {
        if (lightsEvent) {
            lightsEvent = false;
            start_countdown = getTimeInMs();
        } 
        if (start_countdown != -1 && (getTimeInMs() - start_countdown > LIGHT_ON_LENGTH)) {
            start_countdown = -1;
            // turns off the lights
            sharedMem_setValues(off_arr);
        }
        sleep(1);
    }
    return NULL;
}

void nightLight_processEvent(bool isRising) {
    bool isDark = lightSensor_getReading() < LIT_ROOM;
    if (enableLight && isDark && isRising) {
        // turns on the lights
        sharedMem_setValues(on_arr);
    }
    if (enableLight && !isRising && !eventLockout) {
        lightsEvent = true;
        timeout_start_timer(&eventLockout);
    }
}

static void nightLight_turnOn(void) {
    pthread_mutex_lock(&thread_mutex);
    if (!enableLight) {
        enableLight = true;
        if (pthread_create(&thread_id, NULL, nightLight_loop, NULL) != 0) {
            perror("failed to create night light thread");
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&thread_mutex);
}

static void nightLight_turnOff(void) {
    pthread_mutex_lock(&thread_mutex);
    if (enableLight) {
        enableLight = false;
        if (pthread_join(thread_id, NULL) != 0) {
            perror("failed to join night light thread");
            exit(EXIT_FAILURE);
        }
        sharedMem_setValues(off_arr);
    }
    pthread_mutex_unlock(&thread_mutex);
}

void nightLight_setLightMode(bool val) {
    if (val) {
        nightLight_turnOn();
    }
    else {
        nightLight_turnOff();
    }
}

bool nightLight_getLightMode(void) {
    return enableLight;
}

void nightLight_init(void) {
    motionSensor_addSubscriber(nightLight_processEvent);
    sharedMem_setValues(off_arr);
}