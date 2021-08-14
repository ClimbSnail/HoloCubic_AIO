#ifndef APP_CONTORLLER_GUI_H
#define APP_CONTORLLER_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void app_control_gui_init(void);
    void display_app_scr_init(const void *src_img);
    void app_contorl_display_scr(const void *src_img,
                                 lv_scr_load_anim_t anim_type, bool force);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_loading;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif