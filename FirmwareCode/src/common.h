#ifndef COMMON_H
#define COMMON_H

#include "sd_card.h"
#include "Arduino.h"
#include "rgb_led.h"
#include "config.h"


extern SdCard tf;
extern Pixel rgb;
extern Config g_cfg; // 全局配置文件

boolean doDelayMillisTime(long interval, unsigned long *previousMillis, boolean state);

#endif