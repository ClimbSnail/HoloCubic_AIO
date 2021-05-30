#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>

// 时区偏移(小时) 8*60*60
#define TIMEZERO_OFFSIZE (28800)

#define CONN_SUCC 0
#define CONN_ERROR 1
#define CONN_ERR_TIMEOUT 15 // 连接WiFi的超时时间（s）

// wifi是否连接标志
#define AP_DISABLE 0
#define AP_ENABLE 1

struct Weather
{
	int weather_code;
	int temperature;
};

void restCallback(TimerHandle_t xTimer);

class Network
{
public:
	boolean ap_status;		   // 为了保证尝试连接过程中正常执行
	TimerHandle_t xTimer_ap;

public:
	Network();
	void init(String ssid, String password);
	void search_wifi(void);
	boolean start_conn_wifi(const char *ssid, const char *password);
	boolean end_conn_wifi(void);
	boolean open_ap(const char *ap_ssid, const char *ap_password);
	wl_status_t get_wifi_sta_status(void);
	long int getTimestamp(String url);
	Weather getWeather(String url);
};

#endif
