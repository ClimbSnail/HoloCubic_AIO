#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "sys/app_contorller.h"
#include "network.h"
#include "common.h"

#define WEATHER_PAGE_SIZE 2

struct WeatherAppRunDate
{
    unsigned long preWeatherMillis;      // 上一回更新天气时的毫秒数
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔
    int clock_page;

    Weather weather; // 保存天气状况
};

static WeatherAppRunDate *run_data = NULL;

String unit = "c";

Weather getWeather(void)
{
    return run_data->weather;
}

Weather getWeather(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return run_data->weather;

    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    // start connection and send HTTP headerFFF
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            int code_index = (payload.indexOf("code")) + 7;         //获取code位置
            int temp_index = (payload.indexOf("temperature")) + 14; //获取temperature位置
            run_data->weather.weather_code =
                atol(payload.substring(code_index, temp_index - 17).c_str());
            run_data->weather.temperature =
                atol(payload.substring(temp_index, payload.length() - 47).c_str());
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();

    return run_data->weather;
}

void UpdateWeather(Weather *weather, lv_scr_load_anim_t anim_type)
{
    char temperature[10] = {0};
    sprintf(temperature, "%d", weather->temperature);
    display_weather(g_cfg.cityname.c_str(), temperature, weather->weather_code, anim_type);
}

void UpdateTime_RTC(ESP32Time g_rtc, lv_scr_load_anim_t anim_type)
{
    String date = g_rtc.getDate(String("%Y-%m-%d"));
    String time = g_rtc.getTime(String("%H:%M:%S"));

    display_time(date.c_str(), time.c_str(), anim_type);
}

void weather_init(void)
{
    weather_gui_init();
    // 初始化运行时参数
    run_data = (WeatherAppRunDate *)malloc(sizeof(WeatherAppRunDate));
    run_data->weatherUpdataInterval = 900000; // 天气更新的时间间隔
    run_data->clock_page = 0;                 // 时钟桌面的播放记录

    // 变相强制更新
    run_data->preWeatherMillis = millis() - run_data->weatherUpdataInterval;
    g_time_utile.force_update();

    run_data->weather = {0, 0};
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
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        run_data->clock_page = (run_data->clock_page + 1) % WEATHER_PAGE_SIZE;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        // 以下等效与 clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        run_data->clock_page = (run_data->clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
    }
    else if (GO_FORWORD == act_info->active)
    {
        // 后仰时，变相强制更新
        run_data->preWeatherMillis = millis() - run_data->weatherUpdataInterval;
        g_time_utile.force_update();
    }

    if (0 == run_data->clock_page) // 更新天气
    {
        Weather weather = getWeather();
        UpdateWeather(&weather, anim_type);
        // 以下减少网络请求的压力
        if (doDelayMillisTime(run_data->weatherUpdataInterval, &run_data->preWeatherMillis, false))
        {
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, run_data->clock_page);
        }
    }

    if (1 == run_data->clock_page) // 更新时钟
    {
        UpdateTime_RTC(g_time_utile.g_rtc, anim_type);
    }
    if (2 == run_data->clock_page) // NULL后期可以是具体数据
    {
        display_hardware(NULL, anim_type);
    }
    delay(300);
}

void weather_exit_callback(void)
{
    weather_gui_del();

    // 释放运行数据
    free(run_data);
    run_data = NULL;
}

void weather_event_notification(APP_EVENT event, int event_id)
{
    switch (event)
    {
    case APP_EVENT_WIFI_CONN:
    {
        Serial.print(millis());
        Serial.print(F("----->weather_event_notification\n"));
        if (0 == run_data->clock_page && run_data->clock_page == event_id)
        {
            //如果要改城市这里也需要修改
            Weather weather = getWeather("https://api.seniverse.com/v3/weather/now.json?key=" +
                                         g_cfg.weather_key + "&location=" + g_cfg.cityname + "&language=" +
                                         g_cfg.language + "&unit=" + unit);
            UpdateWeather(&weather, LV_SCR_LOAD_ANIM_NONE);
        }
    }
    break;
    case APP_EVENT_WIFI_AP:
    {
    }
    default:
        break;
    }
}

APP_OBJ weather_app = {"Weather", &app_weather, "", weather_init,
                       weather_process, weather_exit_callback,
                       weather_event_notification};