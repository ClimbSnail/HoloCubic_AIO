#ifndef NETWORK_H
#define NETWORK_H

// 时区偏移(小时) 8*60*60
#define TIMEZERO_OFFSIZE (28800)

#define CONN_SUCC 0
#define CONN_ERROR 1
#define CONN_ERR_TIMEOUT 15 // 连接WiFi的超时时间（s）

// wifi是否连接标志
#define AP_DISABLE 0
#define AP_ENABLE 1
// Set your server's logical name here e.g. if 'myserver' then address is http://myserver.local/
#define SERVER_NAME "fileserver"

#ifdef ESP8266
#include <ESP8266WiFi.h>	  // Built-in
#include <ESP8266WiFiMulti.h> // Built-in
#include <ESP8266WebServer.h> // Built-in
#include <ESP8266mDNS.h>
extern ESP8266WebServer server;
#else
#include <WiFi.h>	   // Built-in
#include <WiFiMulti.h> // Built-in
#include <WebServer.h> // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>
#include <HTTPClient.h>
extern WebServer server;
#endif

// #ifdef ESP8266
// //ESP8266WiFiMulti wifiMulti;
// extern ESP8266WebServer server;
// #else
// //WiFiMulti wifiMulti;
// extern WebServer server;
// #endif


extern IPAddress local_ip; // Set your server's fixed IP address here
extern IPAddress gateway;	  // Set your network Gateway usually your Router base address
extern IPAddress subnet;	  // Set your network sub-network mask here
extern IPAddress dns;		  // Set your network DNS usually your Router base address

extern const char *AP_SSID; //热点名称
extern const char *AP_PASS;      //密码

struct Weather
{
	int weather_code;
	int temperature;
};

void restCallback(TimerHandle_t xTimer);

class Network
{
public:
	unsigned long m_preWifiClickMillis;	// 保存上一回的时间戳
	unsigned long m_wifiClickInterval = 15000;	// 日期时钟更新的时间间隔
	long int m_preNetTimestamp = 0;   // 上一次的网络时间戳
	long int m_preLocalTimestamp = 0;   // 上一次的本地机器时间戳
	Weather m_weather;	// 保存天气状况

public:
	Network();
	void init(String ssid, String password);
	void search_wifi(void);
	String get_localIp(void);
	String get_softAPIP(void);
	static boolean start_conn_wifi(const char *ssid, const char *password);
	boolean end_conn_wifi(void);
	boolean open_ap(const char *ap_ssid = AP_SSID, const char *ap_password = AP_PASS);
	wl_status_t get_wifi_sta_status(void);
	long int getTimestamp(String url);
	Weather getWeather(String url);
	void start_web_config(void);
	void stop_web_config(void);
	boolean getApStatus(void);
};

#endif
