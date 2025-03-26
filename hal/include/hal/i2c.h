#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

enum TLA_device {
    JOYSTICK_X,
    JOYSTICK_Y,
    LED_EMITTER
};

void i2c_init(void);

uint16_t i2c_getTLAValue(int device);

int16_t i2c_getIISValue(int dimension);

uint16_t i2c_getBH1750Value(void);

#endif