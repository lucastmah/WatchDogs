// Controls camera movement
#ifndef _CAMERA_CONTROLS_H_
#define _CAMERA_CONTROLS_H_

#include <stdbool.h>

void CameraControls_init(void);
void CameraControls_cleanup(void);

// Move camera 1 step in pan/tilt axis, direction is either -1 or 1
void CameraControls_pan(int direction);
void CameraControls_tilt(int direction);

#endif