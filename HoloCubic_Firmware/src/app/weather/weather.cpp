#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "../sys/app_contorller.h"
#include "../../network.h"
#include "../../common.h"

#define WEATHER_PAGE_SIZE 2

ESP32Time g_rtc;                              // 用于时间解码
int clock_page = 0;                           // 时钟桌面的播放记录int clock_page = 0;        // 时钟桌面的播放记录
unsigned long preWeatherMillis = 0;           // 上一回更新天气时的毫秒数
unsigned long preTimeMillis = 0;              // 上一回从网络更新日期与时间时的毫秒数
unsigned long weatherUpdataInterval = 900000; // 天气更新的时间间隔
unsigned long timeUpdataInterval = 300000;    // 日期时钟更新的时间间隔(300s)

// preWeatherMillis = millis() - weatherUpdataInterval;
// preTimeMillis = millis() - timeUpdataInterval;
String unit = "c";
String time_api = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp";

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

void weather_init(void)
{
    weather_gui_init();
}

void weather_process(AppController *sys,
                     const Imu_Action *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        // 切换界面时，便向强制更新
        preWeatherMillis = millis() - weatherUpdataInterval;
        preTimeMillis = millis() - timeUpdataInterval;
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        clock_page = (clock_page + 1) % WEATHER_PAGE_SIZE;
    }
    else if (TURN_LEFT == act_info->active)
    {
        // 切换界面时，便向强制更新
        preWeatherMillis = millis() - weatherUpdataInterval;
        preTimeMillis = millis() - timeUpdataInterval;
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
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
    delay(300);
}

void weather_exit_callback(void)
{
    weather_gui_del();
}

void weather_event_notification(APP_EVENT event)
{
    if (event == APP_EVENT_WIFI_CONN)
    {
    }
}

APP_OBJ weather_app = {"Weather", &app_weather, weather_init,
                       weather_process, weather_exit_callback,
                       weather_event_notification};