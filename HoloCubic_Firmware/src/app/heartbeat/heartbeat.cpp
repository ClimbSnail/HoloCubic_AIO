#include "heartbeat.h"
#include "heartbeat_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "network.h"
#include <PubSubClient.h>

#define HEARTBEAT_APP_NAME "Heartbeat"

#define HEARTBEAT_CONFIG_PATH "/heartbeat.cfg"

extern AppController *app_controller; // APP控制器

// 常驻数据，可以不随APP的生命周期而释放或删除
struct HeartbeatAppForeverData
{
    long role;                  // 0: heart, 1: beat
    char liz_mqtt_subtopic[10]; // "/beat"
    char liz_mqtt_pubtopic[10]; // "/heart"
    char client_id[10];         // "hc_heart"
    IPAddress mqtt_server;
    WiFiClient espClient;      // 定义wifiClient实例
    PubSubClient *mqtt_client; //(mqtt_server, 1883, callback, espClient);
    static void callback(char *topic, byte *payload, unsigned int length);
    void mqtt_reconnect();
};

void HeartbeatAppForeverData::callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic); // 打印主题信息
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]); // 打印主题内容
    }
    Serial.println();

    app_controller->send_to("Heartbeat", CTRL_NAME, APP_MESSAGE_MQTT_DATA, NULL, NULL);
}

HeartbeatAppForeverData hb_cfg;

static void write_config(HeartbeatAppForeverData *cfg)
{
    char tmp[16];
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%ld\n", cfg->role);
    w_data += tmp;
    w_data = w_data + cfg->mqtt_server.toString() + "\n";
    if (cfg->role == 0)
    {
        strcpy(cfg->liz_mqtt_subtopic, "/beat");
        strcpy(cfg->liz_mqtt_pubtopic, "/heart");
        strcpy(cfg->client_id, "hc_heart");
    }
    else
    {
        strcpy(cfg->liz_mqtt_subtopic, "/heart");
        strcpy(cfg->liz_mqtt_pubtopic, "/beat");
        strcpy(cfg->client_id, "hc_beat");
    }
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%s\n", cfg->liz_mqtt_subtopic);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%s\n", cfg->liz_mqtt_pubtopic);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%s\n", cfg->client_id);
    w_data += tmp;
    g_flashCfg.writeFile(HEARTBEAT_CONFIG_PATH, w_data.c_str());
}

static void read_config(HeartbeatAppForeverData *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(HEARTBEAT_CONFIG_PATH, (uint8_t *)info);
    Serial.printf("size %d\n", size);
    info[size] = 0;
    if (size == 0)
    {
        // 设置了mqtt服务器才能运行！
        Serial.println("Please config first!");
    }
    else
    {
        // 解析数据
        char *param[5] = {0};
        analyseParam(info, 5, param);
        cfg->role = atol(param[0]);
        Serial.printf("hb_role %ld", cfg->role);
        Serial.println();
        cfg->mqtt_server.fromString(param[1]);
        Serial.printf("mqtt_server %s", cfg->mqtt_server.toString().c_str());
        Serial.println();
        strcpy(cfg->liz_mqtt_subtopic, param[2]);
        Serial.printf("liz_mqtt_subtopic %s", cfg->liz_mqtt_subtopic);
        Serial.println();
        strcpy(cfg->liz_mqtt_pubtopic, param[3]);
        Serial.printf("liz_mqtt_pubtopic %s", cfg->liz_mqtt_pubtopic);
        Serial.println();
        strcpy(cfg->client_id, param[4]);
        Serial.printf("client_id %s", cfg->client_id);
        Serial.println();
        if (NULL == cfg->mqtt_client)
        {
            cfg->mqtt_client = new PubSubClient(cfg->mqtt_server, 1883, cfg->callback, cfg->espClient);
        }
    }
}

void HeartbeatAppForeverData::mqtt_reconnect()
{
    Serial.print("Attempting MQTT connection...");
    if (NULL == mqtt_client)
    {
        Serial.print("MQTT Client Error!\n");
    }
    // Attempt to connect
    if (mqtt_client->connect(client_id))
    {
        Serial.println("mqtt connected");
        // 连接成功时订阅主题
        mqtt_client->subscribe(liz_mqtt_subtopic);
        Serial.printf("%s subcribed", liz_mqtt_subtopic);
        Serial.println();
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(mqtt_client->state());
        Serial.println();
    }
}

