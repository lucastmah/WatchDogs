// Pan-Tilt module
// Part of the Hardware Abstraction Layer (HAL)

#ifndef _PANTILT_H_
#define _PANTILT_H_

#include <stdbool.h>

enum Axis {
    PAN,
    TILT
};

void panTilt_init(void);
// Set axis from -100% to 100% value 
// higher number for pan = left, higher number for tilt = down
void panTilt_setPercent(enum Axis axis, int percent);
void panTilt_cleanup(void);

#endif