/*

HOW TO USE:

Call accelerometer_getValues to get the current raw values of acceleration stored in a struct defined in this file.

*/
#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

struct accelerometer_values {
    int x;
    int y;
    int z;
};

void accelerometer_getValues(struct accelerometer_values* buf);

#endif