/*

HOW TO USE:

Call joystick_getDirection repeatedly to get the current direction.

*/

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>

// Map joystick values to [-RANGE, RANGE]
#define JOYSTICK_NORMALIZED_RANGE 100.0
// Size of deadzone area, applied after normalization
#define JOYSTICK_DEADZONE_SIZE 15.0

struct joystickState {
    float X;
    float Y;
};
// Get normalized joystick values
struct joystickState joystick_getState(void);

void joystick_init(void);
void joystick_cleanup(void);

#endif