#include "hal/lightSensor.h"
#include "hal/i2c.h"

unsigned int lightSensor_getReading(void) {
    return i2c_getBH1750Value();
}