#include "heartbeat.h"
#include "heartbeat_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "network.h"
#include <PubSubClient.h>

#define HEARTBEAT_APP_NAME "Heartbeat"

#define DEFALUT_MQTT_ADDR "climbsnail.cn"
#define DEFALUT_MQTT_PORT 1883
#define DEFALUT_MQTT_USERNAME "HoloCubic"
#define DEFALUT_MQTT_PASSWD "ClimbSnail.v0"

// Bilibili的持久化配置
#define HEARTBEAT_CONFIG_PATH "/heartbeat_v2.01.cfg"

extern AppController *app_controller; // APP控制器

// 常驻数据，可以不随APP的生命周期而释放或删除
struct HeartbeatAppForeverData
{
    char mqtt_server[32];    // 服务器地址
    uint16_t mqtt_port;      // mqtt服务器端口
    char mqtt_user[16];      // mqtt服务端的用户名
    char mqtt_password[16];  // mqtt服务的密码
    char mqtt_client_id[64]; // 连接是使用的client_id default "AIO_"+MAC
    int role;                // 0: heart, 1: beat 目前已无作用
    char qq_num[20];         // 用户设置的ID

    char mac_id[20]; // 唯一mac地址
    // 这里发布和订阅是一样的
    char mqtt_subtopic[32];    // 订阅的主题"/cubic/hb/"
    char mqtt_pubtopic[32];    // 发布的主题"/cubic/hb/"
    WiFiClient espClient;      // 定义wifiClient实例
    PubSubClient *mqtt_client; //(mqtt_server, 1883, callback, espClient);
    static void callback(char *topic, byte *payload, unsigned int length);
    void mqtt_reconnect();
};

HeartbeatAppForeverData hb_cfg;

void HeartbeatAppForeverData::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("Message arrived [%s], ", topic);
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]); // 打印主题内容
    }
    Serial.println();
    payload[length] = 0;
    if (strcmp(hb_cfg.mac_id, (char *)payload))
    {
        app_controller->send_to(HEARTBEAT_APP_NAME, CTRL_NAME, APP_MESSAGE_MQTT_DATA, NULL, NULL);
    }
    else
    {
        // 是自己发送出去的消息
        Serial.print("Own message!\n");
    }
}

static void write_config(HeartbeatAppForeverData *cfg)
{
    char tmp[32];
    String w_data;

    memset(tmp, 0, 32);
    snprintf(tmp, 32, "%s\n", cfg->mqtt_server);
    w_data += tmp;

    memset(tmp, 0, 32);
    snprintf(tmp, 32, "%u\n", cfg->mqtt_port);
    w_data += tmp;

    memset(tmp, 0, 32);
    snprintf(tmp, 32, "%s\n", cfg->mqtt_user);
    w_data += tmp;

    memset(tmp, 0, 32);
    snprintf(tmp, 32, "%s\n", cfg->mqtt_password);
    w_data += tmp;

    memset(tmp, 0, 32);
    snprintf(tmp, 16, "%d\n", cfg->role);
    w_data += tmp;

    memset(tmp, 0, 32);
    snprintf(tmp, 32, "%s\n", cfg->qq_num);
    w_data += tmp;

    g_flashCfg.writeFile(HEARTBEAT_CONFIG_PATH, w_data.c_str());
}

