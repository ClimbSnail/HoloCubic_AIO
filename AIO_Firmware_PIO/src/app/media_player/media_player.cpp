#include "media_player.h"
#include "media_gui.h"
#include "sys/app_controller.h"
#include "common.h"
#include "driver/sd_card.h"
#include "docoder.h"
#include "DMADrawer.h"

#define MEDIA_PLAYER_APP_NAME "Media"

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOVIE_PATH "/movie"
#define NO_TRIGGER_ENTER_FREQ_160M 90000UL // 无操作规定时间后进入设置160M主频（90s）
#define NO_TRIGGER_ENTER_FREQ_80M 120000UL // 无操作规定时间后进入设置160M主频（120s）

// 天气的持久化配置
#define MEDIA_CONFIG_PATH "/media.cfg"
struct MP_Config
{
    uint8_t switchFlag; // 是否自动播放下一个（0不切换 1自动切换）
    uint8_t powerFlag;  // 功耗控制（0低发热 1性能优先）
};

static void write_config(MP_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->switchFlag);
    w_data += tmp;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%u\n", cfg->powerFlag);
    w_data += tmp;
    g_flashCfg.writeFile(MEDIA_CONFIG_PATH, w_data.c_str());
}

static void read_config(MP_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(MEDIA_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->switchFlag = 1; // 是否自动播放下一个（0不切换 1自动切换）
        cfg->powerFlag = 0;  // 功耗控制（0低发热 1性能优先）
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[2] = {0};
        analyseParam(info, 2, param);
        cfg->switchFlag = atol(param[0]);
        cfg->powerFlag = atol(param[1]);
    }
}

struct MediaAppRunData
{
    PlayDocoderBase *player_docoder;
    unsigned long preTriggerKeyMillis; // 最近一回按键触发的时间戳
    int movie_pos_increate;
    File_Info *movie_file; // movie文件夹下的文件指针头
    File_Info *pfile;      // 指向当前播放的文件节点
    File file;
};

static MP_Config cfg_data;
static MediaAppRunData *run_data = NULL;

static File_Info *get_next_file(File_Info *p_cur_file, int direction)
{
    // 得到 p_cur_file 的下一个 类型为FILE_TYPE_FILE 的文件（即下一个非文件夹文件）
    if (NULL == p_cur_file)
    {
        return NULL;
    }

    File_Info *pfile = direction == 1 ? p_cur_file->next_node : p_cur_file->front_node;
    while (pfile != p_cur_file)
    {
        if (FILE_TYPE_FILE == pfile->file_type)
        {
            break;
        }
        pfile = direction == 1 ? pfile->next_node : pfile->front_node;
    }
    return pfile;
}

static bool video_start(bool create_new)
{
    if (NULL == run_data->pfile)
    {
        // 视频文件夹空 就跳出去
        return false;
    }

    if (true == create_new)
    {
        run_data->pfile = get_next_file(run_data->pfile, run_data->movie_pos_increate);
    }

    char file_name[FILENAME_MAX_LEN] = {0};
    snprintf(file_name, FILENAME_MAX_LEN, "%s/%s", run_data->movie_file->file_name, run_data->pfile->file_name);

    run_data->file = tf.open(file_name);
    if (NULL != strstr(run_data->pfile->file_name, ".mjpeg") || NULL != strstr(run_data->pfile->file_name, ".MJPEG"))
    {
        // 直接解码mjpeg格式的视频
        run_data->player_docoder = new MjpegPlayDocoder(&run_data->file, true);
        Serial.print(F("MJPEG video start --------> "));
    }
    else if (NULL != strstr(run_data->pfile->file_name, ".rgb") || NULL != strstr(run_data->pfile->file_name, ".RGB"))
    {
        // 使用RGB格式的视频
        run_data->player_docoder = new RgbPlayDocoder(&run_data->file, true);
        Serial.print(F("RGB565 video start --------> "));
    }

    Serial.println(file_name);
    return true;
}

static void release_player_docoder(void)
{
    // 释放具体的播放对象
    if (NULL != run_data->player_docoder)
    {
        delete run_data->player_docoder;
        run_data->player_docoder = NULL;
    }
}

static int media_player_init(AppController *sys)
{
    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.20, 0.001, 50};
    set_rgb_and_run(&rgb_setting);

    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    // run_data = (MediaAppRunData *)malloc(sizeof(MediaAppRunData));
    // memset(run_data, 0, sizeof(MediaAppRunData));
    run_data = (MediaAppRunData *)calloc(1, sizeof(MediaAppRunData));
    run_data->player_docoder = NULL;
    run_data->movie_pos_increate = 1;
    run_data->movie_file = NULL; // movie文件夹下的文件指针头
    run_data->pfile = NULL;      // 指向当前播放的文件节点
    run_data->preTriggerKeyMillis = GET_SYS_MILLIS();

    run_data->movie_file = tf.listDir(MOVIE_PATH);
    if (NULL != run_data->movie_file)
    {
        run_data->pfile = get_next_file(run_data->movie_file->next_node, 1);
    }

    // 设置CPU主频
    setCpuFrequencyMhz(240);

    // 创建播放
    video_start(false);
    return 0;
}

static void media_player_process(AppController *sys,
                                 const ImuAction *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    else if (UNKNOWN != act_info->active)
    {
        // 记录下操作的时间点
        run_data->preTriggerKeyMillis = GET_SYS_MILLIS();
        // 设置CPU主频
        setCpuFrequencyMhz(240);
    }

    if (NULL == run_data->pfile)
    {
        Serial.println(F("Not Found File."));
        sys->app_exit(); // 退出APP
        return;
    }

    if (TURN_RIGHT == act_info->active || TURN_LEFT == act_info->active)
    {
        // 切换方向
        if (TURN_RIGHT == act_info->active)
        {
            run_data->movie_pos_increate = 1;
        }
        else if (TURN_LEFT == act_info->active)
        {
            run_data->movie_pos_increate = -1;
        }
        // 结束播放
        release_player_docoder();
        run_data->file.close(); // 尝试关闭文件

        // 创建播放
        video_start(true);
        vTaskDelay(400 / portTICK_PERIOD_MS); // 暂缓播放 避免手抖
    }

    if (NULL == run_data->pfile)
    {
        // 不存在可以播放的文件
        sys->app_exit(); // 退出APP
        return;
    }

    // 主频控制 为了降低发热量
    if (getCpuFrequencyMhz() > 80 && 0 == cfg_data.powerFlag)
    {
        if (getCpuFrequencyMhz() > 160 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_160M)
        {
            // 设置CPU主频
            setCpuFrequencyMhz(160);
        }
        else if (getCpuFrequencyMhz() > 80 && GET_SYS_MILLIS() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_80M)
        {
            setCpuFrequencyMhz(80);
        }
    }

    if (!run_data->file)
    {
        Serial.println(F("Failed to open file for reading"));
        return;
    }

    if (run_data->file.available())
    {
        // 播放一帧数据
        run_data->player_docoder->video_play_screen();
    }
    else
    {
        // 结束播放
        release_player_docoder();
        run_data->file.close();
        if (0 == cfg_data.switchFlag)
        {
            // 创建播放(重复播放)
            video_start(false);
        }
        else
        {
            // 创建播放(播放下一个)
            video_start(true);
        }
    }
}

static void media_player_background_task(AppController *sys,
                                         const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int media_player_exit_callback(void *param)
{
    // 结束播放
    release_player_docoder();

    run_data->file.close(); // 退出时关闭文件
    // 释放文件循环队列
    release_file_info(run_data->movie_file);

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }

    return 0;
}

static void media_player_message_handle(const char *from, const char *to,
                                        APP_MESSAGE_TYPE type, void *message,
                                        void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "switchFlag"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.switchFlag);
        }
        else if (!strcmp(param_key, "powerFlag"))
        {
            snprintf((char *)ext_info, 32, "%u", cfg_data.powerFlag);
        }
        else
        {
            snprintf((char *)ext_info, 32, "%s", "NULL");
        }
    }
    break;
    case APP_MESSAGE_SET_PARAM:
    {
        char *param_key = (char *)message;
        char *param_val = (char *)ext_info;
        if (!strcmp(param_key, "switchFlag"))
        {
            cfg_data.switchFlag = atol(param_val);
        }
        else if (!strcmp(param_key, "powerFlag"))
        {
            cfg_data.powerFlag = atol(param_val);
        }
    }
    break;
    case APP_MESSAGE_READ_CFG:
    {
        read_config(&cfg_data);
    }
    break;
    case APP_MESSAGE_WRITE_CFG:
    {
        write_config(&cfg_data);
    }
    break;
    default:
        break;
    }
}

APP_OBJ media_app = {MEDIA_PLAYER_APP_NAME, &app_movie, "",
                     media_player_init, media_player_process, media_player_background_task,
                     media_player_exit_callback, media_player_message_handle};
