#include <WString.h>
#include "common.h"
#include "config.h"

void config_read(const char *file_path, Config *cfg)
{
    // cfg->ssid = tf.readFileLine(file_path, 1);        // line-1 for WiFi ssid
    // cfg->password = tf.readFileLine(file_path, 2);    // line-2 for WiFi password
    // cfg->cityname = tf.readFileLine(file_path, 3);    // line-3 for cityname
    // cfg->language = tf.readFileLine(file_path, 4);    // line-2 for language
    // cfg->weather_key = tf.readFileLine(file_path, 5); // line-3 for weather_key
    // return cfg->ssid+cfg->password+cfg->cityname+cfg->language+cfg->weather_key;

    prefs.begin("Config"); // 打开命名空间mynamespace
    cfg->ssid = prefs.getString("ssid", "");
    cfg->password = prefs.getString("password", "");
    cfg->cityname = prefs.getString("cityname", "BeiJing");
    // cfg->cityname = prefs.getString("cityname", "北京");
    cfg->language = prefs.getString("language", "zh-Hans");
    cfg->weather_key = prefs.getString("weather_key", "");
    cfg->backLight = prefs.getUChar("backLight", 80);
    cfg->rotation = prefs.getUChar("rotation", 4);
    cfg->auto_calibration_mpu = prefs.getUChar("auto_mpu", 1);

    prefs.end(); // 关闭当前命名空间

    // if (0 == cfg->auto_calibration_mpu)
    // {
    // 读取mup的校准数据
    mpu_config_read(file_path, cfg);
    time_read(file_path, cfg);
    // }
}

void config_save(const char *file_path, Config *cfg)
{
    // String line("\n");
    // String res = cfg->ssid + line + cfg->password + line + cfg->cityname + line + cfg->language + line + cfg->weather_key;
    // Serial.println(res);
    // tf.deleteFile(file_path);
    // tf.writeFile(file_path, res.c_str());

    prefs.begin("Config"); // 打开命名空间mynamespace
    prefs.putString("ssid", cfg->ssid);
    prefs.putString("password", cfg->password);
    prefs.putString("cityname", cfg->cityname);
    prefs.putString("language", cfg->language);
    prefs.putString("weather_key", cfg->weather_key);
    prefs.putUChar("backLight", cfg->backLight);
    prefs.putUChar("rotation", cfg->rotation);
    prefs.putUChar("auto_mpu", cfg->auto_calibration_mpu);
    prefs.end(); // 关闭当前命名空间

    config_read("/wifi.txt", &g_cfg);
    // 立即更改屏幕方向
    screen.setBackLight(g_cfg.backLight / 100.0);
    tft->setRotation(g_cfg.rotation);
}

void mpu_config_read(const char *file_path, Config *cfg)
{

    prefs.begin("MPU_Config"); // 打开命名空间mynamespace
    cfg->mpu_config.x_gyro_offset = prefs.getInt("x_gyro_offset", 0);
    cfg->mpu_config.y_gyro_offset = prefs.getInt("y_gyro_offset", 0);
    cfg->mpu_config.z_gyro_offset = prefs.getInt("z_gyro_offset", 0);
    cfg->mpu_config.x_accel_offset = prefs.getInt("x_accel_offset", 0);
    cfg->mpu_config.y_accel_offset = prefs.getInt("y_accel_offset", 0);
    cfg->mpu_config.z_accel_offset = prefs.getInt("z_accel_offset", 0);
    prefs.end(); // 关闭当前命名空间
}

void mpu_config_save(const char *file_path, Config *cfg)
{
    prefs.begin("MPU_Config"); // 打开命名空间mynamespace
    prefs.putInt("x_gyro_offset", cfg->mpu_config.x_gyro_offset);
    prefs.putInt("y_gyro_offset", cfg->mpu_config.y_gyro_offset);
    prefs.putInt("z_gyro_offset", cfg->mpu_config.z_gyro_offset);
    prefs.putInt("x_accel_offset", cfg->mpu_config.x_accel_offset);
    prefs.putInt("y_accel_offset", cfg->mpu_config.y_accel_offset);
    prefs.putInt("z_accel_offset", cfg->mpu_config.z_accel_offset);

    prefs.end(); // 关闭当前命名空间
}

void time_read(const char *file_path, Config *cfg)
{
    prefs.begin("Time"); // 打开命名空间mynamespace
    //初始化本地时间，2021-11-18 14:00:00 UTC +8:00
    cfg->last_time = prefs.getLong64("last_time", 1637215200l);
    prefs.end(); // 关闭当前命名空间
}

void time_save(const char *file_path, Config *cfg)
{
    prefs.begin("Time"); // 打开命名空间mynamespace
    prefs.putLong64("last_time", cfg->last_time);
    prefs.end(); // 关闭当前命名空间
}