#include "app_contorller.h"
#include "app_contorller_gui.h"
#include "common.h"
#include "../sys/interface.h"
#include "Arduino.h"

AppController::AppController()
{
    app_num = 0;
    app_exit_flag = 0;
    cur_app_index = 0;
    pre_app_index = 0;
    appList = new APP_OBJ[APP_MAX_NUM];
    app_control_gui_init();
    appList[0].app_image = &app_loading;
    app_contorl_display_scr(appList[cur_app_index].app_image, LV_SCR_LOAD_ANIM_NONE, true);
}

AppController::~AppController()
{
}

int AppController::app_is_legal(const APP_OBJ *app_obj)
{
    // APP的合法性检测
    if (NULL == app_obj)
        return 1;
    if (APP_MAX_NUM <= app_num)
        return 2;
    return 0;
}

int AppController::app_register(const APP_OBJ *app) // 将APP注册到app_controller中
{
    int ret_code = app_is_legal(app);
    if (0 != ret_code)
    {
        return ret_code;
    }

    appList[app_num].app_image = app->app_image;
    appList[app_num].app_init = app->app_init;
    appList[app_num].main_process = app->main_process;
    appList[app_num].exit_callback = app->exit_callback;
    ++app_num;
    return 0; //注册成功
}

int AppController::app_unregister(const APP_OBJ *app) // 将APP从app_controller中去注册（删除）
{
    // todo
    return 0;
}

int AppController::main_process(Imu_Action *act_info)
{
    if (UNKNOWN != act_info->active)
    {
        Serial.print("act_info->active: ");
        Serial.println(act_info->active);
    }

    if (0 == app_exit_flag)
    {
        // 当前没有进入任何app
        lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
        if (TURN_RIGHT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
            pre_app_index = cur_app_index;
            cur_app_index = (cur_app_index + 1) % app_num;
        }
        else if (TURN_LEFT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
            pre_app_index = cur_app_index;
            // 以下等效与 processId = (processId - 1 + APP_NUM) % 4;
            // +3为了不让数据溢出成负数，而导致取模逻辑错误
            cur_app_index = (cur_app_index - 1 + app_num) % app_num; // 此处的3与p_processList的长度一致
        }
        else if (GO_FORWORD == act_info->active)
        {
            app_exit_flag = 1; // 进入app
            if (NULL != appList[cur_app_index].exit_callback)
            {
                (*(appList[cur_app_index].app_init))(); // 执行APP初始化
            }
        }
        app_contorl_display_scr(appList[cur_app_index].app_image, anim_type, false);
        delay(300);
    }
    else
    {
        // 运行APP进程 等效于把控制权交给当前APP
        (*(appList[cur_app_index].main_process))(this, act_info);
    }
    act_info->active = UNKNOWN;
    act_info->isValid = 0;
    return 0;
}

// 事件请求
int req_event(const APP_OBJ *from, APP_EVENT event, int event_id)
{
    return 0;
}

void AppController::app_exit()
{
    app_exit_flag = 0; // 退出APP
    if (NULL != appList[cur_app_index].exit_callback)
    {
        // 执行APP退出回调
        (*(appList[cur_app_index].exit_callback))();
    }
    app_contorl_display_scr(appList[cur_app_index].app_image, LV_SCR_LOAD_ANIM_NONE, true);
}