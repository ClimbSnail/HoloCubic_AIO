#include "bilibili.h"
#include "bilibili_gui.h"
#include "../sys/app_contorller.h"
#include "../../network.h"
#include "../../common.h"

#include <ArduinoJson.h>

//String UID = "344470052";
//String Url = "http://api.bilibili.com/x/relation/stat?vmid=" + UID;   // 粉丝数
//String Url = "http://api.bilibili.com/x/web-interface/card?mid=" + UID;   // 粉丝数
const char* host = "api.bilibili.com";

/******************************修改部分********************************************/

void bilibili_init(void){
    Serial.println("B站粉丝获取开始");

    //创建TCP连接
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)){
        Serial.println("Connection failed");  //网络请求无响应打印连接失败
        return;
    }
    //URL请求地址
    String url = "/x/web-interface/card?mid=" + g_cfg.UID + "&jsonp=jsonp"; // B站粉丝数（新）
    //String url = "/x/relation/stat?vmid=" + g_cfg.UID + "&jsonp=jsonp"; // B站粉丝数（旧）

    //发送网络请求
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
       "Host: " + host + "\r\n" +
       "Connection: close\r\n\r\n");
    delay(2000);
    //定义answer变量用来存放请求网络服务器后返回的数据
    String answer;
    while(client.available()){
        String line = client.readStringUntil('\r');
        answer += line;
    }
    //断开服务器连接
    client.stop();
    Serial.println();
    Serial.println("closing connection");

    /**************************json数据解析**********************/
    String jsonAnswer;
    int jsonIndex;
    //找到有用的返回数据位置i 返回头不要
    for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
            jsonIndex = i;
            break;
        }
    }
    jsonAnswer = answer.substring(jsonIndex);
    Serial.println();
    Serial.println("JSON answer: ");
    Serial.println(jsonAnswer); 
    StaticJsonDocument<2048> doc;
    deserializeJson(doc, jsonAnswer);
    JsonObject data = doc["data"];
    JsonObject data_card = data["card"];
    const char* data_card_mid = data_card["mid"]; // "344470052"
    const char* data_card_name = data_card["name"]; // "溜马小哥"
    const char* data_card_face = data_card["face"]; //头像地址
    int data_card_fan = data_card["fans"]; // 579 粉丝数
    int data_card_friend = data_card["friend"]; // 关注人数
    int data_card_attention = data_card["attention"]; // 112
    const char* data_card_sign = data_card["sign"]; // "骑着蜗牛也要冲"
    int data_follower = data["follower"]; // 579 粉丝数

    
    Serial.println("fan: "); //粉丝数
    Serial.println(data_card_fan);
    
    Serial.println("name: ");//用户昵称
    Serial.println(data_card_name);

    Serial.println("face: ");//头像地址
    Serial.println(data_card_face);

    Serial.println("data_card_friend: ");//关注人数
    Serial.println(data_card_friend);

    Serial.println("data_card_sign: ");//个性签名
    Serial.println(data_card_sign);

    char data_card_fans[10] = {0};
    char data_card_friends[10] = {0};
    sprintf(data_card_fans, "%d", data_card_fan);
    sprintf(data_card_friends, "%d", data_card_friend);
    bilibili_gui_init(data_card_fans , data_card_name , 
                    data_card_face , data_card_friends , 
                    data_card_sign);

    /****************************************************/
}

void bilibili_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        Serial.println("123");
        sys->app_exit(); // 退出APP
        return;
    }
    // delay(300);
}

void bilibili_exit_callback(void){
    Serial.println("B站退出");
    bilibili_gui_del();
}

void bilibili_event_notification(APP_EVENT event){
    

}

APP_OBJ bilibili_app = {"Bilibili", &app_bilibili, bilibili_init,
                     bilibili_process, bilibili_exit_callback,
                     bilibili_event_notification};