static void read_config(HeartbeatAppForeverData *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[256] = {0};
    uint16_t size = g_flashCfg.readFile(HEARTBEAT_CONFIG_PATH, (uint8_t *)info);
    Serial.printf("size %d\n", size);
    info[size] = 0;
    if (size == 0)
    {
        // 设置了mqtt服务器才能运行！
        Serial.println("Please config mqtt first!");
        // 默认值
        strcpy(cfg->mqtt_server, DEFALUT_MQTT_ADDR);
        cfg->mqtt_port = DEFALUT_MQTT_PORT; // mqtt服务端口
        strcpy(cfg->mqtt_user, DEFALUT_MQTT_USERNAME);
        strcpy(cfg->mqtt_password, DEFALUT_MQTT_PASSWD);
        cfg->role = 0; // 角色
        strcpy(cfg->qq_num, "77318186");
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[6] = {0};
        int ind = 0;
        analyseParam(info, 6, param);

        strcpy(cfg->mqtt_server, param[ind++]);
        Serial.printf("mqtt_server %s\n", cfg->mqtt_server);

        cfg->mqtt_port = atol(param[ind++]);
        Serial.printf("mqtt_port %u\n", cfg->mqtt_port);

        strcpy(cfg->mqtt_user, param[ind++]);
        Serial.printf("mqtt_mqtt_user %s\n", cfg->mqtt_user);

        strcpy(cfg->mqtt_password, param[ind++]);
        Serial.printf("mqtt_mqtt_password %s\n", cfg->mqtt_password);

        cfg->role = atoi(param[ind++]);
        Serial.printf(HEARTBEAT_APP_NAME " role %d\n", cfg->role);

        strcpy(cfg->qq_num, param[ind++]);
        Serial.printf("qq_num %s\n", cfg->qq_num);
    }

    // 订阅的主题
    snprintf(cfg->mqtt_subtopic, 32, "/cubic/hb/%s", cfg->qq_num);
    Serial.printf("mqtt_subtopic %s\n", cfg->mqtt_subtopic);
    // 发布的主题
    snprintf(cfg->mqtt_pubtopic, 32, "/cubic/hb/%s", cfg->qq_num);
    Serial.printf("mqtt_pubtopic %s\n", cfg->mqtt_pubtopic);

    snprintf(cfg->mqtt_client_id, 64, "AIO_%llu", ESP.getEfuseMac());
    Serial.printf("mqtt_client_id : %s\n", cfg->mqtt_client_id);

    snprintf(cfg->mac_id, 24, "%llu", ESP.getEfuseMac());
    Serial.printf("mac_id : %s\n", cfg->mac_id);
}

void HeartbeatAppForeverData::mqtt_reconnect()
{
    Serial.print("Attempting MQTT connection...\n");
    if (NULL == mqtt_client)
    {
        Serial.print("MQTT Client Error!\n");
        return;
    }

    if (mqtt_client->connect(mqtt_client_id, mqtt_user, mqtt_password))
    {
        Serial.println("mqtt connected");
        // 连接成功时订阅主题
        mqtt_client->subscribe(mqtt_subtopic);
        Serial.printf("%s subcribed\n", mqtt_subtopic);
    }
    else
    {
        Serial.printf("failed, rc=%d\n", mqtt_client->state());
    }
}

// 动态数据，APP的生命周期结束也需要释放它
struct HeartbeatAppRunData
{
    uint8_t send_cnt = 0;
    uint8_t recv_cnt = 0;
    unsigned long preUpdataMillis;     // 上一回更新的毫秒数
    unsigned long timeUpdataInterval;  // 更新时间计数器
    unsigned long heartContinueMillis; // 心跳的持续时间
    unsigned long lastHeartUpdataTime; // 上次心跳的更新时间
};

// 保存APP运行时的参数信息，理论上关闭APP时推荐在 xxx_exit_callback 中释放掉
static HeartbeatAppRunData *run_data = NULL;

// 当然你也可以添加恒定在内存中的少量变量（退出时不用释放，实现第二次启动时可以读取）

