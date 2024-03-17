#ifndef _EYE_H___
#define _EYE_H___
#include "lvgl.h"
#define EYE_MIN 1
struct eye_run{
    uint8_t mode_eye;
    unsigned long tempD;
    bool eye_flg;
    uint32_t startTime;  // For FPS indicator
};
void eye_process(lv_obj_t *ym);

#endif
