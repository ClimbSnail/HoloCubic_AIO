#include "anniversary.h"
#include "anniversary_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "sys/time.h"

#define ANNIVERSARY_APP_NAME "Anniversary"
#define MAX_ANNIVERSARY_CNT 2
#define TIME_API "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp"

bool tmfromString(const char *date_str, struct tm *date);

// 纪念日的持久化配置
#define ANNIVERSARY_CONFIG_PATH "/anniversary.cfg"
struct AN_Config
{
    unsigned long anniversary_cnt;              // 事件个数
    String event_name[MAX_ANNIVERSARY_CNT];     // 事件名称
    struct tm target_date[MAX_ANNIVERSARY_CNT]; // 目标日
    struct tm current_date;
};

static long long get_timestamp(String url);

static void write_config(AN_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->anniversary_cnt);
    w_data += tmp;
    for (int i = 0; i < MAX_ANNIVERSARY_CNT; ++i)
    {
        w_data = w_data + cfg->event_name[i] + "\n";
        memset(tmp, 0, 16);
        snprintf(tmp, 16, "%d.%d.%d\n", cfg->target_date[i].tm_year, cfg->target_date[i].tm_mon, cfg->target_date[i].tm_mday);
        w_data += tmp;
    }
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%d.%d.%d\n", cfg->current_date.tm_year, cfg->current_date.tm_mon, cfg->current_date.tm_mday);
    w_data += tmp;
    g_flashCfg.writeFile(ANNIVERSARY_CONFIG_PATH, w_data.c_str());
}

static void read_config(AN_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(ANNIVERSARY_CONFIG_PATH, (uint8_t *)info);
    Serial.printf("size %d\n", size);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->anniversary_cnt = 2;
        cfg->event_name[0] = "生日还有";
        cfg->target_date[0].tm_year = 0; // 设置为零则每年重复
        cfg->target_date[0].tm_mon = 1;
        cfg->target_date[0].tm_mday = 1;
        cfg->event_name[1] = "毕业还有";
        cfg->target_date[1].tm_year = 2025;
        cfg->target_date[1].tm_mon = 7;
        cfg->target_date[1].tm_mday = 4;
        write_config(cfg);
        Serial.printf("Write config successful\n");
    }
    else
    {
        // 解析数据
        char *param[MAX_ANNIVERSARY_CNT * 2 + 2] = {0};
        analyseParam(info, MAX_ANNIVERSARY_CNT * 2 + 2, param);
        cfg->anniversary_cnt = atol(param[0]);
        for (int i = 0; i < MAX_ANNIVERSARY_CNT; ++i)
        {
            cfg->event_name[i] = param[2 * i + 1];
            tmfromString(param[2 * i + 2], &(cfg->target_date[i]));
        }
        tmfromString(param[MAX_ANNIVERSARY_CNT * 2 + 1], &(cfg->current_date));
    }
}

// 动态数据，APP的生命周期结束也需要释放它
struct AnniversaryAppRunData
{
    int cur_anniversary; // 当前显示第几个纪念日
    int anniversary_day_count;
    unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
};

static AN_Config cfg_data;
static AnniversaryAppRunData *run_data = NULL;

bool tmfromString(const char *date_str, struct tm *date)
{
    // TODO: add support for "a", "a.b", "a.b.c" formats

    uint16_t acc = 0; // Accumulator
    uint8_t dots = 0;

    while (*date_str)
    {
        char c = *date_str++;
        if (c >= '0' && c <= '9')
        {
            acc = acc * 10 + (c - '0');
        }
        else if (c == '.')
        {
            if (dots == 0)
            {
                date->tm_year = acc;
            }
            else if (dots == 1)
            {
                date->tm_mon = acc;
            }
            if (dots == 2)
            {
                // Too much dots (there must be 3 dots)
                return false;
            }
            acc = 0;
            ++dots;
        }
        else
        {
            // Invalid char
            return false;
        }
    }

    if (dots != 2)
    {
        // Too few dots (there must be 3 dots)
        return false;
    }
    date->tm_mday = acc;
    return true;
}

static int dateDiff(struct tm *date1, struct tm *date2)
{
    int y1, m1, d1;
    int y2, m2, d2;
    m1 = (date1->tm_mon + 9) % 12;
    y1 = (date1->tm_year - m1 / 10);
    d1 = 365 * y1 + y1 / 4 - y1 / 100 + y1 / 400 + (m1 * 306 + 5) / 10 + (date1->tm_mday - 1);

    m2 = (date2->tm_mon + 9) % 12;
    if (date2->tm_year == 0)
    {
        if (date2->tm_mon < date1->tm_mon || (date2->tm_mon == date1->tm_mon && date2->tm_mon < date1->tm_mon))
        {
            y2 = date1->tm_year + 1 - m2 / 10;
        }
        else
        {
            y2 = date1->tm_year - m2 / 10;
        }
    }
    else
    {
        y2 = date2->tm_year - m2 / 10;
    }
    d2 = 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 + (m2 * 306 + 5) / 10 + (date2->tm_mday - 1);
    return (d2 - d1);
}

static void get_date_diff()
{
    time_t timep = run_data->preNetTimestamp / 1000;
    struct tm *p_tm;
    // time(&timep);
    p_tm = localtime(&timep);

    cfg_data.current_date.tm_year = p_tm->tm_year + 1900;
    cfg_data.current_date.tm_mon = p_tm->tm_mon + 1;
    cfg_data.current_date.tm_mday = p_tm->tm_mday;

    // Serial.printf("current_date %d %d %d\n", cfg_data.current_date.tm_year, cfg_data.current_date.tm_mon, cfg_data.current_date.tm_mday);

    run_data->anniversary_day_count = dateDiff(&(cfg_data.current_date), &(cfg_data.target_date[run_data->cur_anniversary]));
}

