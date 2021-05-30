#include "network.h"

#include <HTTPClient.h>
#include <ESPmDNS.h>


uint8_t ap_time_out = 0; // ap无连接的超时时间

Network::Network()
{
	ap_status = AP_DISABLE;
}

void Network::init(String ssid, String password)
{
	search_wifi();
	start_conn_wifi(ssid.c_str(), password.c_str());
}

void Network::search_wifi(void)
{
	Serial.println("scan start");
	int wifi_num = WiFi.scanNetworks();
	Serial.println("scan done");
	if (0 == wifi_num)
	{
		Serial.println("no networks found");
	}
	else
	{
		Serial.print(wifi_num);
		Serial.println(" networks found");
		for (int cnt = 0; cnt < wifi_num; ++cnt)
		{
			Serial.print(cnt + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(cnt));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(cnt));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(cnt) == WIFI_AUTH_OPEN) ? " " : "*");
			delay(10); // todo
		}
	}
}

boolean Network::start_conn_wifi(const char *ssid, const char *password)
{
	ap_status = AP_DISABLE; // 关闭AP运行使能
	Serial.println("");
	Serial.print("Connecting: ");
	Serial.print(ssid);
	Serial.print(" @");
	Serial.println(password);

	//设置为STA模式并连接WIFI
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	// 尝试连接计数器
	// int try_cnt = 0;
	// int max_err_cnt = CONN_ERR_TIMEOUT / 100; //最大的错误连接次数
	// while (WiFi.status() != WL_CONNECTED)
	// {
	// 	delay(100);
	// 	++try_cnt;
	// 	Serial.println(try_cnt);
	// 	if (try_cnt >= max_err_cnt)
	// 	{
	// 		break;
	// 	}
	// }
	return 1;
}

boolean Network::end_conn_wifi(void)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		// 连接超时后 开启AP运行使能
		ap_status = AP_ENABLE;
		Serial.println("\nWiFi connect error.\n");
		return CONN_ERROR;
	}
	Serial.println("\nWiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	return CONN_SUCC;
}

boolean Network::open_ap(const char *ap_ssid, const char *ap_password)
{
	WiFi.mode(WIFI_AP);									//配置为AP模式
	boolean result = WiFi.softAP(ap_ssid, ap_password); //开启WIFI热点
	if (result)
	{
		IPAddress myIP = WiFi.softAPIP();

		//打印相关信息
		Serial.println("");
		Serial.print("Soft-AP IP address = ");
		Serial.println(myIP);
		Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());
		Serial.println("waiting ...");
		ap_time_out = 100;	// 开始计时
		xTimer_ap = xTimerCreate("ap time out", 1000 / portTICK_PERIOD_MS, pdTRUE, (void *)0, restCallback);
		xTimerStart(xTimer_ap, 0); //开启定时器
	}
	else
	{ //开启热点失败
		Serial.println("WiFiAP Failed");
		delay(3000);
		ESP.restart(); //复位esp32
	}
	if (MDNS.begin("esp32"))
	{
		Serial.println("MDNS responder started");
	}
	return 1;
}

wl_status_t Network::get_wifi_sta_status(void)
{
	return WiFi.status();
}

long int Network::getTimestamp(String url)
{
	if(WL_CONNECTED != get_wifi_sta_status())
		return 0;
	
	String time = "";
	HTTPClient http;
	http.begin(url);

	// start connection and send HTTP headerFFF
	int httpCode = http.GET();

	// httpCode will be negative on error
	if (httpCode > 0)
	{
		// file found at server
		if (httpCode == HTTP_CODE_OK)
		{
			String payload = http.getString();
			int time_index = (payload.indexOf("data")) + 12;
			time = payload.substring(time_index, payload.length() - 6);
			Serial.println(time);
		}
	}
	else
	{
		Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	}
	http.end();

	return atoll(time.c_str());
}

Weather Network::getWeather(String url)
{	
	Weather ret_weather;
	if(WL_CONNECTED != get_wifi_sta_status())
		return ret_weather;

	HTTPClient https;
	https.begin(url);

	// start connection and send HTTP headerFFF
	int httpCode = https.GET();

	// httpCode will be negative on error
	if (httpCode > 0)
	{
		// file found at server
		if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
		{
			String payload = https.getString();
			Serial.println(httpCode);
			Serial.println(payload);
			int code_index = (payload.indexOf("code")) + 7;			//获取code位置
			int temp_index = (payload.indexOf("temperature")) + 14; //获取temperature位置
			ret_weather.weather_code =
				atol(payload.substring(code_index, temp_index - 17).c_str());
			ret_weather.temperature =
				atol(payload.substring(temp_index, payload.length() - 47).c_str());
		}
	}
	else
	{
		Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
	}
	https.end();

	return ret_weather;
}

void restCallback(TimerHandle_t xTimer)
{ //长时间不访问WIFI Config 将复位设备

    --ap_time_out;
    Serial.print("ap time out: ");
    Serial.println(ap_time_out);
    if (ap_time_out < 1)
    {
        WiFi.softAPdisconnect(true);
        // ESP.restart();
    }
}