#include "hal/panTilt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdatomic.h>

#define PWM_BASE_FILE_PATH "/dev/hat/pwm/"
// Servo operates in nano seconds, using 20ms standard
#define SERVO_DEFAULT_PERIOD 20000000
#define SERVO_STEP_VALUE 100

#define PAN_DEFAULT_VALUE 1400000
#define TILT_DEFAULT_VALUE 1300000

static char *axis_paths[] = {"GPIO15", "GPIO6"}; 
static int axis_min_values[] = {500000, 600000};
static int axis_max_values[] = {2300000, 2400000};

static atomic_int current_positions[] = {-1, -1};

// Allow module to ensure it has been initialized (once!)
static bool is_initialized = false;

static char *get_pwm_file_name(const enum Axis axis, const char *property)
{
    char *axis_name = axis_paths[axis];
    size_t size = strlen(PWM_BASE_FILE_PATH) + strlen(axis_name) + 1 + strlen(property) + 1;
    char *file_name = (char *)malloc(size);
    if (!file_name)
        return NULL;

    snprintf(file_name, size, "%s%s/%s", PWM_BASE_FILE_PATH, axis_name, property);
    return file_name;
}

static bool set_pwm_property(const enum Axis axis, const char *property, const int value)
{
    char *pwmFilePath = get_pwm_file_name(axis, property);
    if (!pwmFilePath)
    {
        return false;
    }
    FILE *pwmFile = fopen(pwmFilePath, "w");
    free(pwmFilePath);

    if (pwmFile == NULL)
    {
        perror("Error opening PWM file");
        return false;
    }

    int charWritten = fprintf(pwmFile, "%d", value);
    if (charWritten <= 0)
    {
        perror("Error writing data to PWM file");
        return false;
    }
    fclose(pwmFile);
    return true;
}

static void set_pwm_enabled(const enum Axis axis, bool state)
{
    set_pwm_property(axis, "enable", state ? 1 : 0);
}

bool panTilt_setPercent(enum Axis axis, int percent)
{
    if (percent < -100) percent = -100;
    if (percent > 100) percent = 100;
    if (percent == 0) return false;

    int new_val = current_positions[axis] + percent * SERVO_STEP_VALUE; 
    if (new_val < axis_min_values[axis]) {
        new_val = axis_min_values[axis];
    }
    if (new_val > axis_max_values[axis]) {
        new_val = axis_max_values[axis];
    }
    printf("debug: setting %d to %d\n", axis, new_val);
    if (new_val != current_positions[axis]) {
        set_pwm_property(axis, "duty_cycle", new_val);
        current_positions[axis] = new_val;
        return true;
    }
    return false;
}

void panTilt_resetAxis(enum Axis axis) {
    int new_val = axis == PAN ? PAN_DEFAULT_VALUE : TILT_DEFAULT_VALUE;
    if (new_val != current_positions[axis]) {
        set_pwm_property(axis, "duty_cycle", new_val);
        current_positions[axis] = new_val;
    }
}

void panTilt_init(void)
{
    assert(!is_initialized);
    set_pwm_enabled(PAN, false);
    set_pwm_enabled(TILT, false);

    set_pwm_property(PAN, "duty_cycle", 0);
    set_pwm_property(TILT, "duty_cycle", 0);
    set_pwm_property(PAN, "period", SERVO_DEFAULT_PERIOD);
    set_pwm_property(TILT, "period", SERVO_DEFAULT_PERIOD);
    panTilt_resetAxis(PAN);
    panTilt_resetAxis(TILT);

    set_pwm_enabled(PAN, true);
    set_pwm_enabled(TILT, true);
    is_initialized = true;
}

void panTilt_cleanup(void)
{
    // Free any memory, close files, ...
    set_pwm_enabled(PAN, false);
    set_pwm_enabled(TILT, false);
    assert(is_initialized);
    is_initialized = false;
}