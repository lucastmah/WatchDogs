/*

HOW TO USE:

Need to add functions that are interested in knowing when motion is detected when initializing system first with motionSensor_addSubscriber function.

motionSensor_processState is a callback function for Gpio.

*/

#ifndef MOTIONSENSOR_H_
#define MOTIONSENSOR_H_

#include <stdbool.h>

#define SENSOR_CHIP 1
#define SENSOR_PIN 38


void motionSensor_init(void);
void motionSensor_addSubscriber(void (*callback)(bool motion_state));

void motionSensor_processState(int chip, int pin, bool is_rising);
void motionSensor_cleanup(void);


#endif