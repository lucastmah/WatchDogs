#include "hal/accelerometer.h"
#include "hal/i2c.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void accelerometer_getValues(struct accelerometer_values* buf) {
    buf->x = 0;
    buf->y = 0;
    buf->z = 0;
    buf->x = i2c_getIISValue(0);
    buf->y = i2c_getIISValue(1);
    buf->z = i2c_getIISValue(2);
}