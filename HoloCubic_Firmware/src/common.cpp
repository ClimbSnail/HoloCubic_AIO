#include "common.h"
#include "config.h"
#include "network.h"

SdCard tf;
Pixel rgb;
Config g_cfg;      // 全局配置文件
Network g_network; // 网络连接
Preferences prefs; // 声明Preferences对象

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}