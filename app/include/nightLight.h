#ifndef _NIGHTLIGHT_H_
#define _NIGHTLIGHT_H_

// Module to manage turning on light when threshold is reached
#include <stdbool.h>

void nightLight_processEvent(bool isRising);

// Get/set/toggle night light
bool nightLight_setLightMode(bool val);
void nightLight_toggleLightMode(void);
bool nightLight_getLightMode(void);

void nightLight_init(void);

void nightLight_cleanup(void);

#endif