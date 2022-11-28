#include "rgb_led.h"
#include "common.h"
#include <Arduino.h>

void Pixel::init()
{
    FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(rgb_buffers, RGB_LED_NUM);
    FastLED.setBrightness(200);
}

Pixel &Pixel::setRGB(int r, int g, int b)
{
    for (int pos = 0; pos < RGB_LED_NUM; ++pos)
    {
        rgb_buffers[pos] = CRGB(r, g, b);
    }
    FastLED.show();

    return *this;
}

Pixel &Pixel::setHVS(uint8_t ih, uint8_t is, uint8_t iv)
{
    for (int pos = 0; pos < RGB_LED_NUM; ++pos)
    {
        rgb_buffers[pos].setHSV(ih, is, iv);
    }
    FastLED.show();

    return *this;
}

Pixel &Pixel::fill_rainbow(int min_r, int max_r,
                           int min_g, int max_g,
                           int min_b, int max_b)
{
    fill_gradient(rgb_buffers, 0, CHSV(50, 255, 255), 29, CHSV(150, 255, 255), SHORTEST_HUES);
    FastLED.show();

    return *this;
}

Pixel &Pixel::setBrightness(float duty)
{
    duty = constrain(duty, 0, 1);
    FastLED.setBrightness((uint8_t)(255 * duty));
    FastLED.show();

    return *this;
}

LED_RUN_MODE run_mode = RUN_MODE_TIMER;
RgbParam g_rgb;
RgbRunStatus rgb_status;
BaseType_t taskRgbReturned;
TaskHandle_t handleRgb = NULL;
TimerHandle_t xTimer_rgb = NULL;

void rgb_thread_run(RgbParam *rgb_setting, LED_RUN_MODE mode)
{
    // 80Mh主频
    run_mode = mode;
    set_rgb(rgb_setting);
}

void led_hsvTimerHandler(TimerHandle_t xTimer)
{
    led_hsvHandler();
}

void led_hsvTaskHandler(void *parameter)
{
    int *ms = (int *)parameter; // 控制时间
    for (;;)
    {
        led_hsvHandler();
        vTaskDelay(*ms);

        // if (pdTRUE == xSemaphoreTake(lvgl_mutex, portMAX_DELAY))
        // {
        //     lv_task_handler();
        //     xSemaphoreGive(lvgl_mutex);
        // }
    }
}

void led_hsvHandler(void)
{
    // HSV色彩的控制
    rgb_status.current_h += g_rgb.step_h;
    if (rgb_status.current_h >= g_rgb.max_value_h)
    {
        g_rgb.step_h = (-1) * g_rgb.step_h;
        rgb_status.current_h = g_rgb.max_value_h;
    }
    else if (rgb_status.current_h <= g_rgb.min_value_h)
    {
        g_rgb.step_h = (-1) * g_rgb.step_h;
        rgb_status.current_h = g_rgb.min_value_h;
    }

    rgb_status.current_s += g_rgb.step_s;
    if (rgb_status.current_s >= g_rgb.max_value_s)
    {
        g_rgb.step_s = (-1) * g_rgb.step_s;
        rgb_status.current_s = g_rgb.max_value_s;
    }
    else if (rgb_status.current_s <= g_rgb.min_value_s)
    {
        g_rgb.step_s = (-1) * g_rgb.step_s;
        rgb_status.current_s = g_rgb.min_value_s;
    }

    rgb_status.current_v += g_rgb.step_v;
    if (rgb_status.current_v >= g_rgb.max_value_v)
    {
        g_rgb.step_v = (-1) * g_rgb.step_v;
        rgb_status.current_v = g_rgb.max_value_v;
    }
    else if (rgb_status.current_v <= g_rgb.min_value_v)
    {
        g_rgb.step_v = (-1) * g_rgb.step_v;
        rgb_status.current_v = g_rgb.min_value_v;
    }

    // 计算当前背光值
    count_cur_brightness();
    // rgb_status.current_brightness = g_rgb.max_brightness;

    // 设置HSV状态
    rgb.setHVS(rgb_status.current_h,
               rgb_status.current_s,
               rgb_status.current_v)
        .setBrightness(rgb_status.current_brightness);
}

// void IRAM_ATTR led_rgbTimerHandler()
void led_rgbTimerHandler(TimerHandle_t xTimer)
{
    led_rgbHandler();
}

