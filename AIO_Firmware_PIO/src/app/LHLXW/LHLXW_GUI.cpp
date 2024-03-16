#include "LHLXW_GUI.h"
#include "LHLXW_StartAnim.h"
#include "arduino.h"

LV_FONT_DECLARE(APP_OPTION_ico);//定义选项字符

LHLXW_RUN *lhlxw_run = NULL;

/* 功能选项渐变色样式 */
static const int option_color[5][2] = {
    {0x7303c0,0xec38bc},
    {0xdcffbd,0xcc86d1},
    {0xfc466b,0x3f5efb},
    {0xfbd786,0x1cefff},
    {0x00f260,0x0575e6},
};

/* 功能选项x坐标常量 */
static const uint8_t x_zu[5]      = {24,46,74,130,168};
/* 功能选项y坐标常量 */
static const uint8_t y_zu[5]      = {88,72,54,72,88};
/* 功能选项width常量 */
static const uint8_t width_zu[5]  = {50,68,100,68,50};
/* 功能选项height常量 */
static const uint8_t height_zu[5] = {92,124,160,124,92};

/* 有上面四个数组就可以了，这里为了方便写程序，添加下面的数组 */
static const uint8_t obj_var[5][4][2]={
  //   x        y       w       h
    {{168,24},{88,88},{48,48},{92,92}},   //4变化到0
    {{24,46},{88,72},{48,64},{92,124}},  //0变化到1
    {{46,74},{72,54},{64,92},{124,160}}, //1变化到2
    {{74,130},{54,72},{92,64},{160,124}},//2变化到3
    {{130,168},{72,88},{64,48},{124,92}},//3变化到4
};

/* 层与option_num的关系 */
struct tierMode{
    uint8_t tierButton[3];//按钮层
    char tierText[12];//标签层
};
#define OPTION_1  "\xee\x99\xa1"//眼睛
#define OPTION_2  "\xee\x98\xab"//骰子
#define OPTION_3  "\xee\x98\x8d"//心动
#define OPTION_4  "\xee\x98\x9d"//代码
#define OPTION_5  "\xee\x98\xb0"//表情
static const struct tierMode tier_mode[5] = {
    {{2,1,3},OPTION_1},
    {{1,2,0},OPTION_2},
    {{0,1,4},OPTION_3},
    {{4,0,3},OPTION_4},
    {{3,4,2},OPTION_5}
};

static void x_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_x(var,v);
}
static void y_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_y(var,v);
}
static void w_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_width(var,v);
}
static void h_cb(lv_obj_t *var,int32_t v){
    lv_obj_set_height(var,v);
}


void LHLXW_GUI_Init(void){
    lhlxw_run->LV_BACKUP_OBJ = lv_scr_act();//备份此前的屏幕，以便退出APP时恢复原样（实测退出APP不切换到备份屏幕就会出问题）
    lhlxw_run->LV_LHLXW_GUI_OBJ = lv_obj_create(NULL);//创建一个空的活动页面作为app主页面
    /* 如果将启动动画放这里能避免一次性占吃掉很多内存，但是动画结束之后出现表情菜单又非常生硬 */
    //startLog(LV_LHLXW_GUI_OBJ);//开始log动画，动画结束后会切换到LV_LHLXW_GUI_OBJ页面也就是APP主页面
    
    lv_obj_set_style_bg_color(lhlxw_run->LV_LHLXW_GUI_OBJ,lv_color_hex(0),LV_STATE_DEFAULT);
   
    for(uint8_t i=0;i<5;i++){
        lhlxw_run->option_fun[i] = lv_btn_create(lhlxw_run->LV_LHLXW_GUI_OBJ);
        lv_obj_set_pos(lhlxw_run->option_fun[i],x_zu[i],y_zu[i]);
        lv_obj_set_size(lhlxw_run->option_fun[i],width_zu[i],height_zu[i]);
        lv_obj_set_style_bg_color(lhlxw_run->option_fun[i], lv_color_hex(option_color[i][0]),LV_STATE_DEFAULT);//起始色
        lv_obj_set_style_bg_grad_color(lhlxw_run->option_fun[i],lv_color_hex(option_color[i][1]),LV_STATE_DEFAULT);//终止色
        lv_obj_set_style_bg_grad_dir(lhlxw_run->option_fun[i],LV_GRAD_DIR_VER,LV_STATE_DEFAULT);//开启渐变色
    }

    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[2]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[1]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[0].tierButton[0]]);

    lhlxw_run->option_label = lv_label_create(lhlxw_run->LV_LHLXW_GUI_OBJ);//基于按钮创建标签
    lv_obj_set_align(lhlxw_run->option_label,LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lhlxw_run->option_label,&APP_OPTION_ico,LV_STATE_DEFAULT );
    lv_label_set_text(lhlxw_run->option_label,tier_mode[0].tierText);
    /* 放此处会导致表情页面和动画页面同时存在，对RAM要求大  */
    startLog(lhlxw_run->LV_LHLXW_GUI_OBJ);//开始log动画，动画结束后会切换到LV_LHLXW_GUI_OBJ页面也就是APP主页面
}

/* 切换选项 */
void SWITCH_OPTION(bool _flg_,uint8_t mode){
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_time(&a1,607);
    lv_anim_set_path_cb(&a1,lv_anim_path_ease_in);
    for(uint8_t i=0;i<5;i++){
        lv_anim_set_var(&a1,lhlxw_run->option_fun[i]);
        for(uint8_t ii=0;ii<4;ii++){
            if(_flg_)//左到右
                lv_anim_set_values(&a1,obj_var[(i+mode)%5][ii][0],obj_var[(i+mode)%5][ii][1]);
            else//右到左
                lv_anim_set_values(&a1,obj_var[(i+mode+1)%5][ii][1],obj_var[(i+mode+1)%5][ii][0]);
            switch(ii){
                case 0://x变化
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)x_cb);
                break;
                case 1://x变化
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)y_cb);
                break;
                case 2://x变化
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)w_cb);
                break;
                case 3://x变化
                    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)h_cb);
                break;
            }
            lv_anim_start(&a1);
        }
    }
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[2]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[1]]);
    lv_obj_move_foreground(lhlxw_run->option_fun[tier_mode[mode].tierButton[0]]);
    lv_label_set_text(lhlxw_run->option_label,tier_mode[mode].tierText);
    lv_obj_move_foreground(lhlxw_run->option_label);
}

void LHLXW_GUI_DeInit(void){
    /* 切换页面，同时删除旧屏幕(这里不用此函数自带的删除，等执行完后手动删除) */
    lv_scr_load_anim(lhlxw_run->LV_BACKUP_OBJ, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);//调用系统退出函数之前，一定要等待动画结束否则会导致系统重启

    /* 这里加延时是为了防止动画执行完成前就调用系统退出函数或者删除动画对象导致系统出错 */
    for(uint16_t i=0;i<573;i++){
        lv_task_handler();
        delay(1);
    }
    /* 如果要手动删除对象，那么一定要在对象的动画执行完了再删除，否则会有问题*/
    lv_obj_clean(lhlxw_run->LV_LHLXW_GUI_OBJ); //删除对象的所有子项
    lv_obj_del(lhlxw_run->LV_LHLXW_GUI_OBJ); //删除对象（实测会释放内存，不会造成内存泄漏）
}