// 动态数据，APP的生命周期结束也需要释放它
struct HeartbeatAppRunData
{
    uint8_t send_cnt = 0;
    uint8_t recv_cnt = 0;
};

// 保存APP运行时的参数信息，理论上关闭APP时推荐在 xxx_exit_callback 中释放掉
static HeartbeatAppRunData *run_data = NULL;

// 当然你也可以添加恒定在内存中的少量变量（退出时不用释放，实现第二次启动时可以读取）

static int heartbeat_init(void)
{
    // 获取配置参数
    // read_config(&hb_cfg); // already read;
    heartbeat_gui_init();
    // 初始化运行时参数
    run_data = (HeartbeatAppRunData *)calloc(1, sizeof(HeartbeatAppRunData));
    run_data->send_cnt = 0;
    run_data->recv_cnt = 0;
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
        hb_cfg.mqtt_client->publish(hb_cfg.liz_mqtt_pubtopic, "hello!");
        Serial.printf("sent publish %s successful", hb_cfg.liz_mqtt_pubtopic);
        Serial.println();
        // 发送指示灯
        // RgbParam rgb_setting = {LED_MODE_RGB,
        //                     0, 0, 0,
        //                     240,240,240,
        //                     60, 60, 60,
        //                     0.15, 0.25,
        //                     0.001, 4};
        // set_rgb(&rgb_setting);
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
        hb_cfg.mqtt_client->publish(hb_cfg.liz_mqtt_pubtopic, MQTT_SEND_MSG);
        Serial.printf("sent publish %s successful", hb_cfg.liz_mqtt_pubtopic);
    }
    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 heartbeat_message_handle 函数
    // sys->send_to(HEARTBEAT_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_ALIVE, NULL, NULL);

    // 程序需要时可以适当加延时
    display_heartbeat("heartbeat", anim_type);
    heartbeat_set_send_recv_cnt_label(run_data->send_cnt, run_data->recv_cnt);
    display_heartbeat_img();
    delay(30);
}

static int heartbeat_exit_callback(void *param)
{
    // 释放资源
    heartbeat_gui_del();
    free(run_data);
    run_data = NULL;
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
        hb_cfg.mqtt_client->loop(); // 开启mqtt客户端
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
        hb_cfg.mqtt_client->loop(); // 开启mqtt客户端
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "role"))
        {
            snprintf((char *)ext_info, 32, "%ld", hb_cfg.role);
        }
        else if (!strcmp(param_key, "mqtt_server"))
        {
            snprintf((char *)ext_info, 32, "%s", hb_cfg.mqtt_server.toString().c_str());
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
            Serial.printf("hb role %ld", hb_cfg.role);
            Serial.println();
        }
        else if (!strcmp(param_key, "mqtt_server"))
        {
            hb_cfg.mqtt_server.fromString(param_val);
            Serial.printf("mqtt_server %s", hb_cfg.mqtt_server.toString().c_str());
            Serial.println();
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
        if (run_data->send_cnt > 0) //已经手动发送过了
        {
            heartbeat_set_sr_type(HEART);
        }
        else
        {
            heartbeat_set_sr_type(RECV);
        }
        /* 亮一下 */
        // RgbParam rgb_setting = {LED_MODE_RGB,
        //                     0, 0, 0,
        //                     3,36,86,
        //                     1, 1, 1,
        //                     0.15, 0.25,
        //                     0.001, 8};
        // set_rgb(&rgb_setting);
        run_data->recv_cnt++;
        Serial.println("received heartbeat");
    }
    break;
    default:
        break;
    }
}

APP_OBJ heartbeat_app = {HEARTBEAT_APP_NAME, &app_heartbeat, "Author HQ\nVersion 2.0.0\n",
                         heartbeat_init, heartbeat_process,
                         heartbeat_exit_callback, heartbeat_message_handle};
