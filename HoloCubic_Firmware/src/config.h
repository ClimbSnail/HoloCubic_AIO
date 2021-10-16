#ifndef CONFIG_H
#define CONFIG_H
#include <WString.h>

struct Config
{
    String ssid;
    String password;
    String cityname;    // 显示的城市
    String language;    // 天气查询的地址编码
    String weather_key; // 知心天气api_key（秘钥）
    uint8_t backLight;  // 屏幕亮度（1-100）
    uint8_t rotation;   // 屏幕旋转方向
};

void config_read(const char *file_path, Config *cfg);

void config_save(const char *file_path, Config *cfg);

#endif