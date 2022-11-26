/*
 * @Author       : zhangmingxin
 * @Date         : 2022-03-18 10:14:24
 * @FilePath     : \lv_platformio\src\main.c
 * @Email        : 17360690414@189.com
 * @LastEditTime : 2022-03-21 16:36:48
 */
/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */

#include "bsp_mylvgl.h"
#include "lvgl.h"
#include "app_hal.h"

#include "app/example/example_gui.h"
#include "app/weather/weather_gui.h"
#include <stdlib.h>
#include <SDL2/SDL_timer.h>

void weather_app_ico()
{
    static lv_obj_t *default_scr;
    default_scr = lv_scr_act();

    lv_obj_t *ico = lv_img_create(default_scr);
    lv_obj_align(ico, LV_ALIGN_CENTER, 0, -20);
    lv_img_set_src(ico, &app_weather);

    lv_obj_t *label = lv_label_create(default_scr);
    lv_label_set_text(label, "weather");
    lv_obj_align_to(label, ico, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

void weather_app_simulation()
{
    // 构造伪数据
    Weather wea_info;
    strcpy(wea_info.cityname, "北京");
    wea_info.weather_code = 0;
    wea_info.temperature = 20;
    // 获取湿度
    wea_info.humidity = 50;
    char humidity[8] = {0};
    strncpy(humidity, "80%", 8);
    humidity[strlen(humidity) - 1] = 0; // 去除尾部的 % 号
    wea_info.humidity = 80;
    wea_info.maxTemp = 25;
    wea_info.minTemp = 15;
    strcpy(wea_info.windDir, "西北");
    wea_info.windLevel = 3;
    wea_info.airQulity = airQulityLevel(3);

    weather_gui_init();
    display_weather(wea_info, LV_SCR_LOAD_ANIM_NONE);

    // display_curve(short maxT[], short minT[], LV_SCR_LOAD_ANIM_NONE);
    // display_time(struct TimeStr timeInfo, LV_SCR_LOAD_ANIM_NONE);
    // display_space();
    // weather_gui_del();
}

int main(void)
{
    LV_LOG_USER("AIO_Simulater");

    lv_init();

    hal_setup();

    // lv_example_anim_1();
    // lv_demo_widgets();

    // 显示图标
    weather_app_ico();
    SDL_Delay(5);
    lv_task_handler();

    // 延时显示app内部界面
    SDL_Delay(1000); // 延时
    weather_app_simulation();

    hal_loop();
}
