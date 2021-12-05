#include "media_player.h"
#include "media_gui.h"
#include "sys/app_contorller.h"
#include "common.h"
#include "driver/sd_card.h"
#include "docoder.h"
#include "DMADrawer.h"

#include <SD.h>

#define VIDEO_WIDTH 240L
#define VIDEO_HEIGHT 240L
#define MOVIE_PATH "/movie"
#define NO_TRIGGER_ENTER_FREQ_160M 90000UL // 无操作规定时间后进入设置160M主频（90s）
#define NO_TRIGGER_ENTER_FREQ_80M 120000UL  // 无操作规定时间后进入设置160M主频（120s）

struct MediaAppRunData
{
    PlayDocoderBase *player_docoder;
    unsigned long preTriggerKeyMillis; // 最近一回按键触发的时间戳
    int movie_pos_increate;
    File_Info *movie_file; // movie文件夹下的文件指针头
    File_Info *pfile;      // 指向当前播放的文件节点
    File file;
};

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

bool video_start(bool create_new)
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

    run_data->file = SD.open(file_name);
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

void release_player_docoder(void)
{
    // 释放具体的播放对象
    if (NULL != run_data->player_docoder)
    {
        delete run_data->player_docoder;
        run_data->player_docoder = NULL;
    }
}

void media_player_init(void)
{
    // 调整RGB模式  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV, 0, 128, 32,
                            255, 255, 32,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb(&rgb_setting);

    // 初始化运行时参数
    // run_data = (MediaAppRunData *)malloc(sizeof(MediaAppRunData));
    // memset(run_data, 0, sizeof(MediaAppRunData));
    run_data = (MediaAppRunData *)calloc(1, sizeof(MediaAppRunData));
    run_data->player_docoder = NULL;
    run_data->movie_pos_increate = 1;
    run_data->movie_file = NULL; // movie文件夹下的文件指针头
    run_data->pfile = NULL;      // 指向当前播放的文件节点
    run_data->preTriggerKeyMillis = millis();

    run_data->movie_file = tf.listDir(MOVIE_PATH);
    if (NULL != run_data->movie_file)
    {
        run_data->pfile = get_next_file(run_data->movie_file->next_node, 1);
    }

    // 设置CPU主频
    setCpuFrequencyMhz(240);

    // 创建播放
    video_start(false);
}

void media_player_process(AppController *sys,
                          const Imu_Action *act_info)
{
    if (RETURN == act_info->active)
    {
        sys->app_exit(); // 退出APP
        return;
    }
    else if (UNKNOWN != act_info->active)
    {
        // 记录下操作的时间点
        run_data->preTriggerKeyMillis = millis();
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
    }

    if (NULL == run_data->pfile)
    {
        // 不存在可以播放的文件
        sys->app_exit(); // 退出APP
        return;
    }

    // 主频控制 为了降低发热量
    if (getCpuFrequencyMhz() > 80)
    {
        if (getCpuFrequencyMhz() > 160 && millis() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_160M)
        {
            // 设置CPU主频
            setCpuFrequencyMhz(160);
        }
        else if (getCpuFrequencyMhz() > 80 && millis() - run_data->preTriggerKeyMillis >= NO_TRIGGER_ENTER_FREQ_80M)
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
        // 创建播放(重复播放)
        video_start(false);
    }
}

void media_player_exit_callback(void)
{
    // 结束播放
    release_player_docoder();

    run_data->file.close(); // 退出时关闭文件
    // 释放文件循环队列
    release_file_info(run_data->movie_file);

    free(run_data);
    run_data = NULL;

    // 恢复RGB灯  HSV色彩模式
    RgbParam rgb_setting = {LED_MODE_HSV,
                            1, 32, 255,
                            255, 255, 255,
                            1, 1, 1,
                            0.15, 0.25, 0.001, 30};
    set_rgb(&rgb_setting);
}

void media_player_event_notification(APP_EVENT_TYPE type, int event_id)
{
}

APP_OBJ media_app = {"Media", &app_movie, "", media_player_init,
                     media_player_process, media_player_exit_callback,
                     media_player_event_notification};
