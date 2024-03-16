#include "./emoji/emoji.h"
#include "./codeRain/codeRain.h"
#include "./eye/eye.h"
#include "./heartbeat_/heartbeat_.h"
#include "./cyber/cyber.h"
#include "sys/app_controller.h"
#include "LHLXW_GUI.h"


#define APP_NAME   "LH&LXW"//APP名字
extern const lv_img_dsc_t LHLXW_ICO;//APP图标

/*
作者建议：
此APP功能选项框架非常垃圾，没有学习意义，请勿参考！
主要体现在功能上面，如果后续添加功能相当于重新写整个框架。
lvgl动画部分尚有一点参考价值

说明：
此APP共5个功能，每个功能的介绍在各自的.c文件中。

SD卡存放说明(LH&LXW APP):
./LH&LXW    存放LH&LXW APP需要的文件

./LH&LXW/log_anim/bg.bin 为此app启动动画的背景图片，可以修改成你的图片

*/



/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;

/* APP变量 */
extern LHLXW_RUN *lhlxw_run;


static int LHLXW_init(AppController *sys){
    lhlxw_run = (LHLXW_RUN*)malloc(sizeof(LHLXW_RUN));
    lhlxw_run->option_num = 0;//确保每次进入app，当先选项序号都为0
    setCpuFrequencyMhz(240);
    LHLXW_GUI_Init();
    return 0;
}



static void LHLXW_process(AppController *sys,const ImuAction *action){
    while(1){
        lv_timer_handler();

        /* MPU6050数据获取 */
        if (isCheckAction){
            isCheckAction = false;
            act_info = mpu.getAction();
        }

        /* MPU6050动作响应 */
        if (RETURN == act_info->active){
            LHLXW_GUI_DeInit();
            sys->app_exit();
            return;//退出此功能
        }else if(TURN_RIGHT == act_info->active){
            lhlxw_run->option_num++;
            if(lhlxw_run->option_num==5)lhlxw_run->option_num = 0;
            SWITCH_OPTION(true,lhlxw_run->option_num);
            for(uint16_t i=0;i<400;i++){
                lv_task_handler();
                delay(1);
            }
        }else if(TURN_LEFT == act_info->active){
            lhlxw_run->option_num--;
            if(lhlxw_run->option_num>5)lhlxw_run->option_num = 4;
            SWITCH_OPTION(false,lhlxw_run->option_num);
            for(uint16_t i=0;i<400;i++){
                lv_task_handler();
                delay(1);
            }
        }else if(act_info->active == UP){
            if(lhlxw_run->option_num == 4)
                emoji_process(lhlxw_run->LV_LHLXW_GUI_OBJ);
            else if(lhlxw_run->option_num == 3)
                codeRain_process(lhlxw_run->LV_LHLXW_GUI_OBJ);
            else if(lhlxw_run->option_num == 0)
                eye_process(lhlxw_run->LV_LHLXW_GUI_OBJ);//眼睛渲染算法用了递归，导致无法丝滑退出
            else if(lhlxw_run->option_num == 2)
                heartbeat_process(lhlxw_run->LV_LHLXW_GUI_OBJ);
            else 
                cyber_pros(lhlxw_run->LV_LHLXW_GUI_OBJ);
        } 
        act_info->active = ACTIVE_TYPE::UNKNOWN;
        act_info->isValid = 0;
    }

}

static void LHLXW_background_task(AppController *sys,
                                 const ImuAction *act_info)
{
    // 本函数为后台任务，主控制器会间隔一分钟调用此函数
    // 本函数尽量只调用"常驻数据",其他变量可能会因为生命周期的缘故已经释放
}

static int LHLXW_exit_callback(void *param)
{
    free(lhlxw_run);
    return 0;
}

static void LHLXW_message_handle(const char *from, const char *to,
                                APP_MESSAGE_TYPE type, void *message,
                                void *ext_info)
{
}


APP_OBJ LHLXW_app = {APP_NAME, &LHLXW_ICO, "", LHLXW_init,
                    LHLXW_process, LHLXW_background_task, LHLXW_exit_callback,
                    LHLXW_message_handle};