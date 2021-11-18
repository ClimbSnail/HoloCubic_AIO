#include "timeUtile.h"
#include "WiFi.h"
#include "common.h"

void TimeUtile::init()
{
    g_rtc.setTime(g_cfg.last_time);
    configTime(TIMEZERO_OFFSIZE, 0, ntp_server.c_str());
    is_net = false;
    set_interval(900000);
}

void TimeUtile::set_interval(unsigned long updataInterval, bool force)
{
    // 初始化运行时参数
    timeUpdataInterval = updataInterval; // 日期时钟更新的时间间隔(900s)
    if (force)
    {
        // 变相强制更新
        preTimeMillis = millis() - timeUpdataInterval;
    }
}

void TimeUtile::force_update()
{
    preTimeMillis = millis() - timeUpdataInterval;
}

void TimeUtile::update_time()
{

    //同步网络时间就可以了，ESP32Time内部会做RTC本地时钟同步
    if (is_net == false)
    {
        //如果没有联网同步过，加快联网同步频率
        if (doDelayMillisTime(timeUpdataInterval / 10, &preTimeMillis, false))
        {
            get_timestamp_url();
        }
    }
    else
    {
        // 以下减少网络请求的压力
        if (doDelayMillisTime(timeUpdataInterval, &preTimeMillis, false))
        {
            // 尝试同步网络上的时钟
            get_timestamp_url();
        }
    }
}

void TimeUtile::get_timestamp_url()
{
    if ((WiFi.getMode() & WIFI_MODE_STA) == WIFI_MODE_STA)
    {
        if (CONN_SUCC != g_network.end_conn_wifi())
        {
            g_network.start_conn_wifi(g_cfg.ssid.c_str(), g_cfg.password.c_str());
        }
        else if (WiFi.status() == WL_CONNECTED)
        {

            configTime(TIMEZERO_OFFSIZE, 0, ntp_server.c_str());
            struct tm timeinfo;
            if (!getLocalTime(&timeinfo))
            {
                Serial.println("Failed to obtain time");
                return;
            }
            Serial.print(asctime(&timeinfo));
            g_cfg.last_time = mktime(&timeinfo);
            g_rtc.setTime(g_cfg.last_time);
            time_save(NULL, &g_cfg);
            is_net = true;
        }
    }
}
