#include "nightLight.h"
#include "hal/motionSensor.h"
#include "hal/led.h"
#include <stdbool.h>

static _Atomic bool enableLight = false;
static bool isLightOn = false;

void nightLight_processEvent(bool isRising) {
    if (enableLight) {

    }
}

void nightLight_turnOn(void) {
    enableLight = true;
}

void nightLight_turnOff(void) {
    enableLight = false;
}

void nightLight_init(void) {
    motionSensor_addSubscriber(nightLight_processEvent);
}