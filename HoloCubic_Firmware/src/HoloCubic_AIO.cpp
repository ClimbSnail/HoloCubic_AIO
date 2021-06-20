#include "display.h"
#include "imu.h"
#include "ambient.h"
#include "network.h"
#include "lv_port_indev.h"
#include "lv_cubic_gui.h"
#include "lv_demo_encoder.h"
#include "lv_port_fatfs.h"
#include "common.h"
#include "config.h"
#include "ESP32Time.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiMulti.h> // 当我们需要使用ESP8266开发板存储多个WiFi网络连接信息时，可以使用ESP8266WiFiMulti库来实现。

int image_pos = 0;                            // 记录相册播放的历史
int clock_page = 0;                           // 时钟桌面的播放记录
unsigned long pic_perMillis = 0;              // 图片上一回更新的时间
unsigned long preWeatherMillis = 0;           // 上一回更新天气时的毫秒数
unsigned long preTimeMillis = 0;              // 上一回从网络更新日期与时间时的毫秒数
unsigned long picRefreshInterval = 5000;      // 图片播放的时间间隔(10s)
unsigned long weatherUpdataInterval = 900000; // 天气更新的时间间隔
unsigned long timeUpdataInterval = 300000;    // 日期时钟更新的时间间隔(300s)

/*** Component objects **7*/
Display screen;
IMU mpu;
Ambient ambLight;
Imu_Active *act_info;      // 存放mpu6050返回的数据
Network g_network;         // 网络连接
ESP32Time g_rtc;           // 用于时间解码
boolean app_exit_flag = 0; // 表示是否退出APP应用

String unit = "c";
String time_api = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp";

void clock_app_process(Imu_Active *act_info);
void picture_app_process(Imu_Active *act_info);
void movie_app_process(Imu_Active *act_info);
void screen_app_process(Imu_Active *act_info);
void server_app_process(Imu_Active *act_info);

//函数指针组
void (*p_processList[APP_NUM])(Imu_Active *) = {clock_app_process, picture_app_process,
                                                movie_app_process, screen_app_process, server_app_process};
void controller(Imu_Active *act_info); // 对所有控制的通用处理
int processId = 0;

void UpdateWeather(lv_scr_load_anim_t anim_type)
{
    Weather weather;
    // 以下减少网络请求的压力
    if (millis() - preWeatherMillis >= weatherUpdataInterval)
    {
        preWeatherMillis = millis();
        //如果要改城市这里也需要修改
        weather = g_network.getWeather("https://api.seniverse.com/v3/weather/now.json?key=" + g_cfg.weather_key + "&location=" + g_cfg.cityname + "&language=" + g_cfg.language + "&unit=" + unit);
    }
    else
    {
        weather = g_network.getWeather();
    }
    char temperature[10] = {0};
    sprintf(temperature, "%d", weather.temperature);
    display_weather(g_cfg.cityname.c_str(), temperature, weather.weather_code, anim_type);
}

void UpdateTime_RTC(lv_scr_load_anim_t anim_type)
{
    long long timestamp = 0;
    // 以下减少网络请求的压力
    if (millis() - preTimeMillis >= timeUpdataInterval)
    {
        // 尝试同步网络上的时钟
        preTimeMillis = millis();
        timestamp = g_network.getTimestamp(time_api) + TIMEZERO_OFFSIZE; //nowapi时间API
    }
    else
    {
        // 使用本地的机器时钟
        timestamp = g_network.getTimestamp() + TIMEZERO_OFFSIZE; //nowapi时间API
    }

    g_rtc.setTime(timestamp / 1000);
    String date = g_rtc.getDate(String("%Y-%m-%d"));
    String time = g_rtc.getTime(String("%H:%M"));

    display_time(date.c_str(), time.c_str(), anim_type);
}

void wifi_auto_process()
{
    if (doDelayMillisTime(g_network.m_wifiClickInterval, &g_network.m_preWifiClickMillis, false) == true)
    {
        preWeatherMillis = millis() - weatherUpdataInterval;
        preTimeMillis = millis() - timeUpdataInterval;
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            // todo
            // if (AP_ENABLE == g_network.getApStatus())
            // {
            // }
        }
    }
}

