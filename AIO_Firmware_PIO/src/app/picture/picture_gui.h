#ifndef APP_PICTURE_GUI_H
#define APP_PICTURE_GUI_H


#define PIC_FILENAME_MAX_LEN 100

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void photo_gui_init(void);
    void display_photo_init(void);
    void display_photo(const char *file_name, lv_scr_load_anim_t anim_type);
    void photo_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_picture;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif