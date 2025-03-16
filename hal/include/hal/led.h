#ifndef LED_H_

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

void led_initialize(void);

void led_cleanup(void);

void led_reset(enum LED led);

void led_setBrightness(enum LED led, bool activation);

void led_setBlinkInterval(enum LED led, int time_on, int time_off);

#endif