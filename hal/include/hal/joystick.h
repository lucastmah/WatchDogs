/*

HOW TO USE:

Call joystick_getDirection repeatedly to get the current direction.

*/

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

// Map joystick values to [-RANGE, RANGE]
#define JOYSTICK_NORMALIZED_RANGE 100.0
// Size of deadzone area, applied after normalization
#define JOYSTICK_DEADZONE_SIZE 15.0

struct joystickState {
    float X;
    float Y;
};
// Get normalized joystick values
struct joystickState joystick_getState(void);

void joystick_init(void);

void joystick_cleanup(void);

#endif