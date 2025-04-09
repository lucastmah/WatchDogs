#ifndef _NIGHTLIGHT_H_
#define _NIGHTLIGHT_H_

// Module to manage turning on light when threshold is reached
#include <stdbool.h>

void nightLight_processEvent(bool isRising);

void nightLight_setLightMode(bool val);

bool nightLight_getLightMode(void);

void nightLight_init(void);

void nightLight_cleanup(void);

#endif