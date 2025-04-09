// Controls camera movement
#ifndef _CAMERA_CONTROLS_H_
#define _CAMERA_CONTROLS_H_

#include <stdbool.h>

void CameraControls_init(void);
void CameraControls_cleanup(void);

// Set/Get/Toggle camera to be on patrol mode, panning left and right periodically
bool CameraControls_setPatrolMode(bool value);
void CameraControls_togglePatrolMode(void);
bool CameraControls_getPatrolMode(void);

// Move camera 1 step in pan/tilt axis, direction is either -1 or 1
// These functions will turn off patrol mode, if on
void CameraControls_pan(int direction);
void CameraControls_tilt(int direction);

#endif