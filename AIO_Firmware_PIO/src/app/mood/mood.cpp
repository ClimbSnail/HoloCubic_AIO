#include "mood.h"
#include "mood_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "app/media_player/docoder.h"
#include "driver/sd_card.h"
#include "app/media_player/DMADrawer.h"



#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOOD_PATH "/mood"

#define MOOD_MAX 6

//文件名称的字符串数组
const char *mood_file_name[6] = {
        "bukaixin.mjpeg",
        "buxie.mjpeg",
        "shangxin.mjpeg",
        "shengqi.mjpeg",
        "tantou.mjpeg",
        "zhenjing.mjpeg",
};



// 动态数据，APP的生命周期结束也需要释放它
struct MoodAppRunData
{
    int mood_index = 0;
    PlayDocoderBase *player_decoder;
    File_Info mood_file_info;
    char *mood_file_path;
    char *file_name;
    File mood_file;
    //上一个mood标志位;
    int pre_index = -1;
};

// 保存APP运行时的参数信息，理论上关闭APP时推荐在 xxx_exit_callback 中释放掉
static MoodAppRunData *run_data = NULL;
AppController *appController = NULL;


// 释放播放器对象
static void release_player_decoder()
{

    //关闭并释放文件
    if(run_data->mood_file){
        run_data->mood_file.close();
    }

    if (run_data->player_decoder)
    {
        delete run_data->player_decoder;
        run_data->player_decoder = NULL;
    }
}




static int mood_init(AppController *sys)
{


    appController = sys;

    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.20, 0.001, 50};
    set_rgb_and_run(&rgb_setting);

    run_data = (MoodAppRunData *)calloc(1, sizeof(MoodAppRunData));

    //申请内存
    run_data->file_name = (char *)calloc(1, 256);
    run_data->mood_file_path = (char *)calloc(1, 256);
    run_data->player_decoder = NULL;
    run_data->mood_index = appController->sys_cfg.current_mood;
    run_data->pre_index = run_data->mood_index;


    setCpuFrequencyMhz(240);

    update_mood();

    return 0;
}


//更新心情
static int update_mood(){

    //防止越界
    if (run_data->mood_index < 0 || run_data->mood_index >= MOOD_MAX)
    {
        return -1;
    }

    strcpy(run_data->file_name,mood_file_name[run_data->mood_index]);

    //将MOOD_PATH和文件名拼接
    sprintf(run_data->mood_file_path, "%s/%s", MOOD_PATH, run_data->file_name);
    Serial.println(run_data->mood_file_path);


    //根据文件名获取文件信息
    run_data->mood_file = SD.open(run_data->mood_file_path);


    if (!run_data->mood_file)
    {
        Serial.println("没有该文件");
        return -1;
    }


    video_start();
    Serial.println(run_data->mood_file.name());
    Serial.println(run_data->mood_file.size());

    return 0;

}


static void video_start(){


    if(run_data->player_decoder!=NULL){
        free(run_data->player_decoder);
    }

    if(NULL != strstr(run_data->file_name,".mjpeg")){
        run_data->player_decoder = new MjpegPlayDocoder(&run_data->mood_file, true);
        Serial.println("mjpeg开始播放");
    }
    else if(NULL != strstr(run_data->file_name,"rgb")){
        run_data->player_decoder = new RgbPlayDocoder(&run_data->mood_file, true);
        Serial.println("rgb开始播放");
    }
}



static void mood_process(AppController *sys,
                            const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }


    run_data->mood_index = appController->sys_cfg.current_mood;

    //发现变化，开始更新心情
    if (run_data->mood_index != run_data->pre_index){

        //释放原先的播放器对象，防止出现内存溢出导致卡死
        release_player_decoder();

        update_mood();
        run_data->pre_index = run_data->mood_index;
    }



//    // 主频控制 为了降低发热量,如果卡顿可以注释掉
//    if (getCpuFrequencyMhz() > 80)
//    {
//        if (getCpuFrequencyMhz() > 160 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_160M)
//        {
//            // 设置CPU主频
//            setCpuFrequencyMhz(160);
//        }
//        else if (getCpuFrequencyMhz() > 80 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_80M)
//        {
//            setCpuFrequencyMhz(80);
//        }
//    }

    if (NULL == run_data->file_name)
    {
        sys->app_exit(); // 退出APP
        Serial.println("没有该文件");
        return;
    }

    if (run_data->mood_file.available() && run_data->player_decoder != NULL){

        run_data->player_decoder->video_play_screen();
    }else{
        Serial.println("播放失败或结束");
        //从头播放，不必销毁
        run_data->mood_file.seek(0);
    }

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 程序需要时可以适当加延时
    // delay(300);
}

static void mood_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放

    // 发送请求。如果是wifi相关的消息，当请求完成后自动会调用 example_message_handle 函数
    // sys->send_to(EXAMPLE_APP_NAME, CTRL_NAME,
    //              APP_MESSAGE_WIFI_CONN, (void *)run_data->val1, NULL);

    // 也可以移除自身的后台任务，放在本APP可控的地方最合适
    // sys->remove_backgroud_task();

    // 程序需要时可以适当加延时
    // delay(300);
}

static int mood_exit_callback(void *param)
{

    //TODO 向MQTT发送退出消息
    release_player_decoder();

    // 释放资源
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }

    Serial.println("成功释放资源！！！");

    return 0;
}

static void mood_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    // 目前主要是wifi开关类事件（用于功耗控制）
    switch (type)
    {
    case APP_MESSAGE_WIFI_CONN:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_AP:
    {
        // todo
    }
    break;
    case APP_MESSAGE_WIFI_ALIVE:
    {
        // wifi心跳维持的响应 可以不做任何处理
    }
    break;
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
    }
    break;
    default:
        break;
    }
}

APP_OBJ mood_app = {MOOD_APP_NAME, &app_mood, "我的心情，由MQTT召唤此应用",
                       mood_init, mood_process, mood_background_task,
                       mood_exit_callback, mood_message_handle};
