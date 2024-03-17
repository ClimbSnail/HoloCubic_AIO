#include "Arduino.h"
#include "DigitalRainAnimation.hpp"
#include "sys/app_controller.h"
#include "common.h"

/*
功能：代码雨
说明：参考了爆改车间主任推荐的https://github.com/0015/TP_Arduino_DigitalRain_Anim
左右可切换大小
*/


/* 系统变量 */
extern bool isCheckAction;
extern ImuAction *act_info;


void codeRain_process(lv_obj_t * ym){
  DigitalRainAnimation<TFT_eSPI> *matrix_effect = new DigitalRainAnimation<TFT_eSPI>();
  bool codeSizeFont = false;//123564
  lv_obj_t *obj = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(obj,lv_color_hex(0),LV_STATE_DEFAULT);
  lv_scr_load_anim(obj, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);
  /* 延时999ms，防止同时退出app */
  for(uint16_t i=0;i<573;i++){
      lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
      delay(1);//
  }
  matrix_effect->init(tft,codeSizeFont);
  unsigned long tempD = 0;
  while(1){
    matrix_effect->loop();

    /* MPU6050数据获取 */
    if (isCheckAction){
        isCheckAction = false;
        act_info = mpu.getAction();
    }

    /* MPU6050动作响应 */
    if (RETURN == act_info->active){
        lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 573, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
        lv_obj_invalidate(lv_scr_act());//哪怕缓存没变，也让lvgl下次更新全部屏幕
        /* 延时999ms，防止同时退出app */
        for(uint16_t i=0;i<898+500;i++){
            lv_timer_handler();//让LVGL更新屏幕，让操作者可以看到已执行动作
            delay(1);//
        }
        lv_obj_clean(obj);
        lv_obj_del(obj);
        delete matrix_effect;
        return;//退出此功能
    }else if(TURN_RIGHT == act_info->active){
      if(millis()-tempD > 999){
        codeSizeFont = !codeSizeFont;
        matrix_effect->init(tft,codeSizeFont);
        tempD = millis();
      }
    }else if(TURN_LEFT == act_info->active){
      if(millis()-tempD > 999){
        codeSizeFont = !codeSizeFont;
        matrix_effect->init(tft,codeSizeFont);
        tempD = millis();
      }
    }else if(act_info->active == UP){
        
    } 
    act_info->active = ACTIVE_TYPE::UNKNOWN;
    act_info->isValid = 0;
  }
}