static int heartbeat_init(AppController *sys)
{
    heartbeat_gui_init();
    // 初始化运行时参数
    run_data = (HeartbeatAppRunData *)calloc(1, sizeof(HeartbeatAppRunData));
    run_data->send_cnt = 0;
    run_data->recv_cnt = 0;
    run_data->timeUpdataInterval = 5000; // 日期时钟更新的时间间隔30000(30s)
    // run_data->preUpdataMillis = GET_SYS_MILLIS() - run_data->timeUpdataInterval;
    run_data->preUpdataMillis = GET_SYS_MILLIS();
    run_data->heartContinueMillis = 10000; // 心跳的持续时间10s
    // 上次心跳的更新时间
    run_data->lastHeartUpdataTime = GET_SYS_MILLIS() - run_data->heartContinueMillis;

    read_config(&hb_cfg);
    hb_cfg.mqtt_client = NULL;

    // 初始化MQTT
    if (NULL == hb_cfg.mqtt_client)
    {
        if (hb_cfg.mqtt_server[0] >= '0' && hb_cfg.mqtt_server[0] <= '9')
        {
            // ip地址
            IPAddress serverIp;
            serverIp.fromString(hb_cfg.mqtt_server);
            hb_cfg.mqtt_client = new PubSubClient(serverIp, hb_cfg.mqtt_port,
                                                  hb_cfg.callback, hb_cfg.espClient);
        }
        else
        {
            // 域名
            hb_cfg.mqtt_client = new PubSubClient(hb_cfg.mqtt_server, hb_cfg.mqtt_port,
                                                  hb_cfg.callback, hb_cfg.espClient);
        }
    }

    // 连接wifi，并开启mqtt客户端
    sys->send_to(HEARTBEAT_APP_NAME, CTRL_NAME, APP_MESSAGE_WIFI_CONN, NULL, NULL);
    return 0;
}

static void heartbeat_process(AppController *sys,
                              const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    else if (GO_FORWORD == act_info->active) // 向前按发送一条消息
    {
        anim_type = LV_SCR_LOAD_ANIM_MOVE_TOP;
        run_data->send_cnt += 1;
        if (hb_cfg.mqtt_client->connected())
        {
            hb_cfg.mqtt_client->publish(hb_cfg.mqtt_pubtopic, hb_cfg.mac_id);
            Serial.printf(HEARTBEAT_APP_NAME " sent publish %s successful\n",
                          hb_cfg.mqtt_pubtopic);
        }

        // 发送指示灯
        // RgbParam rgb_setting = {LED_MODE_RGB,
        //                     0, 0, 0,
        //                     240,240,240,
        //                     60, 60, 60,
        //                     0.15, 0.25,
        //                     0.001, 4};
        // set_rgb_and_run(&rgb_setting);
    }

    if (run_data->recv_cnt > 0 && run_data->send_cnt > 0)
    {
        heartbeat_set_sr_type(HEART);
    }
    else if (run_data->recv_cnt > 0)
    {
        heartbeat_set_sr_type(RECV);
    }
    else if (run_data->send_cnt == 0) // 进入app时自动发送mqtt消息
    {
        heartbeat_set_sr_type(SEND);
        run_data->send_cnt += 1;
        if (hb_cfg.mqtt_client->connected())
        {

            hb_cfg.mqtt_client->publish(hb_cfg.mqtt_pubtopic, hb_cfg.mac_id);
            Serial.printf(HEARTBEAT_APP_NAME " sent publish %s successful\n",
                          hb_cfg.mqtt_pubtopic);
        }
    }

    if (GET_SYS_MILLIS() - run_data->lastHeartUpdataTime >= run_data->heartContinueMillis)
    {
        // 用于停止heart
        heartbeat_set_sr_type(SEND);
    }

    if (NULL != hb_cfg.mqtt_client)
    {
        // 以下减少网络请求的压力
        if (doDelayMillisTime(run_data->timeUpdataInterval, &run_data->preUpdataMillis, false))
        {
            // 发送请求。如果是wifi相关的消息，
            // 当请求完成后自动会调用 heartbeat_message_handle 函数
            sys->send_to(HEARTBEAT_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);
        }
    }

    // 程序需要时可以适当加延时
    display_heartbeat("heartbeat", anim_type);
    heartbeat_set_send_recv_cnt_label(run_data->send_cnt, run_data->recv_cnt);
    display_heartbeat_img();
    delay(30);
}

