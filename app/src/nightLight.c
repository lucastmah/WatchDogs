#include "nightLight.h"
#include "utils.h"
#include "hal/motionSensor.h"
#include "hal/gpio.h"
#include "hal/led.h"
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

static _Atomic bool enableLight = false;
static bool isLightOn = false;
static pthread_mutex_t motion_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t thread_id;
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static long long start_countdown = -1;

static void* nightLight_loop() {
    while (enableLight) {
        if (pthread_mutex_trylock(&motion_mutex) != 0) {
            pthread_mutex_unlock(&motion_mutex);
            start_countdown = getTimeInMs();
        } 
        else if (start_countdown != -1 && getTimeInMs() - start_countdown > 10000) {
            start_countdown = -1;
            led_setBrightness(BYAI_RED, false);
        }
    }
    return NULL;
}

void nightLight_processEvent(bool isRising) {
    if (enableLight) {
        if (isRising) {
            led_setBrightness(BYAI_RED, true);
            pthread_mutex_lock(&motion_mutex);
        }
    }
}

static void nightLight_turnOn(void) {
    pthread_mutex_lock(&thread_mutex);
    assert(!enableLight);
    enableLight = true;
    if (pthread_create(&thread_id, NULL, nightLight_loop, NULL) != 0) {
        perror("failed to create night light thread");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&thread_mutex);
}

static void nightLight_turnOff(void) {
    pthread_mutex_lock(&thread_mutex);
    assert(!enableLight);
    enableLight = false;
    if (pthread_join(thread_id, NULL) != 0) {
        perror("failed to join night light thread");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&thread_mutex);
}

void nightLight_setLightMode(int param) {
    if (param == 1) {
        nightLight_turnOn();
    }
    else {
        nightLight_turnOff();
    }
}

void nightLight_init(void) {
    motionSensor_addSubscriber(nightLight_processEvent);
}