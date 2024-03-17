#include "eye_functions.h"
#include "sys/app_controller.h"
#include "eye.h"

/*
功能：眼珠子
操作说明：左/右切换眼睛样式
其他：
参考了https://learn.adafruit.com/animated-electronic-eyes/software
以及AIO_FIRMWARE_PIO/lib/TFT_eSPI/examples/Generic/Animated_Eyes_1
没有用爆改车间主任，因为其没有开源。

编译说明：eye.h中有一个mode_eye宏
当EYE_MIN为0，以下模式全部使能，有四种虹膜效果
为1时，只有一种虹膜效果，只生效0，1，2 能大大减小内存内存占用
00：小 default
01：大 default
02：双 default

03：小 dragon
04：大 dragon
05：双 dragon

06：小 goat
07：大 goat
08：双 goat

09：小 noSclera
10：大 noSclera
11：双 noSclera

*/





/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;

eye_run *e_run = NULL;


//128有61帧左右，实测1024也只有63帧左右（原大小的情况下）
//改成240*240大小后，数据从pbuffer转移到pbuffer_m花费大量时间,同时屏幕刷新范围也增加
//改成240*240大小后，pbuffer长度必须为128，确保一次渲染一整行，屏幕实际更新两行（也就是一次渲染128个像素，根据这128个像素同时一次更新240*2个像素）
uint16_t *pbuffer=NULL; // 眼珠子算法渲染的缓冲区
uint16_t *pbuffer_m=NULL;//实际刷新到屏幕的缓存(屏幕一行的大小，240*2)






//由updateEye调用
bool eye_loop(void){
    /* MPU6050数据获取 */
    if (isCheckAction){
        isCheckAction = false;
        act_info = mpu.getAction();

        /* MPU6050动作响应 */
        if (RETURN == act_info->active){
            tft->fillRect(0, 0, 240, 240, 0);
            e_run->eye_flg = false;
            return true;//退出此功能
        }else if(TURN_RIGHT == act_info->active){
        if(millis()-e_run->tempD > 1099){
            tft->fillRect(0, 0, 240, 240, 0);
            e_run->mode_eye--;

            #if EYE_MIN
            if(e_run->mode_eye>11)e_run->mode_eye = 3;
            #else
            if(e_run->mode_eye>11)e_run->mode_eye = 11;
            #endif

            if(e_run->mode_eye>11)e_run->mode_eye = 11;
            e_run->tempD = millis();
        }
        }else if(TURN_LEFT == act_info->active){
        if(millis()-e_run->tempD > 1099){
            tft->fillRect(0, 0, 240, 240, 0);
            e_run->mode_eye++;

            #if EYE_MIN
            e_run->mode_eye%=3;
            #else
            e_run->mode_eye%=12;
            #endif

            e_run->tempD = millis();
        }
        }else if(act_info->active == UP){
            
        } 
        act_info->active = ACTIVE_TYPE::UNKNOWN;
        act_info->isValid = 0;
    }
    return false;
}

void eye_process(lv_obj_t *ym){
    lv_obj_t *obj = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(obj,lv_color_hex(0),LV_STATE_DEFAULT);
    lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);
    for(uint16_t i=0;i<573;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }
    e_run = (eye_run*)malloc(sizeof(eye_run)); 
    pbuffer = (uint16_t*)malloc(128*2); 
    pbuffer_m = (uint16_t*)malloc(240*2); 
    initEyes();
    e_run->eye_flg = true;
    e_run->mode_eye = 0;
    e_run->tempD=0;
    while(e_run->eye_flg){
        updateEye();
    }
    free(pbuffer);
    free(pbuffer_m);
    free(e_run);
    lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 573, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
    lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
    /* 延时999ms，防止同时退出app */
    for(uint16_t i=0;i<898+500;i++){
        lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
        delay(1);//
    }
    lv_obj_clean(obj);
    lv_obj_del(obj);
}



//占用空间 眼睛类型 是否采用
//68.3 default v
//66.5 cat x
//70.6 doe x
//67.8 goat x
//65.3 owl x
//72.0 dragon
//65.3 nauga x
//72.0 noSclera x
//68.3 newt v
//68.3 terminator v
