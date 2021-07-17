#ifndef COMMON_H
#define COMMON_H

#include "sd_card.h"
#include "Arduino.h"
#include "rgb_led.h"
#include "config.h"
#include "imu.h"
#include "network.h"
#include <Preferences.h>

extern SdCard tf;
extern Pixel rgb;
extern Config g_cfg;      // 全局配置文件
extern Network g_network; // 网络连接
extern Preferences prefs; // 声明Preferences对象

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);

#define GFX 0

#if GFX
#include <Arduino_GFX_Library.h>
extern Arduino_HWSPI *bus;
extern Arduino_ST7789 *tft;

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
extern TFT_eSPI *tft;
#endif

#endif