void led_rgbTaskHandler(void *parameter)
{
    int *ms = (int *)parameter; // 控制时间
    for (;;)
    {
        led_rgbHandler();
        vTaskDelay(*ms);

        // if (pdTRUE == xSemaphoreTake(lvgl_mutex, portMAX_DELAY))
        // {
        //     lv_task_handler();
        //     xSemaphoreGive(lvgl_mutex);
        // }
    }
}

void led_rgbHandler()
{
    // RGB色彩的控制
    if (0 == rgb_status.pos) // 控制到R
    {
        rgb_status.current_r += g_rgb.step_r;
        if (rgb_status.current_r >= g_rgb.max_value_r && g_rgb.step_r > 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_r = g_rgb.max_value_r;
        }
        else if (rgb_status.current_r <= g_rgb.min_value_r && g_rgb.step_r < 0)
        {
            g_rgb.step_r = (-1) * g_rgb.step_r;
            rgb_status.current_r = g_rgb.min_value_r;
        }
    }
    else if (1 == rgb_status.pos) // 控制到G
    {
        rgb_status.current_g += g_rgb.step_r;
        if (rgb_status.current_g >= g_rgb.max_value_g && g_rgb.step_r > 0)
        {
            rgb_status.pos = 2;
            rgb_status.current_g = g_rgb.max_value_g;
        }
        else if (rgb_status.current_g <= g_rgb.min_value_g && g_rgb.step_r < 0)
        {
            rgb_status.pos = 0;
            rgb_status.current_g = g_rgb.min_value_g;
        }
    }
    else if (2 == rgb_status.pos) // 控制到B
    {
        rgb_status.current_b += g_rgb.step_r;
        if (rgb_status.current_b >= g_rgb.max_value_b && g_rgb.step_r > 0)
        {
            g_rgb.step_r = (-1) * g_rgb.step_r;
            rgb_status.current_b = g_rgb.max_value_b;
        }
        else if (rgb_status.current_b <= g_rgb.min_value_b && g_rgb.step_r < 0)
        {
            rgb_status.pos = 1;
            rgb_status.current_b = g_rgb.min_value_b;
        }
    }

    // 计算当前背光值
    count_cur_brightness();

    // 设置RGB状态
    rgb.setRGB(rgb_status.current_r,
               rgb_status.current_g,
               rgb_status.current_b)
        .setBrightness(rgb_status.current_brightness);
}

void count_cur_brightness(void)
{
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
}

void set_rgb(RgbParam *rgb_setting)
{
    g_rgb = *rgb_setting;

    void (*ledHandler)(void *); // 声明rgb处理程序

    if (LED_MODE_RGB == g_rgb.mode)
    {
        rgb_status.current_r = g_rgb.min_value_r;
        rgb_status.current_g = g_rgb.min_value_g;
        rgb_status.current_b = g_rgb.min_value_b;
        rgb_status.current_brightness = g_rgb.min_brightness;
        rgb_status.pos = 0;
        ledHandler = led_rgbTimerHandler;
        ledHandler = led_rgbTaskHandler;
    }
    else if (LED_MODE_HSV == g_rgb.mode)
    {
        rgb_status.current_h = g_rgb.min_value_h;
        rgb_status.current_s = g_rgb.min_value_s;
        rgb_status.current_v = g_rgb.min_value_v;
        rgb_status.current_brightness = g_rgb.min_brightness;
        rgb_status.pos = 0;
        ledHandler = led_hsvOnTimer;
        ledHandler = led_hsvTaskHandler;
    }

    if (RUN_MODE_TIMER == run_mode)
    {
        if (NULL != xTimer_rgb)
        {
            xTimerStop(xTimer_rgb, 0);
            xTimer_rgb = NULL;
        }
        xTimer_rgb = xTimerCreate("rgb contorller",
                                  g_rgb.time / portTICK_PERIOD_MS,
                                  pdTRUE, (void *)0, ledHandler);
        xTimerStart(xTimer_rgb, 0); // 开启定时器
    }
    else if (RUN_MODE_TASK == run_mode)
    {
        if (NULL == handleRgb)
            taskRgbReturned = xTaskCreate(
                ledHandler,
                "ledHandler",
                1000,
                (void *)&g_rgb.time,
                configMAX_PRIORITIES - 1,
                // tskIDLE_PRIORITY,
                &handleRgb);
        if (taskRgbReturned != pdPASS)
        {
        }
    }
}

void rgb_thread_del(void)
{
    if (RUN_MODE_TIMER == run_mode &&
        NULL != xTimer_rgb)
    {
        xTimerStop(xTimer_rgb, 0);
        xTimer_rgb = NULL;
    }
}