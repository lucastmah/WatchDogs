#include "hal/joystick.h"
#include <time.h>
#include "hal/i2c.h"
#include <stdbool.h>
#include <math.h>
#include <assert.h>

static int joystick_X_min = 10;
static int joystick_X_max = 1600;
static int joystick_Y_min = 10;
static int joystick_Y_max = 1630;

static bool is_initialized = false;

static float joystick_applyScaling(float value, char axis) {
    // Update max/min values
    int axis_min, axis_max;
    if (axis == 'y') {
        if (value < joystick_Y_min) {
            joystick_Y_min = value;
        }
    
        if (value > joystick_Y_max) {
            joystick_Y_max = value;
        }
        axis_min = joystick_Y_min;
        axis_max = joystick_Y_max;
    } else {
        if (value < joystick_X_min) {
            joystick_X_min = value;
        }
    
        if (value > joystick_X_max) {
            joystick_X_max = value;
        }
        axis_min = joystick_X_min;
        axis_max = joystick_X_max;
    }
    
    // Scale according to normalized_range
    float scaled = (float) value / (float) (axis_max - axis_min) * JOYSTICK_NORMALIZED_RANGE * 2 - JOYSTICK_NORMALIZED_RANGE;

    // Apply deadzone
    if (fabs(scaled) < JOYSTICK_DEADZONE_SIZE) {
        return 0;
    } else {
        float temp = (fabs(scaled) - JOYSTICK_DEADZONE_SIZE) / (JOYSTICK_NORMALIZED_RANGE - JOYSTICK_DEADZONE_SIZE) * JOYSTICK_NORMALIZED_RANGE;
        if (scaled < 0) {
            scaled = -temp;
        } else {
            scaled = temp;
        }
    }
    return scaled;
}

struct joystickState joystick_getState(void) {
    assert(is_initialized);

    // Y
    uint16_t Y_value = i2c_getTLAValue(JOYSTICK_Y);
    float Y_scaled = joystick_applyScaling(Y_value, 'y');

    // X
    uint16_t X_value = i2c_getTLAValue(JOYSTICK_X);
    float X_scaled = joystick_applyScaling(X_value, 'x');

    struct joystickState state = {X_scaled, -Y_scaled}; // Y axis is inverted
    return state;
}

void joystick_init(void) {
    assert(!is_initialized);
    is_initialized = true;
}

void joystick_cleanup(void) {
    assert(is_initialized);
    is_initialized = false;
}