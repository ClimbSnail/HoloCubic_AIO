#ifndef APP_CONTORLLER_H
#define APP_CONTORLLER_H

#include "Arduino.h"
#include "../sys/interface.h"
#include "imu.h"

#define APP_MAX_NUM 10 // 最大的可运行的APP数量

class AppController
{
public:
    AppController();
    ~AppController();
    int app_register(const APP_OBJ *app);   // 将APP注册到app_controller中
    int app_unregister(const APP_OBJ *app); // 将APP从app_controller中去注册（删除）
    int main_process(Imu_Action *act_info);
    void app_exit(void); // 提供给app退出的系统调用
    // 事件请求
    int req_event(const APP_OBJ *from, APP_EVENT event, int event_id);


private:
    int app_is_legal(const APP_OBJ *app_obj);

private:
    APP_OBJ *appList; // 预留10个APP注册位
    unsigned int app_num;
    boolean app_exit_flag; // 表示是否退出APP应用
    int cur_app_index;     // 当前运行的APP下标
    int pre_app_index;     // 上一次运行的APP下标
};

#endif