static void date_update()
{
    get_date_diff();
    anniversary_gui_display_date(&(cfg_data.target_date[run_data->cur_anniversary]), run_data->anniversary_day_count, cfg_data.event_name[run_data->cur_anniversary].c_str());
}

static long long get_timestamp(String url)
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

static int anniversary_init(AppController *sys)
{
    anniversary_gui_init();
    // 获取配置参数
    read_config(&cfg_data);
    // 初始化运行时的参数
    run_data = (AnniversaryAppRunData *)calloc(1, sizeof(AnniversaryAppRunData));
    run_data->cur_anniversary = 0;
    run_data->preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->errorNetTimestamp = 2;
    run_data->preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    run_data->preWeatherMillis = 0;
    run_data->preTimeMillis = 0;
    run_data->coactusUpdateFlag = 0x01;
    Serial.printf("anniversary init successful\n");
}

static void anniversary_process(AppController *sys,
                                const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    else if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
        run_data->cur_anniversary = (run_data->cur_anniversary + 1) % MAX_ANNIVERSARY_CNT;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
        run_data->cur_anniversary = (run_data->cur_anniversary + MAX_ANNIVERSARY_CNT - 1) % MAX_ANNIVERSARY_CNT;
    }
    if (0x01 == run_data->coactusUpdateFlag || doDelayMillisTime(900000, &run_data->preTimeMillis, false))
    {
        // 启动时先用持久化配置中的日期
        run_data->anniversary_day_count = dateDiff(&(cfg_data.current_date), &(cfg_data.target_date[run_data->cur_anniversary]));
        // 尝试同步网络上的时钟
        sys->send_to(ANNIVERSARY_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        run_data->coactusUpdateFlag = 0x00;
        write_config(&cfg_data);
    }
    else
    {
        get_date_diff();
    }
    // tm *cur_target = &(cfg_data.target_date[run_data->cur_anniversary]);
    // Serial.printf("%d %d %d %d", cur_target->tm_year,  cur_target->tm_mon,  cur_target->tm_mday,  cur_target->tm_wday);
    // Serial.println(F(""));
    // Serial.printf("%d %d %d %d", cfg_data.target_date[run_data->cur_anniversary].tm_year,  cfg_data.target_date[run_data->cur_anniversary].tm_mon,  cfg_data.target_date[run_data->cur_anniversary].tm_mday,  cfg_data.target_date[run_data->cur_anniversary].tm_wday);
    // Serial.println(F(""));
    // Serial.println(F(cfg_data.event_name[run_data->cur_anniversary].c_str()));
    display_anniversary("anniversary", anim_type, &(cfg_data.target_date[run_data->cur_anniversary]), run_data->anniversary_day_count, cfg_data.event_name[run_data->cur_anniversary].c_str());
    anniversary_gui_display_date(&(cfg_data.target_date[run_data->cur_anniversary]), run_data->anniversary_day_count, cfg_data.event_name[run_data->cur_anniversary].c_str());
    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 anniversary_message_handle 函数
    // sys->send_to(ANNIVERSARY_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 程序需要时可以适当加延时
    delay(300);
}

static void anniversary_background_task(AppController *sys,
                                        const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int anniversary_exit_callback(void *param)
{
    // 释放资源
    anniversary_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
}

static void anniversary_message_handle(const char *from, const char *to,
                                       APP_MESSAGE_TYPE type, void *message,
                                       void *ext_info)
{
    // 目前主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
        Serial.print(F("ntp update.\n"));

        long long timestamp = get_timestamp(TIME_API); // nowapi时间API
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
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
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "event_name0"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.event_name[0].c_str());
        }
        else if (!strcmp(param_key, "target_date0"))
        {
            struct tm *tmp_tm = &(cfg_data.target_date[0]);
            snprintf((char *)ext_info, 32, "%d.%d.%d", tmp_tm->tm_year, tmp_tm->tm_mon, tmp_tm->tm_mday);
        }
        else if (!strcmp(param_key, "event_name1"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.event_name[1].c_str());
        }
        else if (!strcmp(param_key, "target_date1"))
        {
            struct tm *tmp_tm = &(cfg_data.target_date[1]);
            snprintf((char *)ext_info, 32, "%d.%d.%d", tmp_tm->tm_year, tmp_tm->tm_mon, tmp_tm->tm_mday);
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "event_name0"))
        {
            cfg_data.event_name[0] = param_val;
        }
        else if (!strcmp(param_key, "target_date0"))
        {
            tmfromString(param_val, &(cfg_data.target_date[0]));
        }
        else if (!strcmp(param_key, "event_name1"))
        {
            cfg_data.event_name[1] = param_val;
        }
        else if (!strcmp(param_key, "target_date1"))
        {
            tmfromString(param_val, &(cfg_data.target_date[1]));
        }
    }
    break;
    default:
        break;
    }
}

APP_OBJ anniversary_app = {ANNIVERSARY_APP_NAME, &app_anniversary, "Author Hu Qianjiang\nVersion 0.0.1\n",
                           anniversary_init, anniversary_process, anniversary_background_task,
                           anniversary_exit_callback, anniversary_message_handle};
