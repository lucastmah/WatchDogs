/*

HOW TO USE:

Call lightSensor_getReading to get the latest value (reading may be delayed up to 180ms)

*/

#ifndef LIGHTSENSOR_H_
#define LIGHTSENSOR_H_

// Returns the current raw light sensor reading
unsigned int lightSensor_getReading(void);

#endif