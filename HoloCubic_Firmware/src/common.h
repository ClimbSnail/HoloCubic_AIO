#ifndef COMMON_H
#define COMMON_H

#define AIO_VERSION "1.8.1"

#include "Arduino.h"
#include "config.h"
#include "driver/rgb_led.h"
#include "driver/sd_card.h"
#include "driver/display.h"
#include "driver/ambient.h"
#include "driver/imu.h"
#include "driver/timeUtile.h"
#include "network.h"
#include <Preferences.h>

extern SdCard tf;
extern Pixel rgb;
extern Config g_cfg;           // 全局配置文件
extern Network g_network;      // 网络连接
extern Preferences prefs;      // 声明Preferences对象
extern Display screen;         // 屏幕对象
extern Ambient ambLight;       // 光纤传感器对象
extern TimeUtile g_time_utile; //时间管理对象

boolean doDelayMillisTime(unsigned long interval,
                          unsigned long *previousMillis,
                          boolean state);

#define GFX 0

#if GFX
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1 // Not connected
#define TFT_DC 2
#define TFT_RST 4 // Connect reset to ensure display initialises
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