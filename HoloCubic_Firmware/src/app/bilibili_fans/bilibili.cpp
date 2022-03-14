#include "bilibili.h"
#include "bilibili_gui.h"
#include "sys/app_controller.h"
#include "../../common.h"

#define FANS_API "https://api.bilibili.com/x/relation/stat?vmid="
#define OTHER_API "https://api.bilibili.com/x/space/upstat?mid="

// 天气的持久化配置
#define B_CONFIG_PATH "/bilibili.cfg"
struct B_Config
{
    String bili_uid; // bilibili的uid
};

static void read_config(B_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    String info = g_flashCfg.readFile(B_CONFIG_PATH);
    // 解析数据
    cfg->bili_uid = info; //
}

static void write_config(const B_Config *cfg)
{
    char tmp[25];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->bili_uid + "\n";
    g_flashCfg.writeFile(B_CONFIG_PATH, w_data.c_str());
}

struct BilibiliAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_interval;
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
    HTTPClient *httpClient = new HTTPClient();
    httpClient->setTimeout(1000);
    bool status = httpClient->begin(url);
    if (status == false)
    {
        result.httpCode = -1;
        return result;
    }
    int httpCode = httpClient->GET();
    String httpResponse = httpClient->getString();
    httpClient->end();
    result.httpCode = httpCode;
    result.httpResponse = httpResponse;
    return result;
}

static int bilibili_init(void)
{
    bilibili_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    run_data = (BilibiliAppRunData *)malloc(sizeof(BilibiliAppRunData));
    run_data->fans_num = 0;
    run_data->follow_num = 0;
    run_data->refresh_status = 0;
    run_data->refresh_interval = 900000;
    run_data->refresh_time_millis = millis() - run_data->refresh_interval;
}

static void bilibili_process(AppController *sys,
                             const Imu_Action *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    char fans_num[20] = {0};
    char follow_num[20] = {0};
    if (run_data->fans_num >= 10000)
    {
        // 粉丝过万的
        snprintf(fans_num, 20, "%3.1fw", run_data->fans_num * 1.0 / 10000);
    }
    else
    {
        snprintf(fans_num, 20, "%d", run_data->fans_num);
    }

    if (run_data->follow_num >= 10000)
    {
        // 关注过万的
        snprintf(follow_num, 20, "%3.1fw", run_data->follow_num * 1.0 / 10000);
    }
    else
    {
        snprintf(follow_num, 20, "%d", run_data->follow_num);
    }

    if (0 == run_data->refresh_status)
    {
        display_bilibili("bilibili", anim_type, fans_num, follow_num);
        // 以下减少网络请求的压力
        if (doDelayMillisTime(run_data->refresh_interval, &run_data->refresh_time_millis, false))
        {
            sys->send_to(BILI_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_CONN, NULL, NULL);
        }
    }
    else
    {
        display_bilibili("bilibili", anim_type, fans_num, follow_num);
    }

    delay(300);
}

int bilibili_exit_callback(void *param)
{
    bilibili_gui_del();
    free(run_data);
    run_data = NULL;
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
        Serial.print(millis());
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
    }
    break;
    default:
        break;
    }
}

APP_OBJ bilibili_app = {BILI_APP_NAME, &app_bilibili, "", bilibili_init,
                        bilibili_process, bilibili_exit_callback,
                        bilibili_message_handle};
