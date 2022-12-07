#include "weather_old.h"
#include "weather_old_gui.h"
#include "ESP32Time.h"
#include "sys/app_controller.h"
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

// 天气的持久化配置
#define WEATHER_OLD_CONFIG_PATH "/weather_old.cfg"
struct WT_Config
{
    String cityname;                     // 显示的城市
    String language;                     // 天气查询的地址编码
    String weather_key;                  // 知心天气api_key（秘钥）
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔(s)
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
};

static void write_config(const WT_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->cityname + "\n";
    w_data = w_data + cfg->language + "\n";
    w_data = w_data + cfg->weather_key + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->weatherUpdataInterval);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->timeUpdataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(WEATHER_OLD_CONFIG_PATH, w_data.c_str());
}

static void read_config(WT_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(WEATHER_OLD_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->cityname = "Beijing";
        cfg->language = "zh-Hans";
        cfg->weatherUpdataInterval = 900000; // 天气更新的时间间隔900000(900s)
        cfg->timeUpdataInterval = 900000;    // 日期时钟更新的时间间隔900000(900s)
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[5] = {0};
        analyseParam(info, 5, param);
        cfg->cityname = param[0];
        cfg->language = param[1];
        cfg->weather_key = param[2];
        cfg->weatherUpdataInterval = atol(param[3]);
        cfg->timeUpdataInterval = atol(param[4]);
    }
}

struct WeatherAppRunData
{
    unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long m_preNetTimestamp;    // 上一次的网络时间戳
    long long m_errorNetTimestamp;  // 网络到显示过程中的时间误差
    long long m_preLocalTimestamp;  // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;                 // 时钟桌面的播放记录

    ESP32Time g_rtc; // 用于时间解码
    Weather weather; // 保存天气状况
};

static WT_Config cfg_data;
static WeatherAppRunData *run_data = NULL;

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
            int code_index = (payload.indexOf("code")) + 7;         // 获取code位置
            int temp_index = (payload.indexOf("temperature")) + 14; // 获取temperature位置
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
    run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (GET_SYS_MILLIS() - run_data->m_preLocalTimestamp);
    run_data->m_preLocalTimestamp = GET_SYS_MILLIS();

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
            run_data->m_preLocalTimestamp = GET_SYS_MILLIS();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        run_data->m_preNetTimestamp = run_data->m_preNetTimestamp + (GET_SYS_MILLIS() - run_data->m_preLocalTimestamp);
        run_data->m_preLocalTimestamp = GET_SYS_MILLIS();
    }
    http.end();

    return run_data->m_preNetTimestamp;
}

static void UpdateWeather(Weather *weather, lv_scr_load_anim_t anim_type)
{
    char temperature[10] = {0};
    sprintf(temperature, "%d", weather->temperature);
    display_weather_old(cfg_data.cityname.c_str(), temperature, weather->weather_code, anim_type);
}

static void UpdateTime_RTC(long long timestamp, lv_scr_load_anim_t anim_type)
{
    run_data->g_rtc.setTime(timestamp / 1000);
    String date = run_data->g_rtc.getDate(String("%Y-%m-%d"));
    String time = run_data->g_rtc.getTime(String("%H:%M:%S"));
    display_time_old(date.c_str(), time.c_str(), anim_type);
}

static int weather_init(AppController *sys)
{
    weather_old_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    run_data->m_preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化围殴2020-01-01 00:00:00
    run_data->m_errorNetTimestamp = 2;
    run_data->m_preLocalTimestamp = 0; // 上一次的本地机器时间戳
    run_data->clock_page = 0;          // 时钟桌面的播放记录
    // 变相强制更新
    run_data->preWeatherMillis = GET_SYS_MILLIS() - cfg_data.weatherUpdataInterval;
    run_data->preTimeMillis = GET_SYS_MILLIS() - cfg_data.timeUpdataInterval;
    run_data->coactusUpdateFlag = 0x01;

    run_data->weather = {0, 0};
    return 0;
}

static void weather_process(AppController *sys,
                            const ImuAction *act_info)
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
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(cfg_data.weatherUpdataInterval, &run_data->preWeatherMillis, false))
        {
            sys->send_to(WEATHER_OLD_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_CONN, (void *)run_data->clock_page, NULL);
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
        if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(cfg_data.timeUpdataInterval, &run_data->preTimeMillis, false))
        {
            // 尝试同步网络上的时钟
            sys->send_to(WEATHER_OLD_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_CONN, (void *)run_data->clock_page, NULL);
            run_data->coactusUpdateFlag = 0x00;
        }
    }
    else if (2 == run_data->clock_page) // NULL后期可以是具体数据
    {
        display_hardware_old(NULL, anim_type);
    }

    delay(300);
}

static void weather_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int weather_exit_callback(void *param)
{
    weather_old_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void weather_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(GET_SYS_MILLIS());
        Serial.print(F("----->weather_event_notification\n"));
        int event_id = (int)message;
        if (0 == run_data->clock_page && run_data->clock_page == event_id)
        {
            // 如果要改城市这里也需要修改
            char api[128] = "";
            snprintf(api, 128, ZHIXIN_WEATHER_API, cfg_data.weather_key.c_str(),
                     cfg_data.cityname.c_str(), cfg_data.language.c_str());
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
    case APP_MESSAGE_WIFI_AP:
    {
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "cityname"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.cityname.c_str());
        }
        else if (!strcmp(param_key, "language"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.language.c_str());
        }
        else if (!strcmp(param_key, "weather_key"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.weather_key.c_str());
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.weatherUpdataInterval);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.timeUpdataInterval);
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
        if (!strcmp(param_key, "cityname"))
        {
            cfg_data.cityname = param_val;
        }
        else if (!strcmp(param_key, "language"))
        {
            cfg_data.language = param_val;
        }
        else if (!strcmp(param_key, "weather_key"))
        {
            cfg_data.weather_key = param_val;
        }
        else if (!strcmp(param_key, "weatherUpdataInterval"))
        {
            cfg_data.weatherUpdataInterval = atol(param_val);
        }
        else if (!strcmp(param_key, "timeUpdataInterval"))
        {
            cfg_data.timeUpdataInterval = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ weather_old_app = {WEATHER_OLD_APP_NAME, &app_weather_old, "",
                           weather_init, weather_process, weather_background_task,
                           weather_exit_callback, weather_message_handle};
