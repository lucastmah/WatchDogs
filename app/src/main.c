#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/motionSensor.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "hal/panTilt.h"
#include "sendMail.h"
#include "camera_controls.h"
#include "commands.h"
#include "capture.h"

bool stop = false;

int main() {
    Gpio_addLineToBulk(SENSOR_CHIP, SENSOR_PIN, motionSensor_processState);
    
    Gpio_initialize();
    led_initialize();
    i2c_init();
    joystick_init();
    // panTilt_init();
    // CameraControls_init();
    capture_init();
    // commands_init(&stop);
    while(!stop) {
        sleep(1);
    }

    // commands_cleanup();
    capture_cleanup();
    // CameraControls_cleanup();
    // panTilt_cleanup();
    joystick_cleanup();
    i2c_cleanup();
    led_cleanup();
    Gpio_cleanup();
}