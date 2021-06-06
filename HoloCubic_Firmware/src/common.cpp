#include "common.h"
#include "config.h"

SdCard tf;
Pixel rgb;
Config g_cfg; // 全局配置文件

boolean doDelayMillisTime(long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}