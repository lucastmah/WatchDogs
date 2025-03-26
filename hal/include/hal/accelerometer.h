#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

struct accelerometer_values {
    int x;
    int y;
    int z;
};

void accelerometer_getValues(struct accelerometer_values* buf);

#endif