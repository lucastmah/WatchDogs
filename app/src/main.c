#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/motion_sensor.h"
#include "hal/gpio.h"
#include "hal/i2c.h"

void toggle_LED(bool is_on) {
    led_setBrightness(BYAI_RED, is_on);
}

int main() {
    motionSensor_addSubscriber(toggle_LED);
    Gpio_addLineToBulk(SENSOR_CHIP, SENSOR_PIN, motionSensor_processState);
    Gpio_initialize();
    led_initialize();
    i2c_init();
    
    while(1) {
        i2c_getBH1750Value();
        sleep(1);
    }

    led_cleanup();
    Gpio_cleanup();
}