#include "common.h"
#include "network.h"

IMU mpu;
SdCard tf;
Pixel rgb;
// Config g_cfg;       // 全局配置文件
Network g_network;  // 网络连接
FlashFS g_flashCfg; // flash中的文件系统（替代原先的Preferences）
Display screen;     // 屏幕对象
Ambient ambLight;   // 光线传感器对象

// lvgl handle的锁
SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateMutex();

boolean doDelayMillisTime(unsigned long interval, unsigned long *previousMillis, boolean state)
{
    unsigned long currentMillis = GET_SYS_MILLIS();
    if (currentMillis - *previousMillis >= interval)
    {
        *previousMillis = currentMillis;
        state = !state;
    }
    return state;
}

bool get_current_hour_minute(uint8_t* hour, uint8_t* minute) {
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){
        *hour = timeinfo.tm_hour;
        *minute = timeinfo.tm_min;
        return true;
    }
    return false;
}

bool is_night_mode_time(uint8_t start_hour, uint8_t end_hour)
{
    uint8_t current_hour, current_minute;
    if (!get_current_hour_minute(&current_hour, &current_minute)) {
        Serial.println("Failed to get time for night mode check");
        return false; // 时间获取失败，默认非夜间模式
    }

    if (start_hour < end_hour) {
        // 普通时间段，例如 20:00 - 06:00
        return (current_hour >= start_hour || current_hour < end_hour);
    } else {
        // 跨天时间段，例如 22:00 - 05:00
        return (current_hour >= start_hour || current_hour < end_hour);
    }
}

#if GFX

#include <Arduino_GFX_Library.h>

Arduino_HWSPI *bus = new Arduino_HWSPI(TFT_DC /* DC */, TFT_CS /* CS */, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_ST7789 *tft = new Arduino_ST7789(bus, TFT_RST /* RST */, 3 /* rotation */, true /* IPS */,
                                         240 /* width */, 240 /* height */,
                                         0 /* col offset 1 */, 80 /* row offset 1 */);

#else
#include <TFT_eSPI.h>
/*
TFT pins should be set in path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
TFT_eSPI *tft = new TFT_eSPI(SCREEN_HOR_RES, SCREEN_VER_RES);
#endif