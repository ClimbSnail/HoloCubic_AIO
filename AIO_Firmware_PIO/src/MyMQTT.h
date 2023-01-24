//
// Created by user on 2023/1/20.
//
//
#include"network.h"
#include"PubSubClient.h"
#include"ArduinoJson.h"
#include"Wire.h"
#include"sys/app_controller.h"

#define SUCCESS 1
#define FAIL 0

//#ifndef HOLOCUBIC_AIO_TOMATO_MYMQTT_H
//#define HOLOCUBIC_AIO_TOMATO_MYMQTT_H
//
//#endif //HOLOCUBIC_AIO_TOMATO_MYMQTT_H
//
//
//
//mqtt服务接口地址
#define mqttServer "067***.st1.iotda-device.cn-north-4.myhuaweicloud.com"
//mqtt服务端口
#define mqttPort 1883

//设备id
#define clientId "63ca8************"

//mqtt用户名密码可使用华为云的工具生成，自行百度
#define mqttUser "63ca876**********"
#define mqttPassword "cc2d1d3510e651f9*************1917d257552831b411cf93ad068afe"

//属性上报topic
//#define topic_properties_report  "$oc/devices/63ca876f352830580e465fef_holo_chen/sys/messages/up"
#define topic_Commands_Response "$oc/devices/{device_id}/sys/commands/response/request_id="
#define topic_Commands "$oc/devices/{device_id}/sys/commands/#"
#define topic_Properties "$oc/devices/{device_id}/sys/messages/down"

class MyMQTT{
private:

    WiFiClient wiFiClient;

public:

    PubSubClient client;
    MyMQTT(const char *sid, const char *password, std::function<void(char *, uint8_t *, unsigned int)> callback);
    void loop();
    boolean sendPropertiesReport(char *payload);
};


