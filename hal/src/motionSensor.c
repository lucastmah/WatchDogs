#include "hal/motionSensor.h"
#include "hal/gpio.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <gpiod.h>
#include "hal/gpio.h"
#include "hal/led.h"

#define MAX_SUBSCRIBERS 5

static _Atomic bool motion_detected = false;

static void (*subscribers[MAX_SUBSCRIBERS]) (bool motion_state);
static int sub_count = 0;

static int is_initialized = false;

void motionSensor_addSubscriber(void (*callback)(bool motion_state)) {
    assert(!is_initialized);
    if (sub_count < MAX_SUBSCRIBERS) {
        subscribers[sub_count] = callback;
        sub_count++;
    }
}

void motionSensor_processState(int chip, int pin, bool is_rising) {
    assert(is_initialized);
    if (chip == SENSOR_CHIP && pin == SENSOR_PIN) {
        if (is_rising) {
            motion_detected = true;
        }
        else {
            motion_detected = false;
        }
        for (int i = 0; i < sub_count; i++) {
            subscribers[i](motion_detected);
        }
    }
}

void motionSensor_init(void) {
    assert(!is_initialized);
    is_initialized = true;
    Gpio_addLineToBulk(SENSOR_CHIP, SENSOR_PIN, motionSensor_processState);
}

void motionSensor_cleanup(void) {
    assert(is_initialized);
    is_initialized = false;
}