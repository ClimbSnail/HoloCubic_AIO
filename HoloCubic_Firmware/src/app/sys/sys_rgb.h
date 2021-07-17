#ifndef SYS_RGB_H
#define SYS_RGB_H

#include <esp32-hal-timer.h>

struct RgbParam
{
    // 预先定义一个指针来存放定时器的位置
    int min_value_r;
    int min_value_g;
    int min_value_b;

    int max_value_r;
    int max_value_g;
    int max_value_b;
    int8_t step;

    float min_brightness; // 背光的最大亮度
    float max_brightness; // 背光的最小亮度
    float brightness_step;

    int time; // 定时器的时间
};

struct RgbRunStatus
{
    int current_r;
    int current_g;
    int current_b;
    uint8_t pos;
    float current_brightness;

};

void rgb_thread_init(RgbParam rgb_setting);

void set_rgb(RgbParam rgb_setting);

// void IRAM_ATTR rgbOnTimer();
void rgbOnTimer(TimerHandle_t xTimer);

void rgb_thread_del(void);

#endif