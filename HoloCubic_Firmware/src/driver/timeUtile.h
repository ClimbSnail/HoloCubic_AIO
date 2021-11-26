#ifndef TIME_UTILE_H
#define TIME_UTILE_H

// 时区偏移(小时) 8*60*60
#define TIMEZERO_OFFSIZE (28800)

#include "WiFi.h"
#include "ESP32Time.h"

class TimeUtile
{
    friend class AppController;

private:
    const String ntp_server = "ntp.aliyun.com"; //ntp 授时服务
    bool is_net = false;

    unsigned long preTimeMillis;      // 更新时间计数器
    unsigned long timeUpdataInterval; // 日期时钟更新的时间间隔(900s)

    void get_timestamp_url();
    /*****************************************************************************
	 * @brief 时间更新,根据updataInterval配置的更新间隔，间隔到达后使用网络校准时间
	 *****************************************************************************/
    void update_time();

public:
    void init();
    // 用于时间解码
    ESP32Time g_rtc;
    /*****************************************************************************
	 * @brief 设置联网更新频率
	 * @param updataInterval 联网更新的时间间隔,毫秒单位，默认(900000ms=900s)
     * @param force 是否马上强制从网络同步时间
	 *****************************************************************************/
    void set_interval(unsigned long updataInterval = 900000, bool force = true);
    /*****************************************************************************
	 * @brief 强制联网更新
	 *****************************************************************************/
    void force_update();
};

#endif
