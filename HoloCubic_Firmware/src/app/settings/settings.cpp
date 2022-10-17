#include "settings.h"
#include "message.h"
#include "settings_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#define SETTINGS_APP_NAME "Settings"
#define RECV_BUF_LEN 128

struct SettingsAppRunData
{
    uint8_t *recv_buf;
    uint16_t recv_len;
};

static SettingsAppRunData *run_data = NULL;

int exec_order(int len, const uint8_t *data)
{
    Serial.print("do exec_order\n");
    SettingsMsg msg;
    msg.decode(data);
    if (msg.isLegal() && MODULE_TYPE_CUBIC_SETTINGS == msg.m_msg_head.m_to_who)
    {
        // 如果消息合法
        if (AT_SETTING_SET == msg.m_msg_head.m_action_type)
        {
            // prefs.begin(msg.m_prefs_name); // 打开命名空间mynamespace
            switch (msg.m_value_type)
            {
            case VALUE_TYPE_INT:
            {
                // prefs.putInt(msg.m_key, msg.m_value[0] << 8 + msg.m_value[1]);
            }
            break;
            case VALUE_TYPE_UCHAR:
            {
                // prefs.putUChar(msg.m_key, msg.m_value[0]);
            }
            break;
            case VALUE_TYPE_STRING:
            {
                // prefs.putString(msg.m_key, (char *)msg.m_value);
            }
            break;
            default:
                break;
            }
            // prefs.end(); // 关闭当前命名空间
            Serial.print("Set OK\n");
        }
        else if (AT_SETTING_GET == msg.m_msg_head.m_action_type)
        {
            Serial.print("Get start\n");
            Serial.print(" m_prefs_name-->");
            Serial.print(msg.m_prefs_name);
            Serial.print(" m_key-->");
            Serial.print(msg.m_key);
            Serial.print(" ");
            // prefs.begin(msg.m_prefs_name); // 打开命名空间mynamespace
            switch (msg.m_value_type)
            {
            case VALUE_TYPE_INT:
            {
                int value = 0; // prefs.getInt(msg.m_key);
                msg.m_value[0] = value >> 8;
                msg.m_value[1] = value | 0x00FF;
                Serial.print(" getInt-->");
            }
            break;
            case VALUE_TYPE_UCHAR:
            {
                msg.m_value[0] = 0; // prefs.getUChar(msg.m_key);
                Serial.print(" getUChar-->");
            }
            break;
            case VALUE_TYPE_STRING:
            {
                String value = ""; // prefs.getString(msg.m_key);
                strncpy((char *)msg.m_value, value.c_str(), 15);
                Serial.print(" getString-->");
            }
            break;
            default:
                break;
            }
            // prefs.end(); // 关闭当前命名空间
            unsigned char send_msg[64];
            msg.m_msg_head.m_from_who = MODULE_TYPE_CUBIC_SETTINGS;
            msg.m_msg_head.m_to_who = MODULE_TYPE_TOOL_SETTINGS;
            int count = msg.encode(send_msg);
            Serial.write((char *)send_msg, count);
            Serial.print("Get succeed\n");
        }
    }
    else
    {
        Serial.print("exec_order isn't Legal");
    }
    return 0;
}

/*
 *  解析串口数据
 *  帧格式为 帧头（2字节）+ 帧长度（2字节）+ 发送者（2字节）+ 接收者（2字节）+ 消息类型（2字节）+ 消息数据（帧长度-10）+ 帧尾/r/n（2字节）
 *
 */
int analysis_uart_data(int data_len, uint8_t *data)
{
    Serial.print("do analysis_uart_data\n");
    if (NULL == data || data_len < 4)
    {
        Serial.print("NULL == data\n");
        return -1;
    }
    int frame_len = data[2] << 8 | data[3];
    bool found_flag = false;
    int rear_ind = 0;

    // 找帧尾
    for (int pos = 4; pos < data_len - 1; ++pos)
    {
        if (data[pos] == '\r' && data[pos + 1] == '\n')
        {
            // 找到帧尾
            found_flag = true;
            rear_ind = pos + 1; // 最后一字节所在的下标
            break;
        }
    }
    if (found_flag == false)
    {
        // 没找到帧尾
        Serial.print("no found rear\n");
        return -1;
    }
    else
    {
        Serial.print("found rear\n");
        if (frame_len == rear_ind + 1)
        {
            exec_order(frame_len, data);
        }
        // 此帧不管是完整处理还是因为不完整而没被处理，都进行清楚处理
        memcpy(data, data + rear_ind + 1, data_len - (rear_ind + 1));
        run_data->recv_len = data_len - (rear_ind + 1);
    }
    return 0;
}

static int settings_init(AppController *sys)
{
    // 初始化运行时的参数
    settings_gui_init();

    display_settings(AIO_VERSION, "v 2.3.0", LV_SCR_LOAD_ANIM_NONE);

    // 初始化运行时参数
    run_data = (SettingsAppRunData *)calloc(1, sizeof(SettingsAppRunData));
    run_data->recv_buf = (uint8_t *)malloc(RECV_BUF_LEN);
    run_data->recv_len = 0;
    return 0;
}

static void settings_process(AppController *sys,
                             const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }

    if (Serial.available())
    {
        uint16_t len = Serial.read(run_data->recv_buf + run_data->recv_len,
                                   RECV_BUF_LEN - run_data->recv_len);

        run_data->recv_len += len;
        if (run_data->recv_len > 0)
        {
            Serial.print("rev = ");

            Serial.write(run_data->recv_buf, len);
            analysis_uart_data(run_data->recv_len, run_data->recv_buf);
        }
        delay(50);
    }
    else
    {
        delay(200);
    }

    // 发送请求，当请求完成后自动会调用 settings_event_notification 函数
    // sys->req_event(&settings_app, APP_MESSAGE_WIFI_CONN, run_data->val1);
    // 程序需要时可以适当加延时
    // delay(200);
}

static void settings_background_task(AppController *sys,
                                     const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int settings_exit_callback(void *param)
{
    settings_gui_del();

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void settings_message_handle(const char *from, const char *to,
                                    APP_MESSAGE_TYPE type, void *message,
                                    void *ext_info)
{
    // 目前事件主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
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
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
    }
    break;
    default:
        break;
    }
}

APP_OBJ settings_app = {SETTINGS_APP_NAME, &app_settings, "",
                        settings_init, settings_process, settings_background_task,
                        settings_exit_callback, settings_message_handle};
