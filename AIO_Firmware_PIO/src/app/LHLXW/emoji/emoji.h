#ifndef EMOJI_H
#define EMOJI_H

#include "lvgl.h"
#include "sys/app_controller.h"
#include "../../media_player/docoder.h"//解码器父类


struct EMOJI_RUN{
    uint8_t emoji_var;//当前播放的表情序号
    uint8_t mpu6050key_var;//进入APP后，mpu6050的状态值，左倾斜为2，右倾为1，其余为0
    uint8_t emoji_Maxnum;//总共有多少个表情（SPIFFS不会用，所以人为输入个数，即读取SD卡配置文件)
    bool emoji_mode ;//app运行模式，true为选择表情，false为表情播放
    PlayDocoderBase *emoji_docoder;//表情视频 解码器
    File emoji_file;//表情视频 文件
    lv_obj_t *EMOJI_GUI_OBJ;//EMOJI UI界面
    lv_indev_t * indev_mpu6050key;//输入设备指针
    lv_group_t *optionListGroup;//APP 选项列表 组，用来关联输入设备
};
void emoji_process(lv_obj_t *ym);

#endif