/*

HOW TO USE:

Call the corresponding function for the device you want to get the values from.

TLA = Joystick and Light Emitter
IIS = Accelerometer
BH1750 = Light Sensor

*/

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

enum TLA_device {
    JOYSTICK_X,
    JOYSTICK_Y,
    LED_EMITTER
};

void i2c_init(void);

// Gets the raw reading of the provided device value defined by enum TLA_device
uint16_t i2c_getTLAValue(enum TLA_device device);

// Gets the raw accelerometer reading of the provided axis
int16_t i2c_getIISValue(int dimension);

// Gets the current light reading
uint16_t i2c_getBH1750Value(void);

#endif