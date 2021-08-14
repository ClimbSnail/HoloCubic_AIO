#include "server.h"
#include "server_gui.h"
#include "../sys/app_contorller.h"
#include "../../network.h"
#include "../../common.h"

void server_init(void)
{
    server_gui_init();
}

void server_process(AppController *sys,
                    const Imu_Action *action)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;

    if (RETURN == action->active)
    {
        g_network.stop_web_config();
        sys->app_exit();
        return;
    }

    if (0 == g_network.m_web_start)
    {
        // 如果STA模式连接失败 切换成ap模式
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            g_network.open_ap(AP_SSID);
        }
        display_setting(g_network.get_localIp().c_str(),
                        g_network.get_softAPIP().c_str(),
                        "Domain: holocubic",
                        "WebServer Start", anim_type);
        g_network.start_web_config();
    }
    server.handleClient(); // 一定需要放在循环里扫描
}

void server_exit_callback(void)
{
    setting_gui_del();
}

void server_event_notification(APP_EVENT event)
{
}

APP_OBJ server_app = {"Server", &app_server, server_init,
                      server_process, server_exit_callback,
                      server_event_notification};