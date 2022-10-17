#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H

struct Weather
{
    int weather_code; // 天气现象代码
    int temperature;  // 温度
    int humidity;     // 湿度
    int maxTemp;      // 最高气温
    int minTemp;      // 最低气温
    char windDir[20];
    char cityname[10]; // 城市名
    int windLevel;
    int airQulity;

    short daily_max[7];
    short daily_min[7];
};

struct TimeStr
{
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;
};

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"

#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //等待动画完成

    void weather_gui_init(void);
    void display_curve_init(lv_scr_load_anim_t anim_type);
    void display_curve(short maxT[], short minT[], lv_scr_load_anim_t anim_type);
    void display_weather_init(lv_scr_load_anim_t anim_type);
    void display_weather(struct Weather weaInfo, lv_scr_load_anim_t anim_type);
    void display_time(struct TimeStr timeInfo, lv_scr_load_anim_t anim_type);
    void weather_gui_release(void);
    void weather_gui_del(void);
    void display_space(void);
    int airQulityLevel(int q);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_weather;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif