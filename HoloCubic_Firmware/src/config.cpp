#include <WString.h>
#include "common.h"
#include "config.h"

Preferences prefs;        // 声明Preferences对象

void config_read(const char *file_path, Config *cfg)
{
    // cfg->ssid = tf.readFileLine(file_path, 1);        // line-1 for WiFi ssid
    // cfg->password = tf.readFileLine(file_path, 2);    // line-2 for WiFi password
    // return cfg->ssid+cfg->password+cfg->cityname+cfg->language+cfg->weather_key;

    prefs.begin("sys"); // 打开命名空间mynamespace
    cfg->backLight = prefs.getUChar("backLight", 80);
    cfg->rotation = prefs.getUChar("rotation", 4);
    cfg->auto_calibration_mpu = prefs.getUChar("auto_mpu", 1);
    cfg->mpu_order = prefs.getUChar("mpu_order", 0);
    cfg->ssid = prefs.getString("ssid", "");
    cfg->password = prefs.getString("password", "");
    prefs.end(); // 关闭当前命名空间

    prefs.begin("zhixin"); // 打开命名空间mynamespace
    cfg->cityname = prefs.getString("cityname", "BeiJing");
    cfg->language = prefs.getString("language", "zh-Hans");
    cfg->weather_key = prefs.getString("weather_key", "");
    prefs.end(); // 关闭当前命名空间

    prefs.begin("tianqi"); // 打开命名空间mynamespace
    cfg->tianqi_appid = prefs.getString("tianqi_aid", "");
    cfg->tianqi_appsecret = prefs.getString("tianqi_as", "");
    cfg->tianqi_addr = prefs.getString("tianqi_addr", "北京");
    prefs.end(); // 关闭当前命名空间

    prefs.begin("other"); // 打开命名空间mynamespace
    cfg->bili_uid = prefs.getString("bili_uid", "");
    prefs.end(); // 关闭当前命名空间

    // if (0 == cfg->auto_calibration_mpu)
    // {
    // 读取mup的校准数据
    mpu_config_read(file_path, cfg);
    // }
}

// void config_read(const char *file_path, Config *cfg)
// {
//     // cfg->ssid = tf.readFileLine(file_path, 1);        // line-1 for WiFi ssid
//     // cfg->password = tf.readFileLine(file_path, 2);    // line-2 for WiFi password
//     // return cfg->ssid+cfg->password+cfg->cityname+cfg->language+cfg->weather_key;

//     prefs.begin("sys"); // 打开命名空间mynamespace
//     cfg->backLight = prefs.getUChar("backLight", 100);
//     cfg->rotation = prefs.getUChar("rotation", 4);
//     cfg->auto_calibration_mpu = prefs.getUChar("auto_mpu", 1);
//     cfg->ssid = prefs.getString("ssid", "902");
//     cfg->password = prefs.getString("password", "7#59027#5902");
//     prefs.end(); // 关闭当前命名空间

//     prefs.begin("zhixin"); // 打开命名空间mynamespace
//     cfg->cityname = prefs.getString("cityname", "BeiJing");
//     // cfg->cityname = prefs.getString("cityname", "北京");
//     cfg->language = prefs.getString("language", "zh-Hans");
//     cfg->weather_key = prefs.getString("weather_key", "SfGOpgf5IPnqgFbke");
//     prefs.end(); // 关闭当前命名空间

//     prefs.begin("tianqi"); // 打开命名空间mynamespace
//     cfg->tianqi_appid = prefs.getString("tianqi_aid", "84682835");
//     cfg->tianqi_appsecret = prefs.getString("tianqi_as", "35rAfhRZ");
//     cfg->tianqi_addr = prefs.getString("tianqi_addr", "北京");
//     prefs.end(); // 关闭当前命名空间

//     prefs.begin("other"); // 打开命名空间mynamespace
//     cfg->bili_uid = prefs.getString("bili_uid", "344470052");
//     prefs.end(); // 关闭当前命名空间

//     // if (0 == cfg->auto_calibration_mpu)
//     // {
//     // 读取mup的校准数据
//     mpu_config_read(file_path, cfg);
//     // }
// }

void config_save(const char *file_path, Config *cfg)
{
    // String line("\n");
    // String res = cfg->ssid + line + cfg->password + line + cfg->cityname + line + cfg->language + line + cfg->weather_key;
    // Serial.println(res);
    // tf.deleteFile(file_path);
    // tf.writeFile(file_path, res.c_str());

    prefs.begin("sys"); // 打开命名空间mynamespace
    prefs.putUChar("backLight", cfg->backLight);
    prefs.putUChar("rotation", cfg->rotation);
    prefs.putUChar("auto_mpu", cfg->auto_calibration_mpu);
    prefs.putUChar("mpu_order", cfg->mpu_order);
    prefs.putString("ssid", cfg->ssid);
    prefs.putString("password", cfg->password);
    prefs.end(); // 关闭当前命名空间

    prefs.begin("zhixin"); // 打开命名空间mynamespace
    prefs.putString("cityname", cfg->cityname);
    prefs.putString("language", cfg->language);
    prefs.putString("weather_key", cfg->weather_key);
    prefs.end(); // 关闭当前命名空间

    prefs.begin("tianqi"); // 打开命名空间mynamespace
    prefs.putString("tianqi_aid", cfg->tianqi_appid);
    prefs.putString("tianqi_as", cfg->tianqi_appsecret);
    prefs.putString("tianqi_addr", cfg->tianqi_addr);
    prefs.end(); // 关闭当前命名空间

    prefs.begin("other"); // 打开命名空间mynamespace
    prefs.putString("bili_uid", cfg->bili_uid);
    prefs.end(); // 关闭当前命名空间

    // config_read("/wifi.txt", &g_cfg);
    // // 立即更改屏幕方向
    // screen.setBackLight(g_cfg.backLight / 100.0);
    // tft->setRotation(g_cfg.rotation);
    // mpu.setOrder(g_cfg.mpu_order);
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