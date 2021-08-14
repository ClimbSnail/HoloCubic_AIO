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
    cfg->cityname = prefs.getString("cityname", "Wuhan");
    cfg->UID = prefs.getString("UID", "");
    cfg->language = prefs.getString("language", "zh-Hans");
    cfg->weather_key = prefs.getString("weather_key", "");
    prefs.end(); // 关闭当前命名空间
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
    prefs.putString("UID", cfg->UID);
    prefs.putString("language", cfg->language);
    prefs.putString("weather_key", cfg->weather_key);
    prefs.end(); // 关闭当前命名空间
}