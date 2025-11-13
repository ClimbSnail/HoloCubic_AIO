#include "settings.h"
#include "message.h"
#include "settings_gui.h"
#include "sys/app_controller.h"
#include "common.h"

#define NEW_VERSION "http://climbsnail.cn:5001/holocubicAIO/sn/v1/version/firmware"
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
        // If the message is legal
        if (AT_SETTING_SET == msg.m_msg_head.m_action_type)
        {
            // prefs.begin(msg.m_prefs_name); // Open namespace mynamespace
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
            // prefs.end(); // Close current namespace
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
            // prefs.begin(msg.m_prefs_name); // Open namespace mynamespace
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
            // prefs.end(); // Close current namespace
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
 *  Parse serial port data
 *  Frame format: Frame header (2 bytes) + Frame length (2 bytes) + Sender (2 bytes) + Receiver (2 bytes) + Message type (2 bytes) + Message data (frame length - 10) + Frame tail /r/n (2 bytes)
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

    // Find frame tail
    for (int pos = 4; pos < data_len - 1; ++pos)
    {
        if (data[pos] == '\r' && data[pos + 1] == '\n')
        {
            // Found frame tail
            found_flag = true;
            rear_ind = pos + 1; // Index of the last byte
            break;
        }
    }
    if (found_flag == false)
    {
        // Frame tail not found
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
        // Whether this frame is completely processed or not processed due to incompleteness, perform cleanup
        memcpy(data, data + rear_ind + 1, data_len - (rear_ind + 1));
        run_data->recv_len = data_len - (rear_ind + 1);
    }
    return 0;
}

static int settings_init(AppController *sys)
{
    // Initialize runtime parameters
    settings_gui_init();

    display_settings(AIO_VERSION, AIO_VERSION, LV_SCR_LOAD_ANIM_NONE);

    // Initialize runtime parameters
    run_data = (SettingsAppRunData *)calloc(1, sizeof(SettingsAppRunData));
    run_data->recv_buf = (uint8_t *)malloc(RECV_BUF_LEN);
    run_data->recv_len = 0;
    sys->send_to(SETTINGS_APP_NAME, CTRL_NAME,
                 APP_MESSAGE_WIFI_CONN, NULL, NULL);
    return 0;
}

static void settings_process(AppController *sys,
                             const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // Exit APP
        return;
    }

    if (GO_FORWORD == act_info->active)
    {
        sys->send_to(SETTINGS_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        delay(500);
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

    // Send request, the settings_event_notification function will be called automatically when the request is completed
    // sys->req_event(&settings_app, APP_MESSAGE_WIFI_CONN, run_data->val1);
    // Can add appropriate delay when the program needs it
    // delay(200);
}

static void get_new_version(char *version)
{
    HTTPClient httpClient;
    httpClient.setTimeout(1000);
    bool status = httpClient.begin(NEW_VERSION);
    if (status == false)
    {
        snprintf(version, 16, "v UNKNOWN");
    }
    int httpCode = httpClient.GET();
    String httpResponse = httpClient.getString();
    httpClient.end();
    snprintf(version, 16, "%s", httpResponse.c_str() + 13);
    Serial.printf("latest version = %s\n", version);
}

static void settings_background_task(AppController *sys,
                                     const ImuAction *act_info)
{
    // This function is a background task, the main controller will call this function every minute
    // This function should only call "resident data", other variables may have been released due to lifecycle
}

static int settings_exit_callback(void *param)
{
    settings_gui_del();

    // Release runtime data
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
    // Currently events are mainly WiFi on/off events (for power consumption control)
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
        char ver[16];
        get_new_version(ver);
        display_settings(AIO_VERSION, ver, LV_SCR_LOAD_ANIM_NONE);
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // WiFi heartbeat maintenance response, can do nothing
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
