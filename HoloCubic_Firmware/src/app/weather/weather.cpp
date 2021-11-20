#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "../sys/app_contorller.h"
#include "../../network.h"
#include "../../common.h"
#include "ArduinoJson.h"
#include <map>

#define WEATHER_PAGE_SIZE 2

struct WeatherAppRunDate
{
    unsigned long preWeatherMillis;      // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;         // 更新时间计数器
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(900s)
    long long m_preNetTimestamp;         // 上一次的网络时间戳
    long long m_errorNetTimestamp;         // 网络到显示过程中的时间误差
    long long m_preLocalTimestamp;       // 上一次的本地机器时间戳
    int state_flag;                      // 
    bool isUpdating;                     // 当前是否有更新任务
    int clock_page; 

    ESP32Time g_rtc;    // 用于时间解码
    Weather wea;        // 保存天气状况
};

static WeatherAppRunDate *run_data = NULL;

enum wea_event_Id {UPDATE_NOW, UPDATE_NTP, UPDATE_DAILY};
String time_api = "http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp";
String weatherNow_api = "https://www.tianqiapi.com/free/day?appid=45962355&appsecret=hHvu36je&unescape=1";
String weatherDaliy_api = "https://www.tianqiapi.com/free/week?unescape=1&appid=45962355&appsecret=hHvu36je";
std::map<String, int> weatherMap = {{"qing",0},{"yin",1},{"yu",2},{"yun",3},{"bingbao",4},
                        {"wu",5},{"shachen",6},{"lei",7},{"xue",8}};

int windLevelAnalyse(String str) 
{
    int ret = 0;
    for (char ch : str) {
        if (ch >= '0' && ch <= '9') {
            ret = ret * 10 + (ch - '0');
        }
    }
    return ret;
}

void getWeather(void)
{
    if (WL_CONNECTED != WiFi.status())
        return ;

    HTTPClient http;
    http.setTimeout(1000);
    http.begin(weatherNow_api);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            strcpy(run_data->wea.cityname, sk["city"].as<String>().c_str());
            run_data->wea.weather_code = weatherMap[sk["wea_img"].as<String>()];
            run_data->wea.temperature = sk["tem"].as<int>();
            run_data->wea.maxTmep = sk["tem_day"].as<int>();
            run_data->wea.minTemp = sk["tem_night"].as<int>();
            strcpy(run_data->wea.windDir, sk["win"].as<String>().c_str());
            run_data->wea.windLevel = windLevelAnalyse(sk["win_speed"].as<String>());
            run_data->wea.airQulity = airQulityLevel(sk["air"].as<int>());
            run_data->state_flag |= 0x01;
            run_data->preWeatherMillis = millis();
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

long long getTimestamp()
{
    // 使用本地的机器时钟
    run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (millis() - run_data->m_preLocalTimestamp);
    run_data->m_preLocalTimestamp = millis();
    return run_data->m_preNetTimestamp;
}

long long getTimestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            run_data->m_preNetTimestamp = atoll(time.c_str())+run_data->m_errorNetTimestamp+TIMEZERO_OFFSIZE;
            run_data->m_preLocalTimestamp = millis();
            run_data->preTimeMillis = millis();
            run_data->isUpdating = false;
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (millis() - run_data->m_preLocalTimestamp);
        run_data->m_preLocalTimestamp = millis();
    }
    http.end();

    return run_data->m_preNetTimestamp;
}

void getDaliyWeather(short maxT[], short minT[])
{
    if (WL_CONNECTED != WiFi.status())
        return ;

    HTTPClient http;
    http.setTimeout(1000);
    http.begin(weatherDaliy_api);

    int httpCode = http.GET();
    if (httpCode > 0) {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            for (int gDW_i = 0; gDW_i < 7; ++gDW_i) {
                maxT[gDW_i] = sk["data"][gDW_i]["tem_day"].as<int>();
                minT[gDW_i] = sk["data"][gDW_i]["tem_night"].as<int>();
            }
            run_data->state_flag |= 0x10;
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void UpdateTime_RTC(long long timestamp)
{
    struct TimeStr t;
    run_data->g_rtc.setTime(timestamp / 1000);
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour();
    t.minute = run_data->g_rtc.getMinute();
    t.second = run_data->g_rtc.getSecond();
    t.weekday = run_data->g_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    display_time(t);
}

void switch_wea_scr(AppController *sys, lv_scr_load_anim_t anim_type)
{
    if (run_data->clock_page == 0) {
        display_weather_init(anim_type);

        if (run_data->state_flag & 0x01) {
            display_weather(run_data->wea);
        }
    } else if (run_data->clock_page == 1) {
        // 仅在切换界面时获取一次未来天气
        display_curve_init(anim_type);
        ANIEND;
        if (run_data->state_flag & 0x10) {
            display_curve(run_data->wea.daily_max, run_data->wea.daily_min);
        } else {
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_DAILY);
        }
    }
}

void weather_init(void)
{
    tft->setSwapBytes(true);
    weather_gui_init();
    display_weather_init(LV_SCR_LOAD_ANIM_NONE);
    // 初始化运行时参数
    run_data = (WeatherAppRunDate *)malloc(sizeof(WeatherAppRunDate));
    run_data->weatherUpdataInterval = 1800000;      // 天气更新的时间间隔30min
    run_data->timeUpdataInterval = 600000;          // 时钟更新的时间间隔5min
    run_data->m_preNetTimestamp = 1637664720000;    // 上一次的网络时间戳
    run_data->m_errorNetTimestamp = 2;
    run_data->m_preLocalTimestamp = millis();       // 上一次的本地机器时间戳
    run_data->state_flag = 0x00;                 // 
    run_data->isUpdating = false;
    run_data->clock_page = 0; 
    // 强制更新
    run_data->preWeatherMillis = 0;
    run_data->preTimeMillis = 0;
}

void weather_process(AppController *sys,
                     const Imu_Action *act_info)
{
    if (RETURN == act_info->active) {
        sys->app_exit();
        return;
    } else if (GO_FORWORD == act_info->active) {
        run_data->preWeatherMillis = 0;
        run_data->preTimeMillis = 0;
    } else if (TURN_RIGHT == act_info->active) {
        run_data->clock_page = (run_data->clock_page + 1) % WEATHER_PAGE_SIZE;
        switch_wea_scr(sys, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else if (TURN_LEFT == act_info->active) {
        // 以下等效与 clock_page = (clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        // +3为了不让数据溢出成负数，而导致取模逻辑错误
        run_data->clock_page = (run_data->clock_page + WEATHER_PAGE_SIZE - 1) % WEATHER_PAGE_SIZE;
        switch_wea_scr(sys, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }

    if (run_data->clock_page == 0) {
        if (((millis() - run_data->preTimeMillis) > run_data->timeUpdataInterval || 
            run_data->preTimeMillis == 0) && !run_data->isUpdating) {
            // 尝试同步网络上的时钟
            run_data->isUpdating = true;
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_NTP);
        } else if (millis() - run_data->m_preLocalTimestamp > 1000) {
            UpdateTime_RTC(getTimestamp());
        }
        if (((millis() - run_data->preWeatherMillis) > run_data->weatherUpdataInterval || 
            run_data->preWeatherMillis == 0) && !run_data->isUpdating) {
            sys->req_event(&weather_app, APP_EVENT_WIFI_CONN, UPDATE_NOW);
        } 
        display_space();
    } 

    delay(30);
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
    if (event == APP_EVENT_WIFI_CONN)
    {
        Serial.print(millis());
        Serial.print(F("----->weather_event_notification "));
        if (event_id == UPDATE_NOW) {
            Serial.print(F("weather update.\n"));
            getWeather();
            if (run_data->state_flag & 0x01) {
                display_weather(run_data->wea);
            }
        } else if (event_id == UPDATE_NTP) {
            Serial.print(F("ntp update.\n"));
            long long timestamp = getTimestamp(time_api); //nowapi时间API
            UpdateTime_RTC(timestamp);
        } else if (event_id == UPDATE_DAILY) {
            Serial.print(F("daliy update.\n"));
            getDaliyWeather(run_data->wea.daily_max, run_data->wea.daily_min);
            if (run_data->state_flag & 0x10) {
                display_curve(run_data->wea.daily_max, run_data->wea.daily_min);
            }
        }
    }
}

APP_OBJ weather_app = {"Weather", &app_weather, weather_init,
                       weather_process, weather_exit_callback,
                       weather_event_notification};