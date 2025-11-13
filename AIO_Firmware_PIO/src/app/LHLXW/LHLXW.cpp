#include "./emoji/emoji.h"
#include "./codeRain/codeRain.h"
#include "./eye/eye.h"
#include "./heartbeat_/heartbeat_.h"
#include "./cyber/cyber.h"
#include "sys/app_controller.h"
#include "LHLXW_GUI.h"


#define APP_NAME   "LH&LXW"  // APP name
extern const lv_img_dsc_t LHLXW_ICO;  // APP icon

/*
Author's Note:
This APP's option framework is quite poor and not worth studying for learning purposes!
The main issue is with functionality - adding new features would require rewriting the entire framework.
The LVGL animation part still has some reference value.

Description:
This APP has 5 functions, with each function's introduction in its respective .c file.

SD card storage instructions (LH&LXW APP):
./LH&LXW    Store files required by LH&LXW APP

./LH&LXW/log_anim/bg.bin is the background image for this app's startup animation, can be replaced with your own image

*/



/* System variables */
extern bool isCheckAction;
extern ImuAction *act_info;

/* APP variables */
extern LHLXW_RUN *lhlxw_run;


static int LHLXW_init(AppController *sys){
    lhlxw_run = (LHLXW_RUN*)malloc(sizeof(LHLXW_RUN));
    lhlxw_run->option_num = 0;  // Ensure that each time entering app, the current option number is 0
    setCpuFrequencyMhz(240);
    LHLXW_GUI_Init();
    return 0;
}



static void LHLXW_process(AppController *sys,const ImuAction *action){
    while(1){
        lv_timer_handler();

        /* MPU6050 data acquisition */
        if (isCheckAction){
            isCheckAction = false;
            act_info = mpu.getAction();
        }

        /* MPU6050 action response */
        if (RETURN == act_info->active){
            LHLXW_GUI_DeInit();
            sys->app_exit();
            return;  // Exit this function
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
                eye_process(lhlxw_run->LV_LHLXW_GUI_OBJ);  // Eye rendering algorithm uses recursion, which prevents smooth exit
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
    // This function is a background task, the main controller calls this function at one-minute intervals
    // This function should only call "resident data", other variables may have been released due to lifecycle
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