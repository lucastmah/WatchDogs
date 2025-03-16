#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/motion_sensor.h"
#include "hal/gpio.h"

void toggle_LED(bool is_on) {
    led_setBrightness(BYAI_RED, is_on);
}

int main() {
    motionSensor_addSubscriber(toggle_LED);
    Gpio_initialize();
    led_initialize();
    
    while(1);

    led_cleanup();
    Gpio_cleanup();
}