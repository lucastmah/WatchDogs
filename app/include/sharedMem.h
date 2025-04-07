#ifndef _SHARED_MEM_H_
#define _SHARED_MEM_H_

enum LED_VALUES {
    RED,
    RED_BRIGHT,
    GREEN,
    GREEN_BRIGHT,
    BLUE, 
    BLUE_BRIGHT,
    OFF,
    YELLOW,
    TEAL
};

void sharedMem_setValues(int *led_values);

#endif