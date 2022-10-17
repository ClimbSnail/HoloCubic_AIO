#ifndef CONFIG_H
#define CONFIG_H
#include <WString.h>
#include <Preferences.h>

extern Preferences prefs;       // 声明Preferences对象

struct MPU_Config
{
    int16_t x_gyro_offset;
    int16_t y_gyro_offset;
    int16_t z_gyro_offset;

    int16_t x_accel_offset;
    int16_t y_accel_offset;
    int16_t z_accel_offset;
};

struct Config
{
    String ssid;
    String password;
    String cityname;              // 显示的城市
    String language;              // 天气查询的地址编码
    String weather_key;           // 知心天气api_key（秘钥）
    String tianqi_appid;          // tianqiapid 的 appid
    String tianqi_appsecret;      // tianqiapid 的 appsecret
    String tianqi_addr;           // tianqiapid 的地址（填中文）
    String bili_uid;              // bilibili的uid
    uint8_t backLight;            // 屏幕亮度（1-100）
    uint8_t rotation;             // 屏幕旋转方向
    uint8_t auto_calibration_mpu; // 是否自动校准陀螺仪 0关闭自动校准 1打开自动校准
    uint8_t mpu_order;
    MPU_Config mpu_config; // 保存mup的校准数据
};

void config_read(const char *file_path, Config *cfg);

void config_save(const char *file_path, Config *cfg);

void mpu_config_read(const char *file_path, Config *cfg);

void mpu_config_save(const char *file_path, Config *cfg);

#endif