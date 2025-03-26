#ifndef ROTARY_H_
#define ROTARY_H_

#include <stdbool.h>

#define MIN_BPM 40
#define MAX_BPM 300
#define BASE_BPM 60

#define PIN_A 7
#define PIN_B 8

void rotary_addPushSubscriber(void (*callback)(int push_count));

void rotary_addKnobSubscriber(void (*callback)(int turn_counter));

void rotary_processknobState(int chip, int pin, bool isRising);

void rotary_processPushState(int chip, int pin, bool isRising);

int rotary_getKnobCounter(void);

int rotary_getPushCounter(void);

#endif