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
#include "lcd.h"
#include "sendMail.h"
#include "camera_controls.h"
#include "commands.h"
#include "capture.h"
#include "nightLight.h"

bool stop = false;

int main() {
    nightLight_init();
    motionSensor_init();
    Gpio_initialize();
    led_initialize();
    i2c_init();
    joystick_init();
    // panTilt_init();
    // CameraControls_init();
    capture_init();
    lcd_init();
    commands_init(&stop);

    while(!stop);

    commands_cleanup();
    lcd_cleanup();
    capture_cleanup();
    // CameraControls_cleanup();
    // panTilt_cleanup();
    joystick_cleanup();
    i2c_cleanup();
    led_cleanup();
    Gpio_cleanup();
}