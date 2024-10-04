#include "picture.h"
#include "picture_gui.h"
#include "sys/app_controller.h"
#include "common.h"

// Include the jpeg decoder library
#include <TJpg_Decoder.h>

#define PICTURE_APP_NAME "Picture"

// 相册的持久化配置
#define PICTURE_CONFIG_PATH "/picture.cfg"
struct PIC_Config
{
    unsigned long switchInterval; // 自动播放下一张的时间间隔 ms
};

static void write_config(PIC_Config *cfg)
{
    char tmp[16];
    // 将配置数据保存在文件中（持久化）
    String w_data;
    memset(tmp, 0, 16);
    snprintf(tmp, 16, "%lu\n", cfg->switchInterval);
    w_data += tmp;
    g_flashCfg.writeFile(PICTURE_CONFIG_PATH, w_data.c_str());
}

static void read_config(PIC_Config *cfg)
{
    // 如果有需要持久化配置文件 可以调用此函数将数据存在flash中
    // 配置文件名最好以APP名为开头 以".cfg"结尾，以免多个APP读取混乱
    char info[128] = {0};
    uint16_t size = g_flashCfg.readFile(PICTURE_CONFIG_PATH, (uint8_t *)info);
    info[size] = 0;
    if (size == 0)
    {
        // 默认值
        cfg->switchInterval = 10000; // 是否自动播放下一个（0不切换 默认10000毫秒）
        write_config(cfg);
    }
    else
    {
        // 解析数据
        char *param[1] = {0};
        analyseParam(info, 1, param);
        cfg->switchInterval = atol(param[0]);
    }
}

struct PictureAppRunData
{
    unsigned long pic_perMillis; // 图片上一回更新的时间

    File_Info *image_file;      // movie文件夹下的文件指针头
    File_Info *pfile;           // 指向当前播放的文件节点
    int image_pos_increate = 1; // 文件的遍历方向
    bool refreshFlag = false;   // 是否更新
    bool tftSwapStatus;
};

static PIC_Config cfg_data;
static PictureAppRunData *run_data = NULL;

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft->height())
        return 0;

    // This function will clip the image block rendering automatically at the TFT boundaries
    tft->pushImage(x, y, w, h, bitmap);

    // This might work instead if you adapt the sketch to use the Adafruit_GFX library
    // tft.drawRGBBitmap(x, y, bitmap, w, h);

    // Return 1 to decode next block
    return 1;
}

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

static int picture_init(AppController *sys)
{
    photo_gui_init();
    // 获取配置信息
    read_config(&cfg_data);
    // 初始化运行时参数
    run_data = (PictureAppRunData *)malloc(sizeof(PictureAppRunData));
    run_data->pic_perMillis = 0;
    run_data->image_file = NULL;
    run_data->pfile = NULL;
    run_data->image_pos_increate = 1;
    // 保存系统的tft设置参数 用于退出时恢复设置
    run_data->tftSwapStatus = tft->getSwapBytes();
    tft->setSwapBytes(true); // We need to swap the colour bytes (endianess)

    run_data->image_file = tf.listDir(IMAGE_PATH);
    if (NULL != run_data->image_file)
    {
        run_data->pfile = get_next_file(run_data->image_file->next_node, 1);
    }

    // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
    TJpgDec.setJpgScale(1);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
    return 0;
}

static void picture_process(AppController *sys,
                            const ImuAction *act_info)
{
    lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_FADE_ON;

    if (RETURN == act_info->active)
    {
        sys->app_exit();
        return;
    }

    if (TURN_RIGHT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_RIGHT;
        run_data->image_pos_increate = 1;
        run_data->refreshFlag = true;
    }
    else if (TURN_LEFT == act_info->active)
    {
        anim_type = LV_SCR_LOAD_ANIM_OVER_LEFT;
        run_data->image_pos_increate = -1;
        run_data->refreshFlag = true;
    }

    if (NULL == run_data->image_file)
    {
        sys->app_exit();
        return;
    }

    // 自动切换的时间检测
    if (0 != run_data->image_pos_increate &&
        0 != cfg_data.switchInterval &&
        GET_SYS_MILLIS() - run_data->pic_perMillis >= cfg_data.switchInterval)
    {
        run_data->refreshFlag = true;
    }

    if (true == run_data->refreshFlag)
    {
        if (NULL != run_data->image_file)
        {
            run_data->pfile = get_next_file(run_data->pfile,
                                            run_data->image_pos_increate);
        }
        char file_name[PIC_FILENAME_MAX_LEN] = {0};
        snprintf(file_name, PIC_FILENAME_MAX_LEN, "%s/%s",
                 run_data->image_file->file_name,
                 run_data->pfile->file_name);
        // Draw the image, top left at 0,0
        Serial.print(F("Decode image: "));
        Serial.println(file_name);
        if (NULL != strstr(file_name, ".jpg") || NULL != strstr(file_name, ".JPG"))
        {
            // 直接解码jpg格式的图片
            TJpgDec.drawSdJpg(0, 0, file_name);
        }
        else if (NULL != strstr(file_name, ".bin") || NULL != strstr(file_name, ".BIN"))
        {
            // 使用LVGL的bin格式的图片
            display_photo(file_name, anim_type);
        }
        run_data->refreshFlag = false;
        // 重置更新的时间标记
        run_data->pic_perMillis = GET_SYS_MILLIS();
    }
    delay(300);
}

static void picture_background_task(AppController *sys,
                                    const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int picture_exit_callback(void *param)
{
    photo_gui_del();
    // 释放文件名链表
    release_file_info(run_data->image_file);
    // 恢复此前的驱动参数
    tft->setSwapBytes(run_data->tftSwapStatus);

    // 释放运行数据
    if (NULL != run_data)
    {
        free(run_data);
        run_data = NULL;
    }
    return 0;
}

static void picture_message_handle(const char *from, const char *to,
                                   APP_MESSAGE_TYPE type, void *message,
                                   void *ext_info)
{
    switch (type)
    {
    case APP_MESSAGE_GET_PARAM:
    {
        char *param_key = (char *)message;
        if (!strcmp(param_key, "switchInterval"))
        {
            snprintf((char *)ext_info, 32, "%lu", cfg_data.switchInterval);
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
        if (!strcmp(param_key, "switchInterval"))
        {
            cfg_data.switchInterval = atol(param_val);
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

APP_OBJ picture_app = {PICTURE_APP_NAME, &app_picture, "",
                       picture_init, picture_process, picture_background_task,
                       picture_exit_callback, picture_message_handle};
