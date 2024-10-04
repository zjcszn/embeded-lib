#ifndef _CAMERA_H
#define _CAMERA_H

int open_camera(void);
int close_camera(void);
void camera_button_event(int event);

extern struct CAMERA_CFG camera_cfg;

#endif

