#include "app_controller.h"
#include "app_controller_gui.h"
#include "common.h"
#include "interface.h"
#include "Arduino.h"

const char *app_event_type_info[] = {"APP_MESSAGE_WIFI_CONN", "APP_MESSAGE_WIFI_AP",
                                     "APP_MESSAGE_WIFI_ALIVE", "APP_MESSAGE_WIFI_DISCONN",
                                     "APP_MESSAGE_UPDATE_TIME", "APP_MESSAGE_MQTT_DATA",
                                     "APP_MESSAGE_GET_PARAM", "APP_MESSAGE_SET_PARAM",
                                     "APP_MESSAGE_READ_CFG", "APP_MESSAGE_WRITE_CFG",
                                     "APP_MESSAGE_NONE"};

volatile static bool isRunEventDeal = false;

// TickType_t mainFormRefreshLastTime;
// const TickType_t xDelay500ms = pdMS_TO_TICKS(500);
// mainFormRefreshLastTime = xTaskGetTickCount();
// vTaskDelayUntil(&mainFormRefreshLastTime, xDelay500ms);

void eventDealHandle(TimerHandle_t xTimer)
{
    isRunEventDeal = true;
}

AppController::AppController(const char *name)
{
    strncpy(this->name, name, APP_CONTROLLER_NAME_LEN);
    app_num = 0;
    app_exit_flag = 0;
    cur_app_index = 0;
//    pre_app_index = 0;
    // appList = new APP_OBJ[APP_MAX_NUM];
    app_stack_top = -1;
    m_wifi_status = false;
    m_preWifiReqMillis = GET_SYS_MILLIS();

    // 定义一个事件处理定时器
    xTimerEventDeal = xTimerCreate("Event Deal",
                                   300 / portTICK_PERIOD_MS,
                                   pdTRUE, (void *)0, eventDealHandle);
    // 启动事件处理定时器
    xTimerStart(xTimerEventDeal, 0);
}

void AppController::init(void)
{
    // 设置CPU主频
    if (1 == this->sys_cfg.power_mode)
    {
        setCpuFrequencyMhz(240);
    }
    else
    {
        setCpuFrequencyMhz(80);
    }
    // uint32_t freq = getXtalFrequencyMhz(); // In MHz
    Serial.print(F("CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());

    app_control_gui_init();
    appList[0] = new APP_OBJ();
    appList[0]->app_image = &app_loading;
    appList[0]->app_name = "Loading...";
    appTypeList[0] = APP_TYPE_REAL_TIME;
    app_control_display_scr(appList[cur_app_index]->app_image,
                            appList[cur_app_index]->app_name,
                            LV_SCR_LOAD_ANIM_NONE, true);
    // Display();
}

void AppController::Display()
{
    // appList[0].app_image = &app_loading;
    app_control_display_scr(appList[cur_app_index]->app_image,
                            appList[cur_app_index]->app_name,
                            LV_SCR_LOAD_ANIM_NONE, true);
}

AppController::~AppController()
{
    rgb_stop();
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

// 将APP安装到app_controller中
int AppController::app_install(APP_OBJ *app, APP_TYPE app_type)
{
    int ret_code = app_is_legal(app);
    if (0 != ret_code)
    {
        return ret_code;
    }

    appList[app_num] = app;
    appTypeList[app_num] = app_type;
    ++app_num;
    return 0; // 安装成功
}

// 将APP的后台任务从任务队列中移除(自能通过APP退出的时候，移除自身的后台任务)
int AppController::remove_backgroud_task(void)
{
    return 0; // 安装成功
}

// 将APP从app_controller中卸载（删除）
int AppController::app_uninstall(const APP_OBJ *app)
{
    // todo
    return 0;
}



int AppController::app_auto_start()
{
    // APP自启动
    int index = this->getAppIdxByName(sys_cfg.auto_start_app.c_str());
    if (index < 0)
    {
        // 没找到相关的APP
        return 0;
    }
    // 进入自启动的APP
    app_exit_flag = 1; // 进入app, 如果已经在
    cur_app_index = index;
    (*(appList[cur_app_index]->app_init))(this); // 执行APP初始化
    return 0;
}

int AppController::main_process(ImuAction *act_info)
{
    if (ACTIVE_TYPE::UNKNOWN != act_info->active)
    {
        Serial.print(F("[Operate]\tact_info->active: "));
        Serial.println(active_type_info[act_info->active]);
    }

    if (isRunEventDeal)
    {
        isRunEventDeal = false;
        // 扫描事件
        this->req_event_deal();
    }

    // wifi自动关闭(在节能模式下)
    if (0 == sys_cfg.power_mode && true == m_wifi_status && doDelayMillisTime(WIFI_LIFE_CYCLE, &m_preWifiReqMillis, false))
    {
        send_to(CTRL_NAME, CTRL_NAME, APP_MESSAGE_WIFI_DISCONN, 0, NULL);
    }

    if (0 == app_exit_flag)
    {
        // 当前没有进入任何app
        lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE;
        if (ACTIVE_TYPE::TURN_LEFT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_RIGHT;
//            pre_app_index = cur_app_index;
            cur_app_index = (cur_app_index + 1) % app_num;
            Serial.println(String("Current App: ") + appList[cur_app_index]->app_name);
        }
        else if (ACTIVE_TYPE::TURN_RIGHT == act_info->active)
        {
            anim_type = LV_SCR_LOAD_ANIM_MOVE_LEFT;
//            pre_app_index = cur_app_index;
            // 以下等效与 processId = (processId - 1 + APP_NUM) % 4;
            // +3为了不让数据溢出成负数，而导致取模逻辑错误
            cur_app_index = (cur_app_index - 1 + app_num) % app_num; // 此处的3与p_processList的长度一致
            Serial.println(String("Current App: ") + appList[cur_app_index]->app_name);
        }
        else if (ACTIVE_TYPE::GO_FORWORD == act_info->active)
        {
            app_start(cur_app_index);
//            app_exit_flag = 1; // 进入app
//            if (NULL != appList[cur_app_index]->app_init)
//            {
//                (*(appList[cur_app_index]->app_init))(this); // 执行APP初始化
//            }
        }

        if (ACTIVE_TYPE::GO_FORWORD != act_info->active) // && UNKNOWN != act_info->active
        {
            app_control_display_scr(appList[cur_app_index]->app_image,
                                    appList[cur_app_index]->app_name,
                                    anim_type, false);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        //不用加载图标了，浪费CPU时间，会很卡顿
//        app_control_display_scr(appList[cur_app_index]->app_image,
//                                    appList[cur_app_index]->app_name,
//                                    LV_SCR_LOAD_ANIM_NONE, false);
        // 运行APP进程 等效于把控制权交给当前APP
//        (*(appList[cur_app_index]->main_process))(this, act_info);



        //只运行栈顶进程
        (*(app_stack[app_stack_top]->main_process))(this, act_info);
    }
    act_info->active = ACTIVE_TYPE::UNKNOWN;
    act_info->isValid = 0;
    return 0;
}

APP_OBJ *AppController::getAppByName(const char *name)
{
    for (int pos = 0; pos < app_num; ++pos)
    {
        if (!strcmp(name, appList[pos]->app_name))
        {
            return appList[pos];
        }
    }

    return NULL;
}

int AppController::getAppIdxByName(const char *name)
{
    for (int pos = 0; pos < app_num; ++pos)
    {
        if (!strcmp(name, appList[pos]->app_name))
        {
            return pos;
        }
    }

    return -1;
}

// 通信中心（消息转发）
int AppController::send_to(const char *from, const char *to,
                           APP_MESSAGE_TYPE type, void *message,
                           void *ext_info)
{
    APP_OBJ *fromApp = getAppByName(from); // 来自谁 有可能为空
    APP_OBJ *toApp = getAppByName(to);     // 发送给谁 有可能为空
    if (type <= APP_MESSAGE_MQTT_DATA)
    {
        // 更新事件的请求者
        if (eventList.size() > EVENT_LIST_MAX_LENGTH)
        {
            return 1;
        }
        // 发给控制器的消息(目前都是wifi事件)
        EVENT_OBJ new_event = {fromApp, type, message, 3, 0, 0};
        eventList.push_back(new_event);
        Serial.print("[EVENT]\tAdd -> " + String(app_event_type_info[type]));
        Serial.print(F("\tEventList Size: "));
        Serial.println(eventList.size());
    }
    else
    {
        // 各个APP之间通信的消息
        if (NULL != toApp)
        {
            Serial.print("[Massage]\tFrom " + String(fromApp->app_name) + "\tTo " + String(toApp->app_name) + "\n");
            if (NULL != toApp->message_handle)
            {
                toApp->message_handle(from, to, type, message, ext_info);
            }
        }
        else if (!strcmp(to, CTRL_NAME))
        {
            Serial.print("[Massage]\tFrom " + String(fromApp->app_name) + "\tTo " + CTRL_NAME + "\n");
            deal_config(type, (const char *)message, (char *)ext_info);
        }
    }
    return 0;
}

int AppController::req_event_deal(void)
{
    // 请求事件的处理
    for (std::list<EVENT_OBJ>::iterator event = eventList.begin(); event != eventList.end();)
    {


        if ((*event).nextRunTime > GET_SYS_MILLIS())
        {
            ++event;
            continue;
        }
        // 后期可以拓展其他事件的处理
        bool ret = wifi_event((*event).type);
        if (false == ret)
        {
            // 本事件没处理完成
            (*event).retryCount += 1;
            if ((*event).retryCount >= (*event).retryMaxNum)
            {


                Serial.println("尝试事件");

                // 多次重试失败
                Serial.print("[EVENT]\tDelete -> " + String(app_event_type_info[(*event).type]));
                event = eventList.erase(event); // 删除该响应事件
                Serial.print(F("\tEventList Size: "));
                Serial.println(eventList.size());
            }
            else
            {
                // 下次重试
                (*event).nextRunTime = GET_SYS_MILLIS() + 4000;
                ++event;
            }
            continue;
        }



        // 事件回调
        if (NULL != (*event).from && NULL != (*event).from->message_handle)
        {


            Serial.println("事件回调");
            (*((*event).from->message_handle))(CTRL_NAME, (*event).from->app_name,
                                               (*event).type, (*event).info, NULL);
        }

        Serial.println("完成事件1");

        Serial.print("[EVENT]\tDelete -> " + String(app_event_type_info[(*event).type]));
        event = eventList.erase(event); // 删除该响应完成的事件
        Serial.print(F("\tEventList Size: "));
        Serial.println(eventList.size());



    }
    return 0;
}

/**
 *  wifi事件的处理
 *  事件处理成功返回true 否则false
 * */
bool AppController::wifi_event(APP_MESSAGE_TYPE type)
{
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // 更新请求
        // CONN_ERROR == g_network.end_conn_wifi() ||
        if (false == m_wifi_status)
        {
            g_network.start_conn_wifi(sys_cfg.ssid_0.c_str(), sys_cfg.password_0.c_str());
            m_wifi_status = true;
        }
        m_preWifiReqMillis = GET_SYS_MILLIS();
        if ((WiFi.getMode() & WIFI_MODE_STA) == WIFI_MODE_STA && CONN_SUCC != g_network.end_conn_wifi())
        {
            // 在STA模式下 并且还没连接上wifi
            return false;
        }
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {



        // 更新请求
        g_network.open_ap(AP_SSID);

        Serial.println("kai成功");

        m_wifi_status = true;
        m_preWifiReqMillis = GET_SYS_MILLIS();
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi开关的心跳 持续收到心跳 wifi才不会被关闭
        m_wifi_status = true;
        // 更新请求
        m_preWifiReqMillis = GET_SYS_MILLIS();
    }
    break;
    case APP_MESSAGE_WIFI_DISCONN:
    {
        g_network.close_wifi();
        m_wifi_status = false; // 标志位
        // m_preWifiReqMillis = GET_SYS_MILLIS() - WIFI_LIFE_CYCLE;
    }
    break;
    case APP_MESSAGE_UPDATE_TIME:
    {
    }
    break;
    case APP_MESSAGE_MQTT_DATA:
    {
        Serial.println("APP_MESSAGE_MQTT_DATA");
        if (app_exit_flag == 1 && cur_app_index != getAppIdxByName("Heartbeat")) // 在其他app中
        {
            app_exit_flag = 0;
            (*(appList[cur_app_index]->exit_callback))(NULL); // 退出当前app
        }
        if (app_exit_flag == 0)
        {
            app_exit_flag = 1; // 进入app, 如果已经在
            cur_app_index = getAppIdxByName("Heartbeat");
            (*(getAppByName("Heartbeat")->app_init))(this); // 执行APP初始化
        }
    }
    break;
    default:
        break;
    }

    return true;
}

void AppController::app_exit()
{

    if (app_stack_top < 0)
    {
        // APP栈为空
        return;
    }

    //最后一个APP退出，就标志退出位
    if (app_stack_top == 0)
    {
        app_exit_flag = 0;
    }
//    app_exit_flag = 0; // 退出APP

    // 清空该对象的所有请求
    for (std::list<EVENT_OBJ>::iterator event = eventList.begin(); event != eventList.end();)
    {


//        if (appList[cur_app_index] == (*event).from)
//        {
//            event = eventList.erase(event); // 删除该响应事件
//        }
//        else
//        {
//            ++event;
//        }


        if (app_stack[app_stack_top] == (*event).from)
        {
            event = eventList.erase(event); // 删除该响应事件
        }
        else
        {
            ++event;
        }
    }

    if (NULL != app_stack[app_stack_top]->exit_callback)
    {
        // 执行APP退出回调
        (*(app_stack[app_stack_top]->exit_callback))(NULL);
    }

    //如果不是最后一个app，就不执行下面的操作

    //将控制权交给前一个APP
    app_stack_top--;

    if(app_stack_top != -1){

        Serial.println("开始唤起activate事件");

        if((app_stack[app_stack_top]->activate)!=NULL){

            //重新激活当前应用程序所需要执行的操作
            (*(app_stack[app_stack_top]->activate))(this);
        }
        Serial.println("结束唤起activate事件");

        return;
    }




    app_control_display_scr(appList[cur_app_index]->app_image,
                            appList[cur_app_index]->app_name,
                            LV_SCR_LOAD_ANIM_NONE, true);



    // 恢复RGB灯  HSV色彩模式
    RgbConfig *cfg = &rgb_cfg;
    RgbParam rgb_setting = {LED_MODE_HSV,
                            cfg->min_value_0, cfg->min_value_1, cfg->min_value_2,
                            cfg->max_value_0, cfg->max_value_1, cfg->max_value_2,
                            cfg->step_0, cfg->step_1, cfg->step_2,
                            cfg->min_brightness, cfg->max_brightness,
                            cfg->brightness_step, cfg->time};
    set_rgb_and_run(&rgb_setting);

    // 设置CPU主频
    if (1 == this->sys_cfg.power_mode)
    {
        setCpuFrequencyMhz(240);
    }
    else
    {
        setCpuFrequencyMhz(80);
    }
    Serial.print(F("CpuFrequencyMhz: "));
    Serial.println(getCpuFrequencyMhz());
}




int AppController::app_start(const char *app_name) {

    // APP自启动
    int index = this->getAppIdxByName(app_name);
    if (index < 0)
    {
        // 没找到相关的APP
        return -1;
    }

    app_start(index);
    return 0;
}

int AppController::app_start(int index) {

    //如果是第一个APP，就标志启动位

    if (app_stack_top == -1){
        app_exit_flag = 1;
    }


    //如果启动的是同一个app，那就不执行任何操作
    if(app_stack[app_stack_top] == appList[index]){
        //是同一个应用
        return -1;
    }

//    app_exit_flag = 1; // 进入app, 如果已经在


    if (app_stack_top >= APP_STACK_SIZE)
    {
        // APP栈溢出
        return -1;
    }


    //首次启动需谨慎
    if ( app_stack_top != -1 && (app_stack[app_stack_top]->suspend) != NULL){


        Serial.println("开始唤起suspend事件");
        //挂起当前应用要执行的操作
        (*(app_stack[app_stack_top]->suspend))(this);
        Serial.println("结束唤起suspend事件");
    }



    //将当前app的对象指针存进app栈中
    app_stack[++app_stack_top] = appList[index];

//    //因为是后台程序，这就不用退出了
//    cur_app_index = index;

    (*(app_stack[app_stack_top]->app_init))(this); // 执行APP初始化

    return 0;
}



int AppController::top_app_exit() {

    if (app_stack_top < 0)
    {
        // APP栈为空
        return -1;
    }

    //最后一个APP退出，就标志退出位
    if (app_stack_top == 0)
    {
        app_exit_flag = 0;
    }


    //退出当前app
    (*(app_stack[app_stack_top--]->exit_callback))(NULL);

    return 0;
}

void AppController::set_wifi_status(boolean status) {
    m_wifi_status = status;
}
