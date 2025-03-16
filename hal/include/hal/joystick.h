#ifndef JOYSTICK_H_

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

#define DEADZONE 15

enum DIRECTION {
    CENTER,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct joystickState {
    float X;
    float Y;
    bool isPressed;
};

enum DIRECTION joystick_getDirection(void);

#endif