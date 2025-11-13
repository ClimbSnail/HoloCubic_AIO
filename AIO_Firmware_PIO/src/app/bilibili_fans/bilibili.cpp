#include "bilibili.h"
#include "bilibili_gui.h"
#include "sys/app_controller.h"
#include "../../common.h"

#define FANS_API "https://api.bilibili.com/x/relation/stat?vmid="
#define OTHER_API "https://api.bilibili.com/x/space/upstat?mid="

// Bilibili persistent configuration
#define B_CONFIG_PATH "/bilibili.cfg"
struct B_Config
{
    String bili_uid;              // Bilibili UID
    unsigned long updataInterval; // Update time interval (s)
};

static void write_config(const B_Config *cfg)
{
    char tmp[16];
    // Save configuration data to file (persistence)
    String w_data;
    w_data = w_data + cfg->bili_uid + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->updataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(B_CONFIG_PATH, w_data.c_str());
}

static void read_config(B_Config *cfg)
{
    // If you need to persist configuration files, you can call this function to store the data in flash
    // Configuration file names should preferably start with the APP name and end with ".cfg" to avoid confusion when multiple APPs read them
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(B_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // Default values
        cfg->bili_uid = "344470052";  // Bilibili user ID
        cfg->updataInterval = 900000; // Update time interval 900000(900s)
        write_config(cfg);
    }
    else
    {
        // Parse data
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->bili_uid = param[0];
        cfg->updataInterval = atol(param[1]);
    }
}

struct BilibiliAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

struct MyHttpResult
{
    int httpCode = 0;
    String httpResponse = "";
};

static B_Config cfg_data;
static BilibiliAppRunData *run_data = NULL;

static MyHttpResult http_request(String uid = "344470052")
{
    // String url = "http://www.dtmb.top/api/fans/index?id=" + uid;
    MyHttpResult result;
    String url = FANS_API + uid;
    HTTPClient httpClient;
    httpClient.setTimeout(1000);
    bool status = httpClient.begin(url);
    if (status == false)
    {
        result.httpCode = -1;
        return result;
    }
    int httpCode = httpClient.GET();
    String httpResponse = httpClient.getString();
    httpClient.end();
    result.httpCode = httpCode;
    result.httpResponse = httpResponse;
    return result;
}

static int bilibili_init(AppController *sys)
{
    bilibili_gui_init();
    // Get configuration information
    read_config(&cfg_data);
    // Initialize runtime parameters
    run_data = (BilibiliAppRunData *)malloc(sizeof(BilibiliAppRunData));
    run_data->fans_num = 0;
    run_data->follow_num = 0;
    run_data->refresh_status = 0;
    run_data->refresh_time_millis = GET_SYS_MILLIS() - cfg_data.updataInterval;
    return 0;
}

static void bilibili_process(AppController *sys,
                             const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // Exit APP
        return;
    }

    char fans_num[20] = {0};
    char follow_num[20] = {0};
    if (run_data->fans_num >= 10000)
    {
        // Fans over 10,000
        snprintf(fans_num, 20, "%3.1fw", run_data->fans_num * 1.0 / 10000);
    }
    else
    {
        snprintf(fans_num, 20, "%d", run_data->fans_num);
    }

    if (run_data->follow_num >= 10000)
    {
        // Following over 10,000
        snprintf(follow_num, 20, "%3.1fw", run_data->follow_num * 1.0 / 10000);
    }
    else
    {
        snprintf(follow_num, 20, "%d", run_data->follow_num);
    }

    if (0 == run_data->refresh_status)
    {
        // Reduce network request pressure
        if (doDelayMillisTime(cfg_data.updataInterval, &run_data->refresh_time_millis, false))
        {
            sys->send_to(BILI_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_CONN, NULL, NULL);
        }
    }

    display_bilibili("bilibili", anim_type, fans_num, follow_num);

    delay(300);
}

static void bilibili_background_task(AppController *sys,
                                     const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int bilibili_exit_callback(void *param)
{
    bilibili_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void update_fans_num()
{
    MyHttpResult result = http_request(cfg_data.bili_uid);
    if (-1 == result.httpCode)
    {
        Serial.println("[HTTP] Http request failed.");
        return;
    }
    if (result.httpCode > 0)
    {
        if (result.httpCode == HTTP_CODE_OK || result.httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = result.httpResponse;
            Serial.println("[HTTP] OK");
            Serial.println(payload);
            int startIndex_1 = payload.indexOf("follower") + 10;
            int endIndex_1 = payload.indexOf('}', startIndex_1);
            int startIndex_2 = payload.indexOf("following") + 11;
            int endIndex_2 = payload.indexOf(',', startIndex_2);
            String res = payload.substring(startIndex_1, endIndex_1);
            run_data->fans_num = payload.substring(startIndex_1, endIndex_1).toInt();
            run_data->follow_num = payload.substring(startIndex_2, endIndex_2).toInt();
            run_data->refresh_status = 1;
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
}

static void bilibili_message_handle(const char *from, const char *to,
                                    APP_MESSAGE_TYPE type, void *message,
                                    void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(GET_SYS_MILLIS());
        Serial.println("[SYS] bilibili_event_notification");
        if (0 == run_data->refresh_status)
        {
            update_fans_num();
        }
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "bili_uid"))
        {
            snprintf((char *)ext_info, 32, "%s", cfg_data.bili_uid.c_str());
        }
        else if (!strcmp(param_key, "updataInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.updataInterval);
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
        if (!strcmp(param_key, "bili_uid"))
        {
            cfg_data.bili_uid = param_val;
        }
        else if (!strcmp(param_key, "updataInterval"))
        {
            cfg_data.updataInterval = atol(param_val);
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

APP_OBJ bilibili_app = {BILI_APP_NAME, &app_bilibili, "", bilibili_init,
                        bilibili_process, bilibili_background_task,
                        bilibili_exit_callback, bilibili_message_handle};
