#include "file_msg.h"
#include "file_manager.h"
#include "file_manager_gui.h"
#include "sys/app_contorller.h"
#include "network.h"
#include "common.h"
#include "ESP32FtpServer.h"

#define FILE_MANAGER_REFLUSH_INTERVAL 2000UL // 配置界面重新刷新时间(2s)
#define RECV_BUFFER_SIZE 2000                //
#define SEND_BUFFER_SIZE 2000                //
#define HTTP_PORT 8081                       //设置监听端口

// WiFiServer file_manager_server; //服务端
// WiFiClient file_manager_client; // 客户端
FtpServer ftpSrv;               // 定义FTP服务端

struct FileManagerAppRunDate
{
    boolean tcp_start;                    // 标志是否开启web server服务，0为关闭 1为开启
    boolean req_sent;                     // 标志是否发送wifi请求服务，0为关闭 1为开启
    unsigned long serverReflushPreMillis; // 上一回更新的时间
    uint8_t *recvBuf;                     // 接收数据缓冲区
    uint8_t *sendBuf;                     // 发送数据缓冲区
};

static FileManagerAppRunDate *run_data = NULL;
void file_maneger_init(void)
{
    file_maneger_gui_init();
    // 初始化运行时参数
    run_data = (FileManagerAppRunDate *)calloc(1, sizeof(FileManagerAppRunDate));
    run_data->tcp_start = 0;
    run_data->req_sent = 0;
    run_data->serverReflushPreMillis = 0;
    // run_data->file_manager_server = new WiFiServer();
    // run_data->client = new WiFiClient();
    run_data->recvBuf = (uint8_t *)calloc(1, RECV_BUFFER_SIZE);
    run_data->sendBuf = (uint8_t *)calloc(1, SEND_BUFFER_SIZE);
}

void file_maneger_process(AppController *sys,
                          const Imu_Action *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (0 == run_data->tcp_start && 0 == run_data->req_sent)
    {
        // 预显示
        display_file_manager(
            "File Manager",
            WiFi.softAPIP().toString().c_str(),
            "8081",
            "Wait connect ....",
            LV_SCR_LOAD_ANIM_NONE);
        // 如果web服务没有开启 且 ap开启的请求没有发送 event_id这边没有作用（填0）
        sys->req_event(&file_manager_app, APP_EVENT_WIFI_CONN, 0);
        run_data->req_sent = 1; // 标志为 ap开启请求已发送
    }
    else if (1 == run_data->tcp_start)
    {
        // return ;
        // 发送wifi维持的心跳
        sys->req_event(&file_manager_app, APP_EVENT_WIFI_ALIVE, 0);
#if 0
        if (NULL != file_manager_client)
        {
            // 如果客户端处于连接状态 client.connected()
            file_manager_server.hasClient();
            if (file_manager_client.connected() || file_manager_client.available())
            {
                if (file_manager_client.available())
                {
                    memset(run_data->recvBuf, 0, RECV_BUFFER_SIZE);
                    int data_len = file_manager_client.read(run_data->recvBuf, 1024); //读取帧大小
                    FileSystem fs_msg;
                    uint32_t len = fs_msg.decode(run_data->recvBuf);

                    if (AT_DIR_LIST == fs_msg.m_action_type)
                    {
                        Serial.print("AT_DIR_LIST: ");
                        DirList recv_msg;
                        uint32_t recv_len = recv_msg.decode(run_data->recvBuf);
                        Serial.println(recv_msg.m_dir_path);

                        File_Info *file = tf.listDir(recv_msg.m_dir_path);
                        File_Info *p = file->next_node;
                        if (NULL != p)
                        {
                            char dir_info[400] = {0};
                            int index = 0;
                            do
                            {
                                if (FILE_TYPE_FILE == p->file_type)
                                {
                                    index += sprintf(&dir_info[index], "%s\t", p->file_name);
                                }
                                else
                                {
                                    index += sprintf(&dir_info[index], "%s/\t", p->file_name);
                                }
                                p = p->next_node;
                            } while (p != file->next_node);

                            Serial.println(dir_info);
                            memset(run_data->sendBuf, 0, SEND_BUFFER_SIZE);
                            DirList send_msg(recv_msg.m_dir_path, dir_info);
                            uint32_t send_len = send_msg.encode(run_data->sendBuf);
                            file_manager_client.write(run_data->sendBuf, send_len);
                        }
                    }
                    else
                    {
                    }
                }
            }
            else if (!file_manager_client.connected())
            {
                file_manager_client.stop();
                Serial.println("Controller was disconnect!");
            }
        }
        else
        {
            // 建立客户
            file_manager_client = file_manager_server.available();
            if (file_manager_client)
            {
                Serial.println("Controller was connected!");
                FileSystem fs_msg;
                uint8_t msg[1024];
                uint32_t len = fs_msg.encode(msg);
                file_manager_client.write(msg, len); // 向上位机发送下一帧发送指令
            }
    }
#else
        ftpSrv.handleFTP(); // make sure in loop you call handleFTP()!!
#endif
    }
}

void file_maneger_exit_callback(void)
{
    file_manager_gui_del();

    if (NULL == run_data->recvBuf)
    {
        free(run_data->recvBuf);
        run_data->recvBuf = NULL;
    }

    if (NULL == run_data->sendBuf)
    {
        free(run_data->sendBuf);
        run_data->sendBuf = NULL;
    }

    // file_manager_server.close();

    // 释放运行时参数
    free(run_data);
    run_data = NULL;
}

void file_maneger_event_notification(APP_EVENT event, int event_id)
{
    switch (event)
    {
    case APP_EVENT_WIFI_CONN:
    {
        Serial.print(F("APP_EVENT_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
        // file_manager_server.begin(HTTP_PORT); //服务器启动监听端口号
        // file_manager_server.setNoDelay(true);
        ftpSrv.begin("esp32", "esp32");
    }
    break;
    case APP_EVENT_WIFI_AP:
    {
        Serial.print(F("APP_EVENT_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "8081",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
    }
    break;
    case APP_EVENT_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    default:
        break;
    }
}

APP_OBJ file_manager_app = {"File Manager", &app_file_maneger, file_maneger_init,
                            file_maneger_process, file_maneger_exit_callback,
                            file_maneger_event_notification};