#ifndef _GPIO_H_
#define _GPIO_H_

// Low-level GPIO access using gpiod

#include <stdbool.h>
#include <gpiod.h>


enum eGpioChips {
    GPIO_CHIP_0,
    GPIO_CHIP_1,
    GPIO_CHIP_2,
    GPIO_NUM_CHIPS      // Count of chips
};

struct gpiolines {
    unsigned int chip;
    unsigned int pin;
    void (*action)(int chip, int pin, bool is_rising);
};

enum eGpioLines {
    // ROTARY_A,
    // ROTARY_B,
    // ROTARY_PUSH,
    // JOYSTICK_PUSH,
    MOTION_SENSOR,
    GPIO_NUM_LINES
};

struct gpioEvents {
    int line;
    bool isRising;
};

// Must initialize before calling any other functions.
void Gpio_initialize(void);

void Gpio_cleanup(void);

void Gpio_close(struct gpiod_line* line);

void Gpio_addLineToBulk(int chip, int pin, void (*action)(int chip, int pin, bool is_rising));

#endif