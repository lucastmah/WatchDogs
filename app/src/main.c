#include <stdio.h>

#include "hal/joystick.h"
#include "hal/motionSensor.h"
#include "hal/lightSensor.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "hal/panTilt.h"
#include "hal/R5.h"
#include "lcd.h"
#include "sendMail.h"
#include "camera_controls.h"
#include "button_controls.h"
#include "udp_commands.h"
#include "video_capture.h"
#include "nightLight.h"
#include "shake_detection.h"
#include "audio_proc.h"

bool stop = false;

int main() {
    printf("Starting WatchDog...\n");

    // HAL module initialization
    i2c_init();
    joystick_init();
    R5_init();
    panTilt_init();

    // GPIO only initialized after subscribers added
    lightSensor_init();
    nightLight_init(); // nightLight subscribes to motionSensor & uses lightSensor
    motionSensor_init();
    Gpio_initialize();

    // Main module initialization
    lcd_init();
    CameraControls_init();
    ButtonControls_init();

    capture_init();
    AudioProc_init();
    shakeDetect_init();
    UDPCommands_init(&stop);

    while(!stop);

    // Cleanup initiated
    printf("Exiting WatchDog...\n");

    // Main module cleanup
    UDPCommands_cleanup();
    shakeDetect_cleanup();
    AudioProc_cleanup();
    capture_cleanup();
    ButtonControls_cleanup();
    CameraControls_cleanup();
    lcd_cleanup();

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