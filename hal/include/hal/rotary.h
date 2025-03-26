/*

HOW TO USE:

Devices that are interested in the rotary knob turns or pushes need to be added to the corresponding subscriber function.

Process state functions are callback functions for Gpio.

To manually get the current state, call the corresponding get function.

*/

#ifndef ROTARY_H_
#define ROTARY_H_

#include <stdbool.h>

#define MIN_BPM 40
#define MAX_BPM 300
#define BASE_BPM 60

#define PIN_A 7
#define PIN_B 8

// Provided function will be called when the rotary knob push state is changed
void rotary_addPushSubscriber(void (*callback)(int push_count));

// Provided function will be called when the rotary knob turn state is changed
void rotary_addKnobSubscriber(void (*callback)(int turn_counter));

// Callback function to update the state of the rotary knob turns
void rotary_processknobState(int chip, int pin, bool isRising);

// Callback function to update the state of the rotary knob push
void rotary_processPushState(int chip, int pin, bool isRising);

int rotary_getKnobCounter(void);

int rotary_getPushCounter(void);

#endif