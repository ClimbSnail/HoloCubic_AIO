#ifndef APP_BILIBILI_GUI_H
#define APP_BILIBILI_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void bilibili_gui_init(void);
    void display_bilibili(const char *file_name, lv_scr_load_anim_t anim_type, const char * , const char * );
    void bilibili_obj_del(void);
    void bilibili_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_bilibili;
    extern const lv_img_dsc_t bilibili_logo_ico;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif