// Pan-Tilt module
// Part of the Hardware Abstraction Layer (HAL)

#ifndef _PANTILT_H_
#define _PANTILT_H_

#include <stdbool.h>

// Pan-Tilt Setup:
// Added to /boot/firmware/extlinux/extlinux.conf:
// /overlays/k3-am67a-beagley-ai-pwm-epwm0-gpio15.dtbo /overlays/k3-am67a-beagley-ai-pwm-epwm1-gpio6.dtbo

// $ sudo beagle-pwm-export --pin hat-10
// $ sudo beagle-pwm-export --pin hat-31

enum Axis {
    PAN,
    TILT
};

void panTilt_init(void);
// Set axis from -100% to 100% value 
// higher number for pan = left, higher number for tilt = down
// Returns whether servo has moved
bool panTilt_setPercent(enum Axis axis, int percent);

// Reset axis to default values
void panTilt_resetAxis(enum Axis axis);

void panTilt_cleanup(void);

#endif