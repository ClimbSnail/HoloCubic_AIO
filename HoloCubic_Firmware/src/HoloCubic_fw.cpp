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

int image_pos = 0;  // 记录相册播放的历史
int clock_page = 0; // 时钟桌面的播放记录
unsigned long pic_perMillis = 0;
unsigned long preWeatherMillis = 0;           // 上一回更新天气时的毫秒数
unsigned long preTimeMillis = 0;              // 上一回更新日期与时间时的毫秒数
unsigned long weatherUpdataInterval = 900000; // 天气更新的时间间隔
unsigned long timeUpdataInterval = 10000;     // 日期时钟更新的时间间隔

/*** Component objects **7*/
Display screen;
IMU mpu;
Ambient ambLight;
Imu_Active *act_info; // 存放mpu6050返回的数据
Network g_network;    // 网络连接
ESP32Time g_rtc;      // 用于时间解码

String unit = "c";
String time_api = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp";

void clock_app_process(Imu_Active *act_info);
void picture_app_process(Imu_Active *act_info);

//函数指针组
void (*p_processList[2])(Imu_Active *) = {clock_app_process, picture_app_process};
void controller(Imu_Active *act_info); // 对所有控制的通用处理
int processId = 0;

void UpdateWeather(void)
{
    Weather weather = g_network.getWeather("https://api.seniverse.com/v3/weather/now.json?key=" + g_cfg.weather_key + "&location=" + g_cfg.cityname + "&language=" + g_cfg.language + "&unit=" + unit); //如果要改城市这里也需要修改
    Serial.println(weather.weather_code);
    char temperature[10] = {0};
    sprintf(temperature, "%d", weather.temperature);
    display_weather(g_cfg.cityname.c_str(), temperature, weather.weather_code);
}

void UpdateTime_RTC(void)
{
    long int timestamp = g_network.getTimestamp(time_api) + TIMEZERO_OFFSIZE; //nowapi时间API
    g_rtc.setTime(timestamp);
    String date = g_rtc.getDate(String("%Y-%m-%d"));
    String time = g_rtc.getTime(String("%H:%M"));

    Serial.println("-------------------------------------");
    Serial.println(date.c_str());
    Serial.println(time.c_str());
    Serial.println("-------------------------------------");
    display_time(date.c_str(), time.c_str());
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
    // processId == 1 是相册功能
    if (UNKNOWN == act_info->active && 1 != processId)
        return;

    Serial.print("act_info->active: ");
    Serial.println(act_info->active);
    if (RETURN == act_info->active)
    {
        act_info->active = UNKNOWN;
        act_info->isValid = 0;
        processId = (processId + 1) % 2; // 此处的2与p_processList的长度一致
    }

    (*(p_processList[processId]))(act_info);

    act_info->active = UNKNOWN;
    act_info->isValid = 0;
}

void clock_app_process(Imu_Active *act_info)
{
    Serial.println("-----------> clock_app_process");

    lv_scr_load_anim_t direction = LV_SCR_LOAD_ANIM_NONE;

    if (TURN_RIGHT == act_info->active)
    {
        direction = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        clock_page = (clock_page + 1) % 4;
    }
    else if (TURN_LEFT == act_info->active)
    {
        direction = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page - 1 + 3) % 4;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        clock_page = (clock_page + 3) % 4;
    }

    Serial.println("clock_page");
    Serial.println(clock_page);

    if (clock_page != 2)
        lv_scr_load_anim(scr[clock_page], direction, 500, 300, false);

    if (0 == clock_page && millis() - preWeatherMillis >= weatherUpdataInterval)
    {
        // 更新天气
        preWeatherMillis = millis();
        UpdateWeather();
    }

    if (1 == clock_page && millis() - preTimeMillis >= timeUpdataInterval)
    {
        // 更新时钟
        preTimeMillis = millis();
        UpdateTime_RTC();
    }

    if (2 == clock_page)
    {
        // NULL后期可以是具体数据
        display_hardware(NULL);
    }

    if (3 == clock_page)
    {
        // 如果STA模式连接失败 切换成ap模式
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            g_network.open_ap(AP_SSID);
        }
        display_setting(g_network.get_localIp().c_str(),
                        g_network.get_softAPIP().c_str(),
                        "Domain: holocubic",
                        "WebServer Start");
        g_network.start_web_config();
    }
}

void picture_app_process(Imu_Active *act_info)
{
    Serial.println("-----------> picture_app_process");
    lv_scr_load_anim_t direction = LV_SCR_LOAD_ANIM_NONE;

    // if (TURN_RIGHT == act_info->active)
    // {
    //     direction = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    // }
    // else if (TURN_LEFT == act_info->active)
    // {
    //     direction = LV_SCR_LOAD_ANIM_MOVE_LEFT;
    // }

    if (doDelayMillisTime(3000, &pic_perMillis, false) == true)
    {
        ++image_pos;
    }

    if (image_pos >= photo_file_num)
    {
        image_pos = 0;
    }

    display_photo(file_name_list[image_pos]);
    lv_scr_load_anim(image, direction, 100, 0, false);
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
    UpdateWeather();
    act_info = mpu.update(200);
    act_info->active = GO_FORWORD; // 让第一次能够执行刷新屏幕的判断
}

void loop()
{
    // unsigned int x = ambLight.getLux();
    // Serial.print("AmbLight: ");
    // Serial.println(x);

    screen.routine();
    controller(act_info); // 运行当前进程
    delay(300);
    act_info = mpu.update(200);

    rgb.setBrightness(ambLight.getLux() / 500.0);
    wifi_auto_process();   // 任务调度
    server.handleClient(); // 一定需要放在循环里扫描
    //malloc(2);
}