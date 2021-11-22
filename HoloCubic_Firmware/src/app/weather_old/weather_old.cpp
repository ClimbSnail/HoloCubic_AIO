#include "weather_old.h"
#include "weather_old_gui.h"
#include "ESP32Time.h"
#include "sys/app_contorller.h"
#include "network.h"
#include "common.h"

#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp"
#define ZHIXIN_WEATHER_API "https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=%s&unit=c"
#define WEATHER_PAGE_SIZE 2

struct Weather
{
    int weather_code;
    int temperature;
};

struct WeatherAppRunDate
{
    unsigned long preWeatherMillis;      // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;         // 更新时间计数器
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(900s)
    long long m_preNetTimestamp;         // 上一次的网络时间戳
    long long m_errorNetTimestamp;       // 网络到显示过程中的时间误差
    long long m_preLocalTimestamp;       // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag;      // 强制更新标志
    int clock_page;                      // 时钟桌面的播放记录

    ESP32Time g_rtc; // 用于时间解码
    Weather weather; // 保存天气状况
};

static WeatherAppRunDate *run_data = NULL;

static Weather getWeather(void)
{
    return run_data->weather;
}

static Weather getWeather(String url)
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

static long long getTimestamp()
{
    // 使用本地的机器时钟
    run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (millis() - run_data->m_preLocalTimestamp);
    run_data->m_preLocalTimestamp = millis();

    return run_data->m_preNetTimestamp;
}

static long long getTimestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    // start connection and send HTTP headerFFF
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            run_data->m_preNetTimestamp = atoll(time.c_str()) + run_data->m_errorNetTimestamp;
            run_data->m_preLocalTimestamp = millis();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (millis() - run_data->m_preLocalTimestamp);
        run_data->m_preLocalTimestamp = millis();
    }
    http.end();

    return run_data->m_preNetTimestamp;
}

static void UpdateWeather(Weather *weather, lv_scr_load_anim_t anim_type)
{
    char temperature[10] = {0};
    sprintf(temperature, "%d", weather->temperature);
    display_weather_old(g_cfg.cityname.c_str(), temperature, weather->weather_code, anim_type);
}

static void UpdateTime_RTC(long long timestamp, lv_scr_load_anim_t anim_type)
{
    run_data->g_rtc.setTime(timestamp / 1000);
    String date = run_data->g_rtc.getDate(String("%Y-%m-%d"));
    String time = run_data->g_rtc.getTime(String("%H:%M:%S"));
    display_time_old(date.c_str(), time.c_str(), anim_type);
}

static void weather_init(void)
{
    weather_old_gui_init();
    // 初始化运行时参数
    run_data = (WeatherAppRunDate *)calloc(1, sizeof(WeatherAppRunDate));
    run_data->weatherUpdataInterval = 900000;    // 天气更新的时间间隔
    run_data->timeUpdataInterval = 900000;       // 日期时钟更新的时间间隔(900s)
    run_data->m_preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化围殴2020-01-01 00:00:00
    run_data->m_errorNetTimestamp = 2;
    run_data->m_preLocalTimestamp = 0; // 上一次的本地机器时间戳
    run_data->clock_page = 0;          // 时钟桌面的播放记录
    // 变相强制更新
    run_data->preWeatherMillis = millis() - run_data->weatherUpdataInterval;
    run_data->preTimeMillis = millis() - run_data->timeUpdataInterval;
    run_data->coactusUpdateFlag = 0x01;

    run_data->weather = {0, 0};
}

static void weather_process(AppController *sys,
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
        run_data->coactusUpdateFlag = 0x01;
    }

    if (0 == run_data->clock_page) // 更新天气
    {
        Weather weather = getWeather();
        UpdateWeather(&weather, anim_type);
        // 以下减少网络请求的压力
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(run_data->weatherUpdataInterval, &run_data->preWeatherMillis, false))
        {
            sys->req_event(&weather_old_app, APP_EVENT_WIFI_CONN, run_data->clock_page);
            run_data->coactusUpdateFlag = 0x00;
        }
    }

    // 界面刷新
    if (1 == run_data->clock_page) // 更新时钟
    {
        // 使用本地的机器时钟
        long long timestamp = getTimestamp() + TIMEZERO_OFFSIZE; // nowapi时间API
        UpdateTime_RTC(timestamp, anim_type);
        // 以下减少网络请求的压力
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(run_data->timeUpdataInterval, &run_data->preTimeMillis, false))
        {
            // 尝试同步网络上的时钟
            sys->req_event(&weather_old_app, APP_EVENT_WIFI_CONN, run_data->clock_page);
            run_data->coactusUpdateFlag = 0x00;
        }
    }
    else if (2 == run_data->clock_page) // NULL后期可以是具体数据
    {
        display_hardware_old(NULL, anim_type);
    }

    delay(300);
}

static void weather_exit_callback(void)
{
    weather_old_gui_del();

    // 释放运行数据
    free(run_data);
    run_data = NULL;
}

static void weather_event_notification(APP_EVENT event, int event_id)
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
            char api[128] = "";
            snprintf(api, 128, ZHIXIN_WEATHER_API, g_cfg.weather_key.c_str(), g_cfg.cityname.c_str(), g_cfg.language.c_str());
            Weather weather = getWeather(api);
            // Weather weather = getWeather("https://api.seniverse.com/v3/weather/now.json?key=" +
            //                              g_cfg.weather_key + "&location=" + g_cfg.cityname + "&language=" +
            //                              g_cfg.language + "&unit=" + unit);
            UpdateWeather(&weather, LV_SCR_LOAD_ANIM_NONE);
        }
        else if (1 == run_data->clock_page && run_data->clock_page == event_id)
        {
            long long timestamp = getTimestamp(TIME_API) + TIMEZERO_OFFSIZE; // nowapi时间API
            UpdateTime_RTC(timestamp, LV_SCR_LOAD_ANIM_NONE);
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

APP_OBJ weather_old_app = {"Weather Old", &app_weather_old, "", weather_init,
                           weather_process, weather_exit_callback,
                           weather_event_notification};