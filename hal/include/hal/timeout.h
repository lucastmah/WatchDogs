/*

HOW TO USE:

Call timeout_start_timer with an _Atomic bool pointer to wait 150ms before the boolean is set to false.

*/

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <stdbool.h>

// Starts a thread that sleeps for 150ms and then sets the provided boolean pointer to false
void timeout_start_timer(_Atomic bool* ptr);

#endif