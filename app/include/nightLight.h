#ifndef _NIGHTLIGHT_H_
#define _NIGHTLIGHT_H_

#include <stdbool.h>

void nightLight_processEvent(bool isRising);

void nightLight_setLightMode(int param);

void nightLight_init(void);

#endif