#include <ArduinoJson.h>

#include "bilibili.h"
#include "bilibili_gui.h"
#include "sys/app_controller.h"
#include "../../common.h"

#define BILIBILI_API "https://api.bilibili.com/x/relation/stat?vmid="
// #define OTHER_API "https://api.bilibili.com/x/space/upstat?mid="

// Bilibili的持久化配置
#define B_CONFIG_PATH "/bilibili.cfg"

struct BilibiliConfig
{
    String bili_uid;              // bilibili的uid
    unsigned long updataInterval; // 更新的时间间隔(s)
};

static void write_config(const BilibiliConfig *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    w_data = w_data + cfg->bili_uid + "\n";
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->updataInterval);
    w_data += tmp;
    g_flashCfg.writeFile(B_CONFIG_PATH, w_data.c_str());
}

static void read_config(BilibiliConfig *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(B_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->bili_uid = "344470052";  // B站的用户ID
        cfg->updataInterval = 900000; // 更新的时间间隔900000(900s)
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->bili_uid = param[0];
        cfg->updataInterval = atol(param[1]);
    }
}

struct BilibiliRunData
{
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

struct BilibiliUserInfo
{
    unsigned int follower = 0;
    unsigned int following = 0;
};

struct HttpResponse
{
    int httpCode = 0;
    String httpPayload = "";
};

static BilibiliConfig cfg_data;
static BilibiliRunData *run_data = NULL;
struct BilibiliUserInfo *userInfo = NULL;

static HttpResponse http_request(String uid = "344470052")
{
    // HTTP请求结果
    HttpResponse result;
    // API接口地址+用户UID
    String url = BILIBILI_API + uid;
    // 创建HTTP客户端
    HTTPClient *httpClient = new HTTPClient();
    // 设置超时时间
    httpClient->setTimeout(1000);
    // 发起HTTP请求
    bool status = httpClient->begin(url);
    // 请求失败
    if (status == false)
    {
        result.httpCode = -1;
        result.httpPayload = "";
        return result;
    }
    // 获取返回结果
    result.httpCode = httpClient->GET();          //状态码
    result.httpPayload = httpClient->getString(); //请求结果
    // 关闭HTTP客户端
    httpClient->end();
    return result;
}

static int bilibili_init(AppController *sys)
{
    // 界面初始化
    bilibili_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    run_data = (BilibiliRunData *)malloc(sizeof(BilibiliRunData));
    run_data->refresh_status = 0;
    run_data->refresh_time_millis = millis() - cfg_data.updataInterval;
    // 初始化用户信息
    userInfo = (BilibiliUserInfo *)malloc(sizeof(BilibiliUserInfo));
    userInfo->follower = 0;
    userInfo->following = 0;
}

static void bilibili_process(AppController *sys,
                             const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;
    // 退出APP
    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }
    // 临时缓存
    char follower[20] = {0};
    char following[20] = {0};
    // 粉丝数过万
    if (userInfo->follower >= 10000)
    {
        snprintf(follower, 20, "%3.1fw", userInfo->follower * 1.0 / 10000);
    }
    else
    {
        snprintf(follower, 20, "%d", userInfo->follower);
    }
    // 关注数过万
    if (userInfo->following >= 10000)
    {
        snprintf(following, 20, "%3.1fw", userInfo->following * 1.0 / 10000);
    }
    else
    {
        snprintf(following, 20, "%d", userInfo->following);
    }

    if (0 == run_data->refresh_status)
    {
        display_bilibili("bilibili", anim_type, follower, following);
        // 以下减少网络请求的压力
        if (doDelayMillisTime(cfg_data.updataInterval, &run_data->refresh_time_millis, false))
        {
            sys->send_to(BILI_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_CONN, NULL, NULL);
        }
    }
    else
    {
        display_bilibili("bilibili", anim_type, follower, following);
    }

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
    if (NULL != userInfo)
    {
        free(userInfo);
        userInfo = NULL;
    }
}

static void update_fans_num()
{
    // 发起HTTP请求获取粉丝信息
    HttpResponse response = http_request(cfg_data.bili_uid);
    if (-1 == response.httpCode)
    {
        Serial.println("[HTTP] HTTP request failed.");
        return;
    }
    if (response.httpCode <= 0)
    {
        Serial.println("[HTTP] HTTP request ERROR");
        return;
    }
    if (response.httpCode == HTTP_CODE_OK || response.httpCode == HTTP_CODE_MOVED_PERMANENTLY)
    {
        //获取负载
        String payload = response.httpPayload;
        // JSON文档容量
        const size_t jsonCapacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 70;
        //创建JSON文档对象
        DynamicJsonDocument jsonDocument(jsonCapacity);
        //解析JSON数据
        deserializeJson(jsonDocument, payload);
        //处理JSON数据
        int code = jsonDocument["code"];
        JsonObject data = jsonDocument["data"];
        // {
            int black = jsonDocument["black"];
            int follower = data["follower"];
            int following = data["following"];
            String mid = data["mid"];
            int whisper = data["whisper"];
            // 更新数据
            userInfo->follower = follower;
            userInfo->following = following;
        // }
        String message = jsonDocument["message"];
        int ttl = jsonDocument["ttl"];
        Serial.println(code);
        Serial.println(black);
        Serial.println(follower);
        Serial.println(following);
        Serial.println(mid);
        Serial.println(whisper);
        Serial.println(message);
        Serial.println(ttl);
        Serial.print("[HTTP] HTTP Request success.");
        Serial.println(payload);
        // 更新刷新状态
        run_data->refresh_status = 1;
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
        else if (!strcmp(param_key, "updataInterval"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.updataInterval);
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