void controller(Imu_Active *act_info)
{
    if (UNKNOWN != act_info->active)
    {
        Serial.print("act_info->active: ");
        Serial.println(act_info->active);
    }

    if (0 == app_exit_flag)
    {
        // 当前没有进入任何app
        lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
        if (TURN_RIGHT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
            processId = (processId + 1) % APP_NUM;
        }
        else if (TURN_LEFT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
            // 以下等效与 processId = (processId - 1 + APP_NUM) % 4;
            // +3为了不让数据溢出成负数，而导致取模逻辑错误
            processId = (processId - 1 + APP_NUM) % APP_NUM; // 此处的3与p_processList的长度一致
        }
        else if (GO_FORWORD == act_info->active)
        {
            app_exit_flag = 1; // 进入app
        }

        display_app_scr(processId, anim_type, false);
    }
    else
    {
        // 把控制权交给当前APP
        (*(p_processList[processId]))(act_info);
        delay(300);
    }
    act_info->active = UNKNOWN;
    act_info->isValid = 0;
}

void clock_app_process(Imu_Active *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        app_exit_flag = 0; // 退出APP
        display_app_scr(processId, LV_SCR_LOAD_ANIM_NONE, true);
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        clock_page = (clock_page + 1) % 3;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page - 1 + 3) % 3;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        clock_page = (clock_page + 2) % 3;
    }

    if (0 == clock_page) // 更新天气
    {
        UpdateWeather(anim_type);
    }

    if (1 == clock_page) // 更新时钟
    {
        UpdateTime_RTC(anim_type);
    }

    if (2 == clock_page) // NULL后期可以是具体数据
    {
        display_hardware(NULL, anim_type);
    }
}

void picture_app_process(Imu_Active *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    static int image_pos_increate = 1;

    if (RETURN == act_info->active)
    {
        app_exit_flag = 0; // 退出APP
        display_app_scr(processId, LV_SCR_LOAD_ANIM_NONE, true);
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        image_pos_increate = 1;
        pic_perMillis = millis() - picRefreshInterval; // 间接强制更新
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        image_pos_increate = -1;
        pic_perMillis = millis() - picRefreshInterval; // 间接强制更新
    }

    if (doDelayMillisTime(picRefreshInterval, &pic_perMillis, false) == true)
    {
        // photo_file_num为了防止当 image_pos_increate为-1时，数据取模出错
        image_pos = (image_pos + image_pos_increate + photo_file_num) % photo_file_num;
        Serial.print("image_pos: ");
        Serial.println(image_pos);
        display_photo(file_name_list[image_pos], anim_type);
    }
}

void movie_app_process(Imu_Active *act_info)
{
    app_exit_flag = 0; // 退出APP
}

void screen_app_process(Imu_Active *act_info)
{
    app_exit_flag = 0; // 退出APP
}

void server_app_process(Imu_Active *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == act_info->active)
    {
        g_network.stop_web_config();
        display_app_scr(processId, LV_SCR_LOAD_ANIM_NONE, true);
        app_exit_flag = 0; // 退出APP
        return;
    }

    if (0 == g_network.m_web_start)
    {
        // 如果STA模式连接失败 切换成ap模式
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            g_network.open_ap(AP_SSID);
        }
        display_setting(g_network.get_localIp().c_str(),
                        g_network.get_softAPIP().c_str(),
                        "Domain: holocubic",
                        "WebServer Start", anim_type);
        g_network.start_web_config();
    }
    server.handleClient(); // 一定需要放在循环里扫描
}

void setup()
{
    Serial.begin(115200);

    /*** Init screen ***/
    screen.init();
    screen.setBackLight(0.1);

    /*** Init IMU as input device ***/
    lv_port_indev_init();
    mpu.init();

    /*** Init on-board RGB ***/
    rgb.init();
    rgb.setBrightness(0.1).setRGB(0, 122, 204);

    /*** Init ambient-light sensor ***/
    ambLight.init(ONE_TIME_H_RESOLUTION_MODE);

    /*** Init micro SD-Card ***/
    tf.init();
    lv_fs_if_init();
    config_read("/wifi.txt", &g_cfg);
    tf.listDir("/image", 250);

    /*** Read WiFi info in SD-Card, then scan & connect WiFi ***/
    g_network.init(g_cfg.ssid, g_cfg.password);

    display_init();
    display_app_scr(processId, LV_SCR_LOAD_ANIM_NONE, true);
    act_info = mpu.update(200);
}

void loop()
{
    // unsigned int x = ambLight.getLux();
    // Serial.print("AmbLight: ");
    // Serial.println(x);

    screen.routine();
    controller(act_info); // 运行当前进程
    act_info = mpu.update(200);

    rgb.setBrightness(ambLight.getLux() / 500.0);
    wifi_auto_process(); // 任务调度
    //malloc(2);
}