#ifndef RGB_H
#define RGB_H

#include <FastLED.h>
#include <esp32-hal-timer.h>

#define RGB_LED_NUM 2
#define RGB_LED_PIN 27

#define LED_MODE_RGB 0
#define LED_MODE_HSV 1

enum LED_RUN_MODE : unsigned char
{
    RUN_MODE_TIMER = 0, // rtos timer
    RUN_MODE_TASK,      // rtos task
    RUN_MODE_NONE
};

class Pixel
{
private:
    CRGB rgb_buffers[RGB_LED_NUM];

public:
    void init();

    Pixel &setRGB(int r, int g, int b);

    Pixel &setHVS(uint8_t ih, uint8_t is, uint8_t iv);

    Pixel &fill_rainbow(int min_r, int max_r,
                        int min_g, int max_g,
                        int min_b, int max_b);

    Pixel &setBrightness(float duty);
};

struct RgbConfig
{
    uint8_t mode;
    uint8_t min_value_0;
    uint8_t min_value_1;
    uint8_t min_value_2;
    uint8_t max_value_0;
    uint8_t max_value_1;
    uint8_t max_value_2;
    int8_t step_0;
    int8_t step_1;
    int8_t step_2;
    float min_brightness;
    float max_brightness;
    float brightness_step;
    int time; // 定时器的时间
};

struct RgbParam
{
    uint8_t mode; // 0为RGB色彩(LED_MODE_RGB) 1为HSV色彩(LED_MODE_HSV)
    union
    {
        uint8_t min_value_r;
        uint8_t min_value_h;
    };
    union
    {
        uint8_t min_value_g;
        uint8_t min_value_s;
    };
    union
    {
        uint8_t min_value_b;
        uint8_t min_value_v;
    };

    union
    {
        uint8_t max_value_r;
        uint8_t max_value_h;
    };
    union
    {
        uint8_t max_value_g;
        uint8_t max_value_s;
    };
    union
    {
        uint8_t max_value_b;
        uint8_t max_value_v;
    };

    union
    {
        int8_t step_r;
        int8_t step_h;
    };
    union
    {
        int8_t step_g;
        int8_t step_s;
    };
    union
    {
        int8_t step_b;
        int8_t step_v;
    };

    float min_brightness; // 背光的最小亮度
    float max_brightness; // 背光的最大亮度
    float brightness_step;

    int time; // 定时器的时间
};

struct RgbRunStatus
{
    union
    {
        uint8_t current_r;
        uint8_t current_h;
    };
    union
    {
        uint8_t current_g;
        uint8_t current_s;
    };
    union
    {
        uint8_t current_b;
        uint8_t current_v;
    };
    uint8_t pos;
    float current_brightness;
};

bool set_rgb_and_run(RgbParam *rgb_setting,
                     LED_RUN_MODE mode = RUN_MODE_TASK);
void rgb_stop(void);


#endif
