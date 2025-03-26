/*

HOW TO USE:

Need to call led_initialize to setup leds first.

Call led_cleanup when shutting down.

*/

#ifndef LED_H_
#define LED_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#define BUFFER_SIZE 100

enum LED {
    BYAI_RED,
    BYAI_GREEN
};

// Initializes leds
void led_initialize(void);

// Prepares leds for shutdown
void led_cleanup(void);

// Resets the state for a given led
void led_reset(enum LED led);

// Turns on or off an led
void led_setBrightness(enum LED led, bool activation);

// Sets the blink interval of a given led by providing the amount of time it should be on and off for each interval
void led_setBlinkInterval(enum LED led, int time_on, int time_off);

#endif