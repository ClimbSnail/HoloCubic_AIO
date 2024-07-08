#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ArduinoJson.h"
#include "ArduinoUZlib.h"
#include "URLCode.h"
#include <esp32-hal-timer.h>
#include <map>

#define WEATHER_APP_NAME "Weather"

#define QWEATHER_NOW_API "https://devapi.qweather.com/v7/weather/now?location=%s&key=%s"
#define QWEATHER_3_DAY_API "https://devapi.qweather.com/v7/weather/3d?location=%s&key=%s"
#define QWEATHER_DALIY_API "https://devapi.qweather.com/v7/weather/7d?location=%s&key=%s"
#define QWEATHER_AIR_QUALITY_NOW_API "https://devapi.qweather.com/v7/air/now?location=%s&key=%s"
#define QWEATHER_LOCATION_ID_API "https://geoapi.qweather.com/v2/city/lookup?range=%s&adm=%s&location=%s&key=%s"

#define TIME_API "https://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp"
#define IP_INFO_API "https://ip.useragentinfo.com/json"

#define UPDATE_WEATHER 0x01       // 更新天气
#define UPDATE_DALIY_WEATHER 0x02 // 更新每天天气
#define UPDATE_TIME 0x04          // 更新时间
#define UPDATE_IP_LOCATION 0x08      // 更新位置

// 天气的持久化配置
#define WEATHER_CONFIG_PATH "/weather_218.cfg"

struct Location_Info
{
    String country;        // 国家
    String province;       // 省份
    String city;           // 城市
    String area;           // 区县
    String LocationId;     // 自动获取，无需在Web Server中显示
};

struct WT_Config
{
    String key;                          // 和风天气KEY
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔(s)
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
    uint8_t auto_get_location;           // 0: 手动设置位置，1: 自动获取位置
    Location_Info location_info;         // 地理位置
};

struct WeatherAppRunData
{
    unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;
    unsigned int update_type; // 更新类型的标志位

    BaseType_t xReturned_task_task_update; // 更新数据的异步任务
    TaskHandle_t xHandle_task_task_update; // 更新数据的异步任务

    ESP32Time g_rtc; // 用于时间解码
    Weather wea;     // 保存天气状况
};

struct Task_Param
{
    WeatherAppRunData *task_run_data;
    WT_Config *task_cfg_data;
};

enum wea_event_Id
{
    UPDATE_NOW,
    UPDATE_NTP,
    UPDATE_DAILY
};

static WT_Config *cfg_data = NULL;
static WeatherAppRunData *run_data = NULL;
static Task_Param *task_param_data = NULL;

static void write_config(WT_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    cfg->location_info.country.toLowerCase();
    w_data = w_data + cfg->key + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->weatherUpdataInterval);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->timeUpdataInterval);
    w_data += tmp;
    w_data = w_data + cfg->auto_get_location + "\n";
    w_data = w_data + cfg->location_info.country + "\n";
    w_data = w_data + cfg->location_info.province + "\n";
    w_data = w_data + cfg->location_info.city + "\n";
    w_data = w_data + cfg->location_info.area + "\n";
    w_data = w_data + cfg->location_info.LocationId + "\n";
    g_flashCfg.writeFile(WEATHER_CONFIG_PATH, w_data.c_str());
}

static void read_config(WT_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(WEATHER_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->key = "";
        cfg->auto_get_location = 1;
        cfg->weatherUpdataInterval = 900000; // 天气更新的时间间隔900000(900s)
        cfg->timeUpdataInterval = 900000;    // 日期时钟更新的时间间隔900000(900s)
        cfg->location_info={"cn", "北京市", "北京", "朝阳", "101010300"};
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[10] = {0};
        analyseParam(info, 9, param);
        cfg->key = param[0];
        cfg->weatherUpdataInterval = atol(param[1]);
        cfg->timeUpdataInterval = atol(param[2]);
        cfg->auto_get_location = atoi(param[3]);
        cfg->location_info.country = param[4];
        cfg->location_info.country.toLowerCase();
        cfg->location_info.province = param[5];
        cfg->location_info.city = param[6];
        cfg->location_info.area = param[7];
        cfg->location_info.LocationId = param[8];
    }
}

std::map<String, int> weatherMap = {{"100", 0}, {"101", 1}, {"102", 2}, {"103", 3}, {"104", 4},
                                    {"150", 5}, {"151", 6}, {"152", 7}, {"153", 8}, {"300", 9},
                                    {"301", 10}, {"302", 11}, {"303", 12}, {"304", 13}, {"305", 14},
                                    {"306", 15}, {"307", 16}, {"308", 17}, {"309", 18}, {"310", 19},
                                    {"311", 20}, {"312", 21}, {"313", 22}, {"314", 23}, {"315", 24},
                                    {"316", 25}, {"317", 26}, {"318", 27}, {"350", 28}, {"351", 29},
                                    {"399", 30}, {"400", 31}, {"401", 32}, {"402", 33}, {"403", 34},
                                    {"404", 35}, {"405", 36}, {"406", 37}, {"407", 38}, {"408", 39},
                                    {"409", 40}, {"410", 41}, {"456", 42}, {"457", 43}, {"499", 44},
                                    {"500", 45}, {"501", 46}, {"502", 47}, {"503", 48}, {"504", 49},
                                    {"507", 50}, {"508", 51}, {"509", 52}, {"510", 53}, {"511", 54},
                                    {"512", 55}, {"513", 56}, {"514", 57}, {"515", 58}, {"900", 59},
                                    {"901", 60}, {"999", 61}};

static void task_update(void *parameter); // 异步更新任务

static bool getRestfulAPI(const char *url, JsonDocument &doc)
{
    while (WL_CONNECTED != WiFi.status())
    {
        Serial.println(F("WiFi not connected, wait for 100ms"));
        delay(100);
    }

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    static const char *headers[] = {"Content-Encoding"};
    Serial.print(F("Request "));
    Serial.println(url);
    for (uint8_t i = 0; i < 3; i++)
    {
        bool shouldRetry = false;
        if (http.begin(client, url))
        {
            http.collectHeaders(headers, sizeof(headers[0]) / sizeof(headers));
            int httpCode = http.GET();
            if (httpCode == 200)
            {
                DeserializationError error;
                if (http.header(headers[0]).indexOf("gzip") > -1)
                {
                    uint8_t *response = (uint8_t *)calloc(1, http.getSize());
                    http.getStream().readBytes(response, http.getSize());
                    uint8_t *buffer = NULL;
                    uint32_t size = 0;
                    ArduinoUZlib::decompress(response, http.getSize(), buffer, size);
                    free(response);
                    error = deserializeJson(doc, buffer, size);
                    if (buffer != NULL)
                    {
                        free(buffer);
                        buffer = NULL;
                    }
                    if (!error)
                    {
                        http.end();
                        return true;
                    }
                }
                else
                {
                    error = deserializeJson(doc, client);
                    if (!error)
                    {
                        http.end();
                        return true;
                    }
                }
                if (error)
                {
                    Serial.print(F("Parse JSON failed, error: "));
                    Serial.println(error.f_str());
                    shouldRetry = error == DeserializationError::IncompleteInput;
                }
            }
            else
            {
                Serial.print(F("GET failed, error: "));
                if (httpCode < 0)
                {
                    Serial.println(http.errorToString(httpCode));
                    shouldRetry = httpCode == HTTPC_ERROR_CONNECTION_REFUSED ||
                                    httpCode == HTTPC_ERROR_CONNECTION_LOST ||
                                    httpCode == HTTPC_ERROR_READ_TIMEOUT;
                }
                else Serial.println(httpCode);
            }
            http.end();
        }
        else Serial.println(F("Unable to begin"));
        if (!shouldRetry)
        {
            Serial.println(F("GET failed and no retry"));
            return false;
        }
        Serial.println(F("Retry"));
    }
    Serial.println(F("Tried three times but still GET failed, stop GET"));
    return false;
}

static void get_LocationId(WT_Config *cfg)
{
    URLCode urlobject;
    char api[512] = {0};
    String adm = "";
    String location = "";

    if (cfg->location_info.area.isEmpty())
    {
        urlobject.strcode = cfg->location_info.province;
        urlobject.urlencode();
        adm = urlobject.urlcode;

        urlobject.strcode = cfg->location_info.city;
        urlobject.urlencode();
        location = urlobject.urlcode;
    }
    else
    {
        urlobject.strcode = cfg->location_info.city;
        urlobject.urlencode();
        adm = urlobject.urlcode;

        urlobject.strcode = cfg->location_info.area;
        urlobject.urlencode();
        location = urlobject.urlcode;
    }
    snprintf(api, 512, QWEATHER_LOCATION_ID_API,
            cfg->location_info.country.c_str(),
            adm.c_str(),
            location.c_str(),
            cfg->key.c_str());

    JsonDocument doc;
    if (getRestfulAPI(api, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (200 != sk["code"].as<int>())
        {
            Serial.printf("Get LocationId failed, code: %s\n", sk["code"].as<String>().c_str());
            return;
        }
        cfg->location_info.LocationId = sk["location"][0]["id"].as<String>();
    }
}

static void get_ip_location(WT_Config *cfg)
{
    Location_Info IP_Localtion_Info = {"cn", "北京市", "北京", "朝阳", "101010300"};

    JsonDocument doc;
    if (getRestfulAPI(IP_INFO_API, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (HTTP_CODE_OK != sk["code"].as<int>())
            Serial.printf("Get IP info failed, code: %s, desc: %s\n", sk["code"].as<String>().c_str(), sk["desc"].as<String>().c_str());
        else
        {
            IP_Localtion_Info.country = sk["short_name"].as<String>();
            IP_Localtion_Info.country.toLowerCase();
            IP_Localtion_Info.province = sk["province"].as<String>();
            IP_Localtion_Info.city = sk["city"].as<String>();
            IP_Localtion_Info.area = sk["area"].as<String>();
            if (IP_Localtion_Info.city.length() == 0)
                IP_Localtion_Info = {"cn", "北京市", "北京", "朝阳", "101010300"};
        }
    }
    cfg->location_info = IP_Localtion_Info;
}

static void get_max_min_temp(void)
{
    char api[128] = {0};
    snprintf(api, 128, QWEATHER_3_DAY_API,
             cfg_data->location_info.LocationId.c_str(),
             cfg_data->key.c_str());

    JsonDocument doc;
    if (getRestfulAPI(api, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (HTTP_CODE_OK != sk["code"].as<int>())
        {
            Serial.printf("Get max and min temp failed, code: %s\n", sk["code"].as<String>().c_str());
            return;
        }
        run_data->wea.maxTemp = sk["daily"][0]["tempMax"].as<int>();
        run_data->wea.minTemp = sk["daily"][0]["tempMin"].as<int>();
    }
}

static void get_air_quality(void)
{
    char api[128] = {0};
    snprintf(api, 128, QWEATHER_AIR_QUALITY_NOW_API,
             cfg_data->location_info.LocationId.c_str(),
             cfg_data->key.c_str());

    JsonDocument doc;
    if (getRestfulAPI(api, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (200 != sk["code"].as<int>())
        {
            Serial.printf("Get air quality failed, code: %s\n", sk["code"].as<String>().c_str());
            return;
        }
        int aqi = sk["now"]["aqi"].as<int>();
        if (aqi <= 50) run_data->wea.airQulity = 0;
        else if (aqi <= 100) run_data->wea.airQulity = 1;
        else if (aqi <= 150) run_data->wea.airQulity = 2;
        else if (aqi <= 200) run_data->wea.airQulity = 3;
        else if (aqi <= 300) run_data->wea.airQulity = 4;
        else run_data->wea.airQulity = 5;
    }
}

static void get_weather(void)
{
    char api[128] = {0};
    String weather_icon;
    std::map<String, int>::iterator it;

    snprintf(api, 128, QWEATHER_NOW_API,
             cfg_data->location_info.LocationId.c_str(),
             cfg_data->key.c_str());

    JsonDocument doc;
    if(getRestfulAPI(api, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (HTTP_CODE_OK != sk["code"].as<int>())
        {
            Serial.printf("Get weather failed, code: %s\n", sk["code"].as<String>().c_str());
            return;
        }
        if (cfg_data->location_info.area.isEmpty())
            strcpy(run_data->wea.cityname, cfg_data->location_info.city.c_str());
        else
            strcpy(run_data->wea.cityname, cfg_data->location_info.area.c_str());

        weather_icon = sk["now"]["icon"].as<String>();
        it = weatherMap.find(weather_icon);
        run_data->wea.weather_code = (it == weatherMap.end())?61:it->second;

        run_data->wea.temperature = sk["now"]["temp"].as<int>();

        // 获取湿度
        char humidity[8] = {0};
        strncpy(humidity, sk["now"]["humidity"].as<String>().c_str(), 8);
        run_data->wea.humidity = atoi(humidity);

        strcpy(run_data->wea.windDir, sk["now"]["windDir"].as<String>().c_str());
        run_data->wea.windLevel = sk["now"]["windScale"].as<int>();

        get_max_min_temp();
        get_air_quality();
    }
}

static void get_daliyWeather(short maxT[], short minT[])
{
    char api[128] = {0};
    snprintf(api, 128, QWEATHER_DALIY_API,
             cfg_data->location_info.LocationId.c_str(),
             cfg_data->key.c_str());

    JsonDocument doc;
    if(getRestfulAPI(api, doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (HTTP_CODE_OK != sk["code"].as<int>())
        {
            Serial.printf("Get daliy weather failed, code: %s\n", sk["code"].as<String>().c_str());
            return;
        }
        for (int gDW_i = 0; gDW_i < 7; ++gDW_i)
        {
            maxT[gDW_i] = sk["daily"][gDW_i]["tempMax"].as<short>();
            minT[gDW_i] = sk["daily"][gDW_i]["tempMin"].as<short>();
        }
    }
}

static long long get_timestamp()
{
    // 使用本地的机器时钟
    run_data->preNetTimestamp = run_data->preNetTimestamp + (GET_SYS_MILLIS() - run_data->preLocalTimestamp);
    run_data->preLocalTimestamp = GET_SYS_MILLIS();
    return run_data->preNetTimestamp;
}

static long long get_timestamp(String url)
{
    JsonDocument doc;
    if(getRestfulAPI(url.c_str(), doc))
    {
        JsonObject sk = doc.as<JsonObject>();
        if (sk["ret"][0].as<String>().indexOf("SUCCESS") <= -1) goto can_not_get_timestamp;
        String time = sk["data"]["t"].as<String>();
        // 以网络时间戳为准
        run_data->preNetTimestamp = atoll(time.c_str()) + run_data->errorNetTimestamp + TIMEZERO_OFFSIZE;
        run_data->preLocalTimestamp = GET_SYS_MILLIS();
    }
    else
    {
        // 得不到网络时间戳时
        can_not_get_timestamp:
        run_data->preNetTimestamp = run_data->preNetTimestamp + (GET_SYS_MILLIS() - run_data->preLocalTimestamp);
        run_data->preLocalTimestamp = GET_SYS_MILLIS();
    }
    return run_data->preNetTimestamp;
}

static void rtc_to_date(long long timestamp, TimeStr &t)
{
    run_data->g_rtc.setTime(timestamp / 1000);
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour(true);
    t.minute = run_data->g_rtc.getMinute();
    t.second = run_data->g_rtc.getSecond();
    t.weekday = run_data->g_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    // display_time(t, LV_SCR_LOAD_ANIM_NONE);
}

static int weather_init(AppController *sys)
{
    tft->setSwapBytes(true);
    weather_gui_init();

    // 初始化运行时参数
    cfg_data = new WT_Config;
    run_data = new WeatherAppRunData;
    task_param_data = new Task_Param;
    task_param_data->task_run_data = run_data;
    task_param_data->task_cfg_data = cfg_data;

    // 获取配置信息
    read_config(cfg_data);

    memset((char *)&(run_data->wea), 0, sizeof(Weather));
    run_data->preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->errorNetTimestamp = 2;
    run_data->preLocalTimestamp = GET_SYS_MILLIS(); // 上一次的本地机器时间戳
    run_data->clock_page = 0;
    run_data->preWeatherMillis = 0;
    run_data->preTimeMillis = 0;
    // 强制更新
    run_data->coactusUpdateFlag = 0x01;
    run_data->update_type = 0x00; // 表示什么也不需要更新

    run_data->xReturned_task_task_update = xTaskCreate(
        task_update,                          /*任务函数*/
        "Task_update",                        /*带任务名称的字符串*/
        8192,                                 /*堆栈大小，单位为字节*/
        (void *)task_param_data,              /*作为任务输入传递的参数*/
        1,                                    /*任务的优先级*/
        &run_data->xHandle_task_task_update); /*任务句柄*/

    return 0;
}

static void weather_process(AppController *sys,
                            const ImuAction *act_info)
{
    static int last_clock_page = 0;
    static bool is_change = false;
    static TickType_t tmp_time = 0;
    if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(cfg_data->weatherUpdataInterval, &run_data->preWeatherMillis, false))
    {
        sys->send_to(WEATHER_APP_NAME, CTRL_NAME,
                    APP_MESSAGE_WIFI_CONN, (void *)UPDATE_NOW, NULL);
        sys->send_to(WEATHER_APP_NAME, CTRL_NAME,
                    APP_MESSAGE_WIFI_CONN, (void *)UPDATE_DAILY, NULL);
    }
    if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(cfg_data->timeUpdataInterval, &run_data->preTimeMillis, false))
    {
        // 尝试同步网络上的时钟
        sys->send_to(WEATHER_APP_NAME, CTRL_NAME,
                    APP_MESSAGE_WIFI_CONN, (void *)UPDATE_NTP, NULL);
    }
    run_data->coactusUpdateFlag = 0x00; // 取消强制更新标志
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
        run_data->clock_page = 1;
    }
    else if (TURN_LEFT == act_info->active)
    {
        run_data->clock_page = 0;
    }
    if (run_data->clock_page == 0)
    {
        TimeStr t;
        rtc_to_date(get_timestamp(), t);
        if (run_data->clock_page != last_clock_page)
        {
            is_change = true;
            tmp_time = GET_SYS_MILLIS();
            display_weather(run_data->wea, t, LV_SCR_LOAD_ANIM_MOVE_LEFT);
        }
        else if (is_change && (GET_SYS_MILLIS() - tmp_time > 800 || GET_SYS_MILLIS() - tmp_time < 0))
            is_change = false;
        if (!is_change)
        {
            display_weather(run_data->wea, t, LV_SCR_LOAD_ANIM_NONE);
            display_space();
            delay(30);
        }
    }
    else if (run_data->clock_page == 1)
    {
        if (run_data->clock_page != last_clock_page)
        {
            is_change = true;
            tmp_time = GET_SYS_MILLIS();
            display_curve(run_data->wea.daily_max, run_data->wea.daily_min, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
        }
        else if (is_change && (GET_SYS_MILLIS() - tmp_time > 800 || GET_SYS_MILLIS() - tmp_time < 0))
            is_change = false;
        if (!is_change)
        {
            display_curve(run_data->wea.daily_max, run_data->wea.daily_min, LV_SCR_LOAD_ANIM_NONE);
            delay(30);
        }
    }
    last_clock_page = run_data->clock_page;
}

static void weather_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int weather_exit_callback(void *param)
{
    weather_gui_del();

    // 查杀异步任务
    if (run_data->xReturned_task_task_update == pdPASS)
    {
        vTaskDelete(run_data->xHandle_task_task_update);
    }

    // 释放运行数据
    delete cfg_data;
    delete run_data;
    delete task_param_data;

    return 0;
}

static void task_update(void *parameter)
{
    Task_Param *param_in_task = (Task_Param *)parameter;
    WeatherAppRunData *run_data_in_task = param_in_task->task_run_data;
    WT_Config *cfg_data_in_task = param_in_task->task_cfg_data;

    // 数据更新任务
    while (1)
    {
        if (run_data_in_task->update_type & UPDATE_TIME)
        {
            Serial.println(F("time update."));
            get_timestamp(TIME_API);
            if (run_data_in_task->clock_page == 0)
            {
                TimeStr t;
                rtc_to_date(get_timestamp(), t);
                display_weather(run_data_in_task->wea, t, LV_SCR_LOAD_ANIM_NONE);
            }
            run_data_in_task->update_type &= (~UPDATE_TIME);
        }
        if(run_data_in_task->update_type & UPDATE_IP_LOCATION)
        {
            Serial.println(F("location update."));
            get_ip_location(cfg_data_in_task);
            get_LocationId(cfg_data_in_task);
            // Serial.printf("country: %s, province: %s, city: %s, area: %s, LocationID: %s\n",
            //                 cfg_data_in_task->location_info.country.c_str(), cfg_data_in_task->location_info.province.c_str(),
            //                 cfg_data_in_task->location_info.city.c_str(), cfg_data_in_task->location_info.area.c_str(),
            //                 cfg_data_in_task->location_info.LocationId.c_str());
            write_config(cfg_data_in_task);
            if (run_data_in_task->clock_page == 0)
            {
                TimeStr t;
                rtc_to_date(get_timestamp(), t);
                display_weather(run_data_in_task->wea, t, LV_SCR_LOAD_ANIM_NONE);
            }
            run_data_in_task->update_type &= (~UPDATE_IP_LOCATION);
        }
        if (run_data_in_task->update_type & UPDATE_WEATHER)
        {
            Serial.println(F("weather update."));
            get_weather();
            if (run_data_in_task->clock_page == 0)
            {
                TimeStr t;
                rtc_to_date(get_timestamp(), t);
                display_weather(run_data_in_task->wea, t, LV_SCR_LOAD_ANIM_NONE);
            }
            run_data_in_task->update_type &= (~UPDATE_WEATHER);
        }
        if (run_data_in_task->update_type & UPDATE_DALIY_WEATHER)
        {
            Serial.println(F("daliy update."));
            get_daliyWeather(run_data_in_task->wea.daily_max, run_data_in_task->wea.daily_min);
            if (run_data_in_task->clock_page == 1)
            {
                display_curve(run_data_in_task->wea.daily_max, run_data_in_task->wea.daily_min, LV_SCR_LOAD_ANIM_NONE);
            }
            run_data_in_task->update_type &= (~UPDATE_DALIY_WEATHER);
        }
        vTaskDelay(100);
    }
}

static void weather_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    while (WL_CONNECTED != WiFi.status())
    {
        Serial.println(F("WiFi not connected, wait for 100ms"));
        delay(100);
    }
    static bool first_get = true;
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // Serial.println(F("----->weather_event_notification"));
        if (first_get && cfg_data->auto_get_location)
        {
            run_data->update_type |= UPDATE_IP_LOCATION;
        }
        first_get = false;
        int event_id = (int)message;
        switch (event_id)
        {
        case UPDATE_NOW:
        {
            run_data->update_type |= UPDATE_WEATHER;
        };
        break;
        case UPDATE_NTP:
        {
            run_data->update_type |= UPDATE_TIME;
        };
        break;
        case UPDATE_DAILY:
        {
            run_data->update_type |= UPDATE_DALIY_WEATHER;
        };
        break;
        default:
            break;
        }
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "key"))
        {
            snprintf((char *)ext_info, 64, "%s", cfg_data->key.c_str());
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data->weatherUpdataInterval);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data->timeUpdataInterval);
        }
        else if (!strcmp(param_key, "auto_get_location"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data->auto_get_location);
        }
        else if (!strcmp(param_key, "country"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data->location_info.country.c_str());
        }
        else if (!strcmp(param_key, "province"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data->location_info.province.c_str());
        }
        else if (!strcmp(param_key, "city"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data->location_info.city.c_str());
        }
        else if (!strcmp(param_key, "area"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data->location_info.area.c_str());
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "key"))
        {
            cfg_data->key = param_val;
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            cfg_data->weatherUpdataInterval = atol(param_val);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            cfg_data->timeUpdataInterval = atol(param_val);
        }
        else if (!strcmp(param_key, "auto_get_location"))
        {
            cfg_data->auto_get_location = atoi(param_val);
        }
        else if (!strcmp(param_key, "country"))
        {
            cfg_data->location_info.country = param_val;
            cfg_data->location_info.country.toLowerCase();
        }
        else if (!strcmp(param_key, "province"))
        {
            cfg_data->location_info.province = param_val;
        }
        else if (!strcmp(param_key, "city"))
        {
            cfg_data->location_info.city = param_val;
        }
        else if (!strcmp(param_key, "area"))
        {
            cfg_data->location_info.area = param_val;
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ weather_app = {WEATHER_APP_NAME, &app_weather, "",
                       weather_init, weather_process, weather_background_task,
                       weather_exit_callback, weather_message_handle};
