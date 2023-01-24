//
// Created by user on 2023/1/20.
//
#include "MyMQTT.h"
#include "network.h"
#include "common.h"
// #include<sys/app_controller.h>


MyMQTT::MyMQTT(const char * sid, const char * password, MQTT_CALLBACK_SIGNATURE) {

    wiFiClient = WiFiClient();
    client = PubSubClient(wiFiClient);

//    //打开wifi
//    Network network = Network();


    g_network.start_conn_wifi(sid,password);
    g_network.open_ap();


    this->client.setServer(mqttServer,mqttPort);

    client.setCallback(callback);
//    client.setKeepAlive(60);

    Serial.println("尝试开始链接。。。");

    this->loop();


}


void MyMQTT::loop() {

    //内存再套循环太卡了
    if(!this->client.connected()){


        Serial.println("连接失败");

        if(client.connect(clientId,mqttUser,mqttPassword)){
            Serial.println("connected");
            boolean result = this->client.subscribe(topic_Commands);
            Serial.println(topic_Commands);
            Serial.println(result == 1 ? "订阅成功" : "订阅失败");
        }else{
            Serial.println("fail:");
            Serial.print(client.state());
//            delay(6000);
        }
    }
    this->client.loop();
    delay(5000);
}

boolean MyMQTT::sendPropertiesReport(char *payload) {

    //发送当前设备的某些属性


    return 0;
}
