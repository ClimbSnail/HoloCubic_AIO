#ifndef APP_CLOCK_GUI_H
#define APP_CLOCK_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void clock_gui_init(void);
    void clock_gui_start(lv_scr_load_anim_t anim_type);
    void clock_gui_exit(void);
    void display_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_clock_info;
    extern const lv_img_dsc_t app_clock;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif