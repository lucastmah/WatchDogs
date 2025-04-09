#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "hal/joystick.h"
#include "hal/led.h" // Unused
#include "hal/motionSensor.h"
#include "hal/lightSensor.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "hal/panTilt.h"
#include "hal/rotary.h"
#include "hal/R5.h"
// #include "audio_proc.h"
#include "lcd.h"
#include "sendMail.h"
#include "camera_controls.h"
#include "udp_commands.h"
#include "capture.h"
#include "nightLight.h"
#include "menu.h"
#include "shake.h"

bool stop = false;

int main() {
    printf("Starting WatchDog...\n");

    // HAL module initialization
    i2c_init();
    joystick_init();
    R5_init();
    panTilt_init();

    // GPIO only initialized after subscribers added
    nightLight_init(); // nightLight subscribes to motionSensor & uses lightSensor
    lightSensor_init();
    motionSensor_init();
    Gpio_initialize();

    // Main module initialization
    CameraControls_init();

    // rotary_init();
    // capture_init();
    // AudioProc_init();
    // shake_init();
    // lcd_init();
    // menu_init();
    UDPCommands_init(&stop);

    while(!stop);

    // Cleanup initiated
    printf("Exiting WatchDog...\n");

    // Main module cleanup
    UDPCommands_cleanup();
    // lcd_cleanup();
    // AudioProc_cleanup();
    // shake_cleanup();
    // capture_cleanup();
    CameraControls_cleanup();

    // GPIO subscribers cleanup after
    Gpio_cleanup();
    motionSensor_cleanup();
    nightLight_cleanup();
    lightSensor_cleanup(); // Night light depends on lightsensor

    // HAL module cleanup
    panTilt_cleanup();
    joystick_cleanup();
    i2c_cleanup();
    R5_cleanup();

}