#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>

class Display
{
public:
    bool night_mode = false; // 是否处于夜间模式

    void init(uint8_t rotation, uint8_t backLight);
    void routine();
    void setBackLight(float);
    uint8_t getBrightness();
private:
    float current_backlight = 1.0; // 当前背光 0~1
};


#endif
