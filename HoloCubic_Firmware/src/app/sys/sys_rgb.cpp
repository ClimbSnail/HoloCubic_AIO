#include "sys_rgb.h"
#include "../../common.h"
// #include <esp32-hal-timer.h>

RgbParam g_rgb;
hw_timer_t *rgb_timer;
RgbRunStatus rgb_status;
TimerHandle_t xTimer_rgb;

void rgb_thread_init(RgbParam rgb_setting)
{
    g_rgb = rgb_setting;
    rgb_status.current_r = g_rgb.min_value_r;
    rgb_status.current_g = g_rgb.min_value_g;
    rgb_status.current_b = g_rgb.min_value_b;
    rgb_status.current_brightness = g_rgb.min_brightness;
    rgb_status.pos = 0;

    // // 80Mh主频
    // rgb_timer = timerBegin(0, 80, true);
    // timerAttachInterrupt(rgb_timer, &rgbOnTimer, true);
    // //  操作的定时器 定时时长
    // timerAlarmWrite(rgb_timer, 1000000, true);
    // timerAlarmEnable(rgb_timer); // 使能报警器
    // Serial.print("timerAlarmWrite\n");

    xTimer_rgb = xTimerCreate("rgb contorller", g_rgb.time / portTICK_PERIOD_MS, pdTRUE, (void *)0, rgbOnTimer);
    xTimerStart(xTimer_rgb, 0); //开启定时器
}

// void IRAM_ATTR rgbOnTimer()
void rgbOnTimer(TimerHandle_t xTimer)
{
    // 色彩的控制
    if (0 == rgb_status.pos) // 控制到R
    {
        rgb_status.current_r += g_rgb.step;
        if (rgb_status.current_r >= g_rgb.max_value_r && g_rgb.step > 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_r = g_rgb.max_value_r;
        }
        else if (rgb_status.current_r <= 0 && g_rgb.step < 0)
        {
            g_rgb.step = (-1) * g_rgb.step;
            rgb_status.current_r = g_rgb.min_value_r;
        }
    }
    else if (1 == rgb_status.pos) // 控制到G
    {
        rgb_status.current_g += g_rgb.step;
        if (rgb_status.current_g >= g_rgb.max_value_g && g_rgb.step > 0)
        {
            rgb_status.pos = 2;
            rgb_status.current_g = g_rgb.max_value_g;
        }
        else if (rgb_status.current_g <= g_rgb.min_value_g && g_rgb.step < 0)
        {
            rgb_status.pos = 0;
            rgb_status.current_g = g_rgb.min_value_g;
        }
    }
    else if (2 == rgb_status.pos) // 控制到B
    {
        rgb_status.current_b += g_rgb.step;
        if (rgb_status.current_b >= g_rgb.max_value_b && g_rgb.step > 0)
        {
            g_rgb.step = (-1) * g_rgb.step;
            rgb_status.current_b = g_rgb.max_value_b;
        }
        else if (rgb_status.current_b <= g_rgb.min_value_b && g_rgb.step < 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_b = g_rgb.min_value_b;
        }
    }

    // 背光的控制
    rgb_status.current_brightness += g_rgb.brightness_step;
    if (rgb_status.current_brightness >= g_rgb.max_brightness)
    {
        rgb_status.current_brightness = g_rgb.max_brightness;
        g_rgb.brightness_step = (-1) * g_rgb.brightness_step;
    }
    else if (rgb_status.current_brightness <= g_rgb.min_brightness)
    {
        rgb_status.current_brightness = g_rgb.min_brightness;
        g_rgb.brightness_step = (-1) * g_rgb.brightness_step;
    }

    // 设置RGB状态
    rgb.setRGB(rgb_status.current_r,
               rgb_status.current_g,
               rgb_status.current_b)
        .setBrightness(rgb_status.current_brightness);
}

void set_rgb(RgbParam rgb_setting)
{
    g_rgb = rgb_setting;
}

void rgb_thread_del(void)
{
    // timerEnd(rgb_timer);
    xTimerStop(xTimer_rgb, 0);
}