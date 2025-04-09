// R5 module
// Part of the Hardware Abstraction Layer (HAL)

#ifndef _R5_H_
#define _R5_H_

#include <stdbool.h>

enum Colour {
    GREEN,
    RED,
    BLUE,
    ORANGE,
    YELLOW,
    GREEN_BRIGHT,
    RED_BRIGHT,
    BLUE_BRIGHT,
    OFF,
    COUNT
};
#define NEO_NUM_LEDS          8   // # LEDs in our string
#define BRIGHT_OFFSET GREEN_BRIGHT - GREEN // Offset in enum from normal to bright colour version

void R5_init(void);
void R5_cleanup(void);

// Set LEDs to colours specified by input
void R5_setLEDs(enum Colour input[]);

// Get current Rotary Encoder button state
bool R5_getEncButtonState(void);

// Get current Joystick button state
bool R5_getJoystickButtonState(void);

#endif