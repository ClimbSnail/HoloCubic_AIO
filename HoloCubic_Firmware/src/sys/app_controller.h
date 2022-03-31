#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "Arduino.h"
#include "interface.h"
#include "driver/imu.h"
#include "common.h"
#include <list>

#define CTRL_NAME "AppCtrl"
#define APP_MAX_NUM 15             // 最大的可运行的APP数量
#define WIFI_LIFE_CYCLE 60000      // wifi的生命周期（60s）
#define EVENT_LIST_MAX_LENGTH 10   // 消息队列的容量
#define APP_CONTROLLER_NAME_LEN 16 // app控制器的名字长度

// struct EVENT_OBJ
// {
//     const APP_OBJ *from;   // 发送请求服务的APP
//     const APP_OBJ *to;     // 接受请求服务的APP
//     APP_MESSAGE_TYPE type; // app的消息类型
//     unsigned int id;       // 发送请求服务的id
//     void *message;         // 附带数据，可以为任何数据类型
// };

struct EVENT_OBJ
{
    const APP_OBJ *from;       // 发送请求服务的APP
    APP_MESSAGE_TYPE type;     // app的事件类型
    void *info;                // 请求携带的信息
    uint8_t retryMaxNum;       // 重试次数
    uint8_t retryCount;        // 重试计数
    unsigned long nextRunTime; // 下次运行的时间戳
};

class AppController
{
public:
    AppController(const char *name = CTRL_NAME);
    ~AppController();
    void init(void);
    void Display(void);                    // 显示接口
    int app_install(APP_OBJ *app);         // 将APP注册到app_controller中
    int app_uninstall(const APP_OBJ *app); // 将APP从app_controller中卸载（删除）
    int main_process(ImuAction *act_info);
    void app_exit(void); // 提供给app退出的系统调用
    // 消息发送
    int send_to(const char *from, const char *to,
                APP_MESSAGE_TYPE type, void *message,
                void *ext_info);
    void deal_config(APP_MESSAGE_TYPE type,
                     const char *key, char *value);
    // 事件处理
    int req_event_deal(void);
    bool wifi_event(APP_MESSAGE_TYPE type); // wifi事件的处理
    void read_config(SysUtilConfig *cfg);
    void write_config(SysUtilConfig *cfg);
    void read_config(SysMpuConfig *cfg);
    void write_config(SysMpuConfig *cfg);
    void read_config(RgbConfig *cfg);
    void write_config(RgbConfig *cfg);

private:
    APP_OBJ *getAppByName(const char *name);
    int app_is_legal(const APP_OBJ *app_obj);

private:
    char name[APP_CONTROLLER_NAME_LEN]; // app控制器的名字
    APP_OBJ *appList[APP_MAX_NUM];      // 预留10个APP注册位
    // std::list<const APP_OBJ *> app_list; // APP注册位(为了C语言可移植，放弃使用链表)
    std::list<EVENT_OBJ> eventList;   // 用来储存事件
    boolean m_wifi_status;            // 表示是wifi状态 true开启 false关闭
    unsigned long m_preWifiReqMillis; // 保存上一回请求的时间戳
    unsigned int app_num;
    boolean app_exit_flag; // 表示是否退出APP应用
    int cur_app_index;     // 当前运行的APP下标
    int pre_app_index;     // 上一次运行的APP下标

public:
    SysUtilConfig sys_cfg;
    SysMpuConfig mpu_cfg;
    RgbConfig rgb_cfg;
};

#endif