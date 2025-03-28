#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/motionSensor.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "sendMail.h"
#include "joystick_controls.h"

void toggle_LED(bool is_on) {
    led_setBrightness(BYAI_RED, is_on);
}

int main() {
    motionSensor_addSubscriber(toggle_LED);
    Gpio_addLineToBulk(SENSOR_CHIP, SENSOR_PIN, motionSensor_processState);
    Gpio_initialize();
    led_initialize();
    i2c_init();
    joystick_init();
    JoystickControls_init();
    
    sendMail_send("lucastmah@gmail.com");
    while(1) {
        i2c_getBH1750Value();
        sleep(1);
    }

    JoystickControls_cleanup();
    joystick_cleanup();
    led_cleanup();
    Gpio_cleanup();
}