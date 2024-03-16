#include "lvgl.h"
#include "EMOJI.h"
#include "../LHLXW_StartAnim.h"
#include "stdlib.h"
#include "sys/app_controller.h"

extern EMOJI_RUN *emj_run;

void next_emoji(void){
    lv_group_focus_next(emj_run->optionListGroup);
}
void prev_emoji(void){
    lv_group_focus_prev(emj_run->optionListGroup);
}

//APP输入设备键值读取回调函数（lvgl会以LV_INDEV_DEF_READ_PERIOD为周期调用）
//退出APP后，创建的输入设备indev_mpu6050key被删除，故系统（lvgl）不会再调用
static void mpu6050key_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data){
    // uint8_t act_key = mpu6050key_var;//由LHLXW_process函数action->active的值来改变
    if(emj_run->mpu6050key_var!=0){//mpu6050key_var由LHLXW_process函数action->active的值来改变
        data->state = LV_INDEV_STATE_PR;
        switch(emj_run->mpu6050key_var){
            case 1:
                data->key = LV_KEY_NEXT;//下一个
            break;
            case 2:
                data->key = LV_KEY_PREV;//上一个
            break;
        }
    }else{
        data->state = LV_INDEV_STATE_REL;
    }
}

//焦点改变回调函数
//之所以要写一个回调函数来改变当前选中的表情序号，是因为尝试将此功能写在mpu6050key_read函数或者
//LHLXW_process函数中，会导致快速改变焦点时，emoji_var的值与焦点表情对应不上
void focus_alter_cb(){
    if(emj_run->emoji_mode){
        if(emj_run->mpu6050key_var == 1){//下翻
            emj_run->emoji_var++;
            if(emj_run->emoji_var>emj_run->emoji_Maxnum)emj_run->emoji_var = 1;
        }else{//上翻
            emj_run->emoji_var--;
            if(emj_run->emoji_var<1)emj_run->emoji_var = emj_run->emoji_Maxnum;
        }
        emj_run->mpu6050key_var = 0;
    }
}

//emoji GUI初始化
void EMOJI_GUI_Init(void){
    emj_run->EMOJI_GUI_OBJ = lv_obj_create(NULL);//创建一个空的活动页面作为app主页面

    lv_obj_set_style_bg_color(emj_run->EMOJI_GUI_OBJ,lv_color_hex(0),LV_STATE_DEFAULT);//将此活动页面背景颜色设置为黑色

    static lv_indev_drv_t mpu6050key_driver;//创建输入设备（这里必须用static修饰，否则会导致系统重启）
    lv_indev_drv_init(&mpu6050key_driver);//初始化输入设备
    mpu6050key_driver.type = LV_INDEV_TYPE_KEYPAD;//输入设备类型设为键盘类
    mpu6050key_driver.read_cb = mpu6050key_read;//键盘值读取函数（lvgl会以LV_INDEV_DEF_READ_PERIOD为周期调用）
    emj_run->indev_mpu6050key = lv_indev_drv_register(&mpu6050key_driver);//注册驱动设备，返回指向新设备的指针
    emj_run->optionListGroup= lv_group_create();//创建一个组
    lv_indev_set_group(emj_run->indev_mpu6050key, emj_run->optionListGroup);//输入设备与组关联
    
    char *path = (char*)malloc(38);//必须用char*类型，不能用uint8_t*
    lv_obj_t *option_obj;//按钮部件
    lv_obj_t *optionImg_obj;//图片部件  直接用图片按钮部件无法被选中

    for(uint8_t i = 0;i<emj_run->emoji_Maxnum; i++){
        option_obj = lv_btn_create(emj_run->EMOJI_GUI_OBJ);//基于空页面创建按钮部件
        lv_obj_set_size(option_obj,60,60);//设置大小
        lv_obj_set_style_bg_color(option_obj,lv_color_hex(0),LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(option_obj,lv_color_hex(0x00ff00),LV_STATE_FOCUSED);//设置背景颜色
        lv_obj_set_x(option_obj,15+(i%3)*75);//设置坐标
        lv_obj_set_y(option_obj,15+(i/3)*75);//设置坐标
        lv_group_add_obj(emj_run->optionListGroup,option_obj);//将按钮添加到组

        optionImg_obj = lv_img_create(option_obj);//基于按钮部件创建图片部件
        lv_obj_align(optionImg_obj,LV_ALIGN_CENTER,0,-1);//在按钮部件中心位置往上便宜1
        sprintf(path,"S:/LH&LXW/emoji/images/image%d.bin",i+1);//图标路径
        lv_img_set_src(optionImg_obj,path);//设置图片源
    }
    lv_group_set_focus_cb(emj_run->optionListGroup,(lv_group_focus_cb_t)focus_alter_cb);
    lv_scr_load_anim(emj_run->EMOJI_GUI_OBJ, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 580, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启
    /* 这里加延时是为了防止动画执行完成前就调用系统退出函数或者删除动画对象导致系统出错 */
    for(uint16_t i=0;i<780;i++){
        lv_task_handler();
        delay(1);
    }
    free(path);
}


void EMOJI_GUI_DeInit(lv_obj_t *ym){
    lv_indev_delete(emj_run->indev_mpu6050key);//删除输入设备
    lv_group_del(emj_run->optionListGroup);//删除APP选项列表组
    /* 切换页面，同时删除旧屏幕(这里不用此函数自带的删除，等执行完后手动删除) */
    lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_TOP, 580, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启

    /* 这里加延时是为了防止动画执行完成前就调用系统退出函数或者删除动画对象导致系统出错 */
    for(uint16_t i=0;i<780;i++){
        lv_task_handler();
        delay(1);
    }
    /* 如果要手动删除对象，那么一定要在对象的动画执行完了再删除，否则会有问题*/
    lv_obj_clean(emj_run->EMOJI_GUI_OBJ); //删除对象的所有子项
    lv_obj_del(emj_run->EMOJI_GUI_OBJ); //删除对象（实测会释放内存，不会造成内存泄漏）
}