static void heartbeat_background_task(AppController *sys,
                                      const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int heartbeat_exit_callback(void *param)
{
    // 释放资源
    heartbeat_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void heartbeat_message_handle(const char *from, const char *to,
                                     APP_MESSAGE_TYPE type, void *message,
                                     void *ext_info)
{
    // 目前主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.println(F("MQTT keep alive"));

        if (!hb_cfg.mqtt_client->connected())
        {
            hb_cfg.mqtt_reconnect();
        }
        else
        {
            hb_cfg.mqtt_client->loop(); // 开启mqtt客户端
        }
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        Serial.println(F("MQTT keep alive(APP_MESSAGE_WIFI_ALIVE)"));
        if (!hb_cfg.mqtt_client->connected())
        {
            hb_cfg.mqtt_reconnect();
        }
        else
        {
            hb_cfg.mqtt_client->loop(); // 开启mqtt客户端
        }
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "role"))
        {
            snprintf((char *)ext_info, 32, "%d", hb_cfg.role);
        }
        else if (!strcmp(param_key, "qq_num"))
        {
            snprintf((char *)ext_info, 32, "%s", hb_cfg.qq_num);
        }
        else if (!strcmp(param_key, "mqtt_server"))
        {
            snprintf((char *)ext_info, 32, "%s", hb_cfg.mqtt_server);
        }
        else if (!strcmp(param_key, "mqtt_port"))
        {
            snprintf((char *)ext_info, 32, "%u", hb_cfg.mqtt_port);
        }
        else if (!strcmp(param_key, "mqtt_user"))
        {
            snprintf((char *)ext_info, 32, "%s", hb_cfg.mqtt_user);
        }
        else if (!strcmp(param_key, "mqtt_password"))
        {
            snprintf((char *)ext_info, 32, "%s", hb_cfg.mqtt_password);
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "role"))
        {
            hb_cfg.role = atol(param_val);
            Serial.printf("hb role %d", hb_cfg.role);
            Serial.println();
        }
        else if (!strcmp(param_key, "qq_num"))
        {
            snprintf((char *)hb_cfg.qq_num, 32, "%s", param_val);
            Serial.printf("qq_num %s\n", hb_cfg.qq_num);
        }
        else if (!strcmp(param_key, "mqtt_server"))
        {
            snprintf((char *)hb_cfg.mqtt_server, 32, "%s", param_val);
            Serial.printf("mqtt_server %s\n", hb_cfg.mqtt_server);
        }
        else if (!strcmp(param_key, "mqtt_port"))
        {
            hb_cfg.mqtt_port = atol(param_val);
            Serial.printf("mqtt_port %u\n", hb_cfg.mqtt_port);
        }
        else if (!strcmp(param_key, "mqtt_user"))
        {
            snprintf((char *)hb_cfg.mqtt_user, 16, "%s", param_val);
            Serial.printf("mqtt_user %s\n", param_val);
        }
        else if (!strcmp(param_key, "mqtt_password"))
        {
            snprintf((char *)hb_cfg.mqtt_password, 16, "%s", param_val);
            Serial.printf("mqtt_password %s\n", param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&hb_cfg);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&hb_cfg);
    }
    break;
    case APP_MESSAGE_MQTT_DATA:
    {
        // if (run_data->send_cnt > 0) //已经手动发送过了
        // {
        //     heartbeat_set_sr_type(HEART);
        // }
        // else
        // {
        //     heartbeat_set_sr_type(RECV);
        // }
        heartbeat_set_sr_type(HEART);
        run_data->lastHeartUpdataTime = GET_SYS_MILLIS();
        /* 亮一下 */
        // RgbParam rgb_setting = {LED_MODE_RGB,
        //                     0, 0, 0,
        //                     3,36,86,
        //                     1, 1, 1,
        //                     0.15, 0.25,
        //                     0.001, 8};
        // set_rgb_and_run(&rgb_setting);
        run_data->recv_cnt++;
        Serial.println("received heartbeat");
    }
    break;
    default:
        break;
    }
}

APP_OBJ heartbeat_app = {HEARTBEAT_APP_NAME, &app_heartbeat, "Author WoodwindHu\nVersion 2.0.0\n",
                         heartbeat_init, heartbeat_process, heartbeat_background_task,
                         heartbeat_exit_callback, heartbeat_message_handle};
