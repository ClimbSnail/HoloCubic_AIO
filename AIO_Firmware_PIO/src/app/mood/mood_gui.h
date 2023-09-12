#ifndef APP_EXAMPLE_GUI_H
#define APP_EXAMPLE_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void mood_gui_init(void);
    void display_mood(const char *file_name, lv_scr_load_anim_t anim_type);
    void mood_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_mood;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif