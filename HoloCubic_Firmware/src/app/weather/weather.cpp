#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "sys/app_contorller.h"
#include "network.h"
#include "common.h"
#include "ArduinoJson.h"
#include <esp32-hal-timer.h>
#include <map>

#define WEATHER_NOW_API "https://www.tianqiapi.com/free/day?appid=%s&appsecret=%s&unescape=1&city=%s"   // &city=%s 
#define WEATHER_DALIY_API "https://www.tianqiapi.com/free/week?unescape=1&appid=%s&appsecret=%s"
#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp"
#define WEATHER_PAGE_SIZE 2

struct WeatherAppRunData
{
    unsigned long preWeatherMillis;      // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;         // 更新时间计数器
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(900s)
    long long preNetTimestamp;           // 上一次的网络时间戳
    long long errorNetTimestamp;         // 网络到显示过程中的时间误差
    long long preLocalTimestamp;         // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag;      // 强制更新标志
    int clock_page;

    ESP32Time g_rtc; // 用于时间解码
    Weather wea;     // 保存天气状况
};

static WeatherAppRunData *run_data = NULL;

enum wea_event_Id
{
    UPDATE_NOW,
    UPDATE_NTP,
    UPDATE_DAILY
};

std::map<String, int> weatherMap = {{"qing", 0}, {"yin", 1}, {"yu", 2}, {"yun", 3}, {"bingbao", 4}, {"wu", 5}, {"shachen", 6}, {"lei", 7}, {"xue", 8}};

static int windLevelAnalyse(String str)
{
    int ret = 0;
    for (char ch : str)
    {
        if (ch >= '0' && ch <= '9')
        {
            ret = ret * 10 + (ch - '0');
        }
    }
    return ret;
}

static void getWeather(void)
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = "";
    snprintf(api, 128, WEATHER_NOW_API, g_cfg.tianqi_appid, g_cfg.tianqi_appsecret, g_cfg.tianqi_addr);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            strcpy(run_data->wea.cityname, sk["city"].as<String>().c_str());
            run_data->wea.weather_code = weatherMap[sk["wea_img"].as<String>()];
            run_data->wea.temperature = sk["tem"].as<int>();
            run_data->wea.humidity = 50;
            run_data->wea.maxTmep = sk["tem_day"].as<int>();
            run_data->wea.minTemp = sk["tem_night"].as<int>();
            strcpy(run_data->wea.windDir, sk["win"].as<String>().c_str());
            run_data->wea.windLevel = windLevelAnalyse(sk["win_speed"].as<String>());
            run_data->wea.airQulity = airQulityLevel(sk["air"].as<int>());
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

static long long getTimestamp()
{
    // 使用本地的机器时钟
    run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
    run_data->preLocalTimestamp = millis();
    return run_data->preNetTimestamp;
}

static long long getTimestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            run_data->preNetTimestamp = atoll(time.c_str()) + run_data->errorNetTimestamp + TIMEZERO_OFFSIZE;
            run_data->preLocalTimestamp = millis();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
        run_data->preLocalTimestamp = millis();
    }
    http.end();

    return run_data->preNetTimestamp;
}

static void getDaliyWeather(short maxT[], short minT[])
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = "";
    snprintf(api, 128, WEATHER_DALIY_API, g_cfg.tianqi_appid, g_cfg.tianqi_appsecret);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            for (int gDW_i = 0; gDW_i < 7; ++gDW_i)
            {
                maxT[gDW_i] = sk["data"][gDW_i]["tem_day"].as<int>();
                minT[gDW_i] = sk["data"][gDW_i]["tem_night"].as<int>();
            }
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

static void UpdateTime_RTC(long long timestamp)
{
    struct TimeStr t;
    run_data->g_rtc.setTime(timestamp / 1000);
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour(true);
    t.minute = run_data->g_rtc.getMinute();
    t.second = run_data->g_rtc.getSecond();
    t.weekday = run_data->g_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    display_time(t, LV_SCR_LOAD_ANIM_NONE);
}

static void weather_init(void)
{
    tft->setSwapBytes(true);
    weather_gui_init();
    // 初始化运行时参数
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    memset((char *)&run_data->wea, 0, sizeof(Weather));
    run_data->weatherUpdataInterval = 900000;  // 天气更新的时间间隔
    run_data->timeUpdataInterval = 900000;     // 日期时钟更新的时间间隔(900s)
    run_data->preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->errorNetTimestamp = 2;
    run_data->preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    run_data->clock_page = 0;
    run_data->preWeatherMillis = 0;
    run_data->preTimeMillis = 0;
    // 强制更新
    run_data->coactusUpdateFlag = 0x01;
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
    else if (GO_FORWORD == act_info->active)
    {
        // 间接强制更新
        run_data->coactusUpdateFlag = 0x01;
        delay(500); // 以防间接强制更新后，生产很多请求 使显示卡顿
    }
    else if (TURN_RIGHT == act_info->active)
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

    // 界面刷新
    if (run_data->clock_page == 0)
    {
        display_weather(run_data->wea, anim_type);
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(run_data->weatherUpdataInterval, &run_data->preWeatherMillis, false))
        {
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_NOW);
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_DAILY);
        }

        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(run_data->timeUpdataInterval, &run_data->preTimeMillis, false))
        {
            // 尝试同步网络上的时钟
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_NTP);
        }
        else if (millis() - run_data->preLocalTimestamp > 400)
        {
            UpdateTime_RTC(getTimestamp());
        }
        run_data->coactusUpdateFlag = 0x00; // 取消强制更新标志
        display_space();
        delay(30);
    }
    else if (run_data->clock_page == 1)
    {
        // 仅在切换界面时获取一次未来天气
        display_curve(run_data->wea.daily_max, run_data->wea.daily_min, anim_type);
        delay(300);
    }
}

static void weather_exit_callback(void)
{
    weather_gui_del();
    // 释放运行数据
    free(run_data);
    run_data = NULL;
}

static void weather_event_notification(APP_EVENT_TYPE type, int event_id)
{
    if (type == APP_EVENT_WIFI_CONN)
    {
        Serial.println(F("----->weather_event_notification"));
        switch (event_id)
        {
        case UPDATE_NOW:
        {
            Serial.print(F("weather update.\n"));
            getWeather();
            if (run_data->clock_page == 0)
            {
                display_weather(run_data->wea, LV_SCR_LOAD_ANIM_NONE);
            }
        };
        break;
        case UPDATE_NTP:
        {
            Serial.print(F("ntp update.\n"));
            long long timestamp = getTimestamp(TIME_API); // nowapi时间API
            if (run_data->clock_page == 0)
            {
                UpdateTime_RTC(timestamp);
            }
        };
        break;
        case UPDATE_DAILY:
        {
            Serial.print(F("daliy update.\n"));
            getDaliyWeather(run_data->wea.daily_max, run_data->wea.daily_min);
            if (run_data->clock_page == 1)
            {
                display_curve(run_data->wea.daily_max, run_data->wea.daily_min, LV_SCR_LOAD_ANIM_NONE);
            }
        };
        break;
        default:
            break;
        }
    }
}

APP_OBJ weather_app = {"Weather", &app_weather, "", weather_init,
                       weather_process, weather_exit_callback,
                       weather_event_notification};