#include "sys/interface.h"

static int server_init(AppController *sys)
{
    (void)sys;
    return 0;
}

static void server_process(AppController *sys, const ImuAction *act_info)
{
    (void)sys;
    (void)act_info;
}

static void server_background_task(AppController *sys, const ImuAction *act_info)
{
    (void)sys;
    (void)act_info;
}

static int server_exit_callback(void *param)
{
    (void)param;
    return 0;
}

static void server_message_handle(const char *from, const char *to,
                                  APP_MESSAGE_TYPE type, void *message,
                                  void *ext_info)
{
    (void)from;
    (void)to;
    (void)type;
    (void)message;
    (void)ext_info;
}

APP_OBJ server_app = {
    "WebServer",
    (void *)0,
    "",
    server_init,
    server_process,
    server_background_task,
    server_exit_callback,
    server_message_handle
};