#include "message.h"
#include "file_manager.h"
#include "file_manager_gui.h"
#include "sys/app_controller.h"
#include "network.h"
#include "common.h"
#include "ESP32FtpServer.h"

#define FILE_MANAGER_APP_NAME "File Manager"

#define FILE_MANAGER_REFLUSH_INTERVAL 2000UL // Configuration interface refresh time (2s)

#define RECV_BUFFER_SIZE 2000    //
#define SEND_BUFFER_SIZE 2000    //
#define SERVER_PORT 8081         // Set listening port
#define SHARE_WIFI_ALIVE 20000UL // Time to maintain WiFi heartbeat (20s)

FtpServer ftpSrv; // Define FTP server

struct FileManagerAppRunData
{
    boolean tcp_start;                    // Flag whether the web server service is enabled, 0 for off, 1 for on
    boolean req_sent;                     // Flag whether the WiFi request service has been sent, 0 for off, 1 for on
    unsigned long serverReflushPreMillis; // Time of last update
    unsigned long apAlivePreMillis;       // Time of last update
    uint8_t *recvBuf;                     // Receive data buffer
    uint8_t *sendBuf;                     // Send data buffer
};

static FileManagerAppRunData *run_data = NULL;
static int file_maneger_init(AppController *sys)
{
    file_maneger_gui_init();
    // Initialize runtime parameters
    run_data = (FileManagerAppRunData *)calloc(1, sizeof(FileManagerAppRunData));
    run_data->tcp_start = 0;
    run_data->req_sent = 0;
    run_data->serverReflushPreMillis = 0;
    run_data->recvBuf = (uint8_t *)calloc(1, RECV_BUFFER_SIZE);
    run_data->sendBuf = (uint8_t *)calloc(1, SEND_BUFFER_SIZE);
    return 0;
}

static void file_maneger_process(AppController *sys,
                                 const ImuAction *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        sys->app_exit();
        return;
    }

    if (0 == run_data->tcp_start && 0 == run_data->req_sent)
    {
        // Pre-display
        display_file_manager(
            "File Manager",
            WiFi.softAPIP().toString().c_str(),
            "21",
            "Wait connect ....",
            LV_SCR_LOAD_ANIM_NONE);
        // If web service is not started and AP start request has not been sent, event_id has no effect (fill 0)
        sys->send_to(FILE_MANAGER_APP_NAME, CTRL_NAME,
                     APP_MESSAGE_WIFI_CONN, NULL, NULL);
        run_data->req_sent = 1; // Flag as AP start request sent
    }
    else if (1 == run_data->tcp_start)
    {
        if (doDelayMillisTime(SHARE_WIFI_ALIVE, &run_data->apAlivePreMillis, false))
        {
            // Send WiFi heartbeat
            sys->send_to(FILE_MANAGER_APP_NAME, CTRL_NAME,
                         APP_MESSAGE_WIFI_ALIVE, NULL, NULL);
        }
        ftpSrv.handleFTP(); // make sure in loop you call handleFTP()!!
    }
}

static void file_maneger_background_task(AppController *sys,
                                         const ImuAction *act_info)
{
    // This function is a background task, the main controller will call this function every minute
    // This function should only call "resident data", other variables may have been released due to lifecycle
}

static int file_maneger_exit_callback(void *param)
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

    // Release runtime data
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void file_maneger_message_handle(const char *from, const char *to,
                                        APP_MESSAGE_TYPE type, void *message,
                                        void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "21",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
        ftpSrv.begin("holocubic", "aio");
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        Serial.print(F("APP_MESSAGE_WIFI_AP enable\n"));
        display_file_manager(
            "File Manager",
            WiFi.localIP().toString().c_str(),
            "21",
            "Connect succ",
            LV_SCR_LOAD_ANIM_NONE);
        run_data->tcp_start = 1;
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // WiFi heartbeat maintenance response, can do nothing
    }
    break;
    default:
        break;
    }
}

APP_OBJ file_manager_app = {FILE_MANAGER_APP_NAME, &app_file_manager, "",
                            file_maneger_init, file_maneger_process, file_maneger_background_task,
                            file_maneger_exit_callback, file_maneger_message_handle};
                            