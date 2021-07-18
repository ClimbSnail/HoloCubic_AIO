#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void weather_gui_init(void);
    void display_hardware(const char *info, lv_scr_load_anim_t anim_type);
    void display_weather(const char *cityname, const char *temperature,
                         int weathercode, lv_scr_load_anim_t anim_type);
    void display_time(const char *date, const char *time,
                      lv_scr_load_anim_t anim_type);
    void weather_gui_del(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_hardware_info;
    extern const lv_img_dsc_t app_weather;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif