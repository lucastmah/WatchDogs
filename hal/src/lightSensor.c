#include "hal/lightSensor.h"
#include "hal/i2c.h"
#include <stdbool.h>
#include <assert.h>

static bool is_initialized = false;

void lightSensor_init(void) {
    assert(!is_initialized);
    is_initialized = true;
}

unsigned int lightSensor_getReading(void) {
    assert(is_initialized);
    return i2c_getBH1750Value();
}

void lightSensor_cleanup(void) {
    assert(is_initialized);
    is_initialized = false;
}