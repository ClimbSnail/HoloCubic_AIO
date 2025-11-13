#include <WString.h>
#include "common.h"
#include "config.h"

#ifndef NEW_FLASH_FS

/***************************************************
    Due to the use of a new file system, the functionality implemented in config.cpp/config.h
    has been deprecated. The code is kept for reference purposes and has not been deleted.

    Github repositories：https://github.com/ClimbSnail/HoloCubic_AIO

    Last review/edit by ClimbSnail: 2023/03/14
 ****************************************************/

Preferences prefs;        // Declare Preferences object

void config_read(const char *file_path, Config *cfg)
{
        // Read configuration values from Preferences

        prefs.begin("sys"); // Open namespace "sys"
        cfg->backLight = prefs.getUChar("backLight", 80);
        cfg->rotation = prefs.getUChar("rotation", 4);
        cfg->auto_calibration_mpu = prefs.getUChar("auto_mpu", 1);
        cfg->mpu_order = prefs.getUChar("mpu_order", 0);
        cfg->ssid = prefs.getString("ssid", "");
        cfg->password = prefs.getString("password", "");
        prefs.end(); // Close namespace "sys"

        prefs.begin("zhixin"); // Open namespace "zhixin"
        cfg->cityname = prefs.getString("cityname", "BeiJing");
        cfg->language = prefs.getString("language", "zh-Hans");
        cfg->weather_key = prefs.getString("weather_key", "");
        prefs.end(); // Close namespace "zhixin"

        prefs.begin("tianqi"); // Open namespace "tianqi"
        cfg->tianqi_appid = prefs.getString("tianqi_aid", "");
        cfg->tianqi_appsecret = prefs.getString("tianqi_as", "");
        cfg->tianqi_addr = prefs.getString("tianqi_addr", "Beijing");
        prefs.end(); // Close namespace "tianqi"

        prefs.begin("other"); // Open namespace "other"
        cfg->bili_uid = prefs.getString("bili_uid", "");
        prefs.end(); // Close namespace "other"

        // Read MPU configuration data
        mpu_config_read(file_path, cfg);
}

void config_save(const char *file_path, Config *cfg)
{
        // Save configuration values to Preferences

        prefs.begin("sys"); // Open namespace "sys"
        prefs.putUChar("backLight", cfg->backLight);
        prefs.putUChar("rotation", cfg->rotation);
        prefs.putUChar("auto_mpu", cfg->auto_calibration_mpu);
        prefs.putUChar("mpu_order", cfg->mpu_order);
        prefs.putString("ssid", cfg->ssid);
        prefs.putString("password", cfg->password);
        prefs.end(); // Close namespace "sys"

        prefs.begin("zhixin"); // Open namespace "zhixin"
        prefs.putString("cityname", cfg->cityname);
        prefs.putString("language", cfg->language);
        prefs.putString("weather_key", cfg->weather_key);
        prefs.end(); // Close namespace "zhixin"

        prefs.begin("tianqi"); // Open namespace "tianqi"
        prefs.putString("tianqi_aid", cfg->tianqi_appid);
        prefs.putString("tianqi_as", cfg->tianqi_appsecret);
        prefs.putString("tianqi_addr", cfg->tianqi_addr);
        prefs.end(); // Close namespace "tianqi"

        prefs.begin("other"); // Open namespace "other"
        prefs.putString("bili_uid", cfg->bili_uid);
        prefs.end(); // Close namespace "other"

        // Save MPU configuration data
        mpu_config_save(file_path, cfg);
}

void mpu_config_read(const char *file_path, Config *cfg)
{
        // Read MPU configuration values from Preferences

        prefs.begin("MPU_Config"); // Open namespace "MPU_Config"
        cfg->mpu_config.x_gyro_offset = prefs.getInt("x_gyro_offset", 0);
        cfg->mpu_config.y_gyro_offset = prefs.getInt("y_gyro_offset", 0);
        cfg->mpu_config.z_gyro_offset = prefs.getInt("z_gyro_offset", 0);
        cfg->mpu_config.x_accel_offset = prefs.getInt("x_accel_offset", 0);
        cfg->mpu_config.y_accel_offset = prefs.getInt("y_accel_offset", 0);
        cfg->mpu_config.z_accel_offset = prefs.getInt("z_accel_offset", 0);
        prefs.end(); // Close namespace "MPU_Config"
}

void mpu_config_save(const char *file_path, Config *cfg)
{
        // Save MPU configuration values to Preferences

        prefs.begin("MPU_Config"); // Open namespace "MPU_Config"
        prefs.putInt("x_gyro_offset", cfg->mpu_config.x_gyro_offset);
        prefs.putInt("y_gyro_offset", cfg->mpu_config.y_gyro_offset);
        prefs.putInt("z_gyro_offset", cfg->mpu_config.z_gyro_offset);
        prefs.putInt("x_accel_offset", cfg->mpu_config.x_accel_offset);
        prefs.putInt("y_accel_offset", cfg->mpu_config.y_accel_offset);
        prefs.putInt("z_accel_offset", cfg->mpu_config.z_accel_offset);
        prefs.end(); // Close namespace "MPU_Config"
}

#endif
