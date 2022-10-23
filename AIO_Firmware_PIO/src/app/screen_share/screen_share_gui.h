#ifndef APP_PICTURE_GUI_H
#define APP_PICTURE_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_screen;

    void screen_share_gui_init(void);
    void display_share_init(void);
    void display_screen_share(const char *title, const char *ip,
                              const char *port, const char *info,
                              lv_scr_load_anim_t anim_type);
    void screen_share_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif