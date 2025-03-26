#ifndef MOTION_SENSOR_H_
#define MOTION_SENSOR_H_

#include <stdbool.h>

#define SENSOR_CHIP 1
#define SENSOR_PIN 38

// void motion_sensor_initialize(void);

// void motion_sensor_cleanup(void);

void motionSensor_addSubscriber(void (*callback)(bool motion_state));

void motionSensor_processState(int chip, int pin, bool is_rising);

#endif