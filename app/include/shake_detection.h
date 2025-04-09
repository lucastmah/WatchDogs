#ifndef _SHAKE_H_
#define _SHAKE_H_

// Module to detect attack on board

// Set email to send notification to
void shakeDetect_setEmail(char input[]);

void shakeDetect_init(void);
void shakeDetect_cleanup(void);

#endif