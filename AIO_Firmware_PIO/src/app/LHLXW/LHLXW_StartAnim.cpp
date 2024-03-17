#include "LHLXW_StartAnim.h"
#include "arduino.h"

#define LCD_W 240
#define LCD_H 240
#define log_bar_w 118
#define tim_log_bar 1000//bar自加持续时间，单位ms
#define log_anim_bg "S:/LH&LXW/log_anim/bg.bin"


static void anim_x_cb1(lv_obj_t *var,int32_t v){
    lv_obj_set_y(var,v);
}
static void anim_x_cb2(lv_obj_t *var,int32_t v){
    lv_obj_set_y(var,v);
}
static void anim_x_cb3(lv_obj_t *var,int32_t v){
    lv_obj_set_style_text_opa(var,v,LV_STATE_DEFAULT);
}
static void anim_x_cb4(lv_obj_t *var,int32_t v){
    lv_obj_set_style_img_opa(var,v,0);
}

void startLog(lv_obj_t *ym){
    
    /* 创建一个log anim屏幕,并切换到此屏幕*/
    lv_obj_t *LOG_SCR = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(LOG_SCR,lv_color_hex(0),LV_STATE_DEFAULT);//将此活动页面背景颜色设置为黑色
    lv_scr_load(LOG_SCR);
    /* 刷新屏幕与缓存 */
    lv_obj_invalidate(LOG_SCR);
    // lv_task_handler();
    /* 创建log anim背景图片 */
    lv_obj_t *bg_img = lv_img_create(LOG_SCR);
    lv_img_set_src(bg_img,log_anim_bg);
    /* 设置背景透明度为0 */
    lv_obj_set_style_img_opa(bg_img, 0,0);
    /* 创建一个bar部件 */
    lv_obj_t* log_bar = lv_bar_create(bg_img);
    lv_obj_set_pos(log_bar,(LCD_W-log_bar_w)/2,LCD_H/2-10);

    /* 设置bar指示器背景颜色和透明度为可见(0xa8ff78,0x78ffd6) */
    lv_obj_set_style_bg_color( log_bar, lv_color_hex(0x00f260),LV_STATE_DEFAULT|LV_PART_INDICATOR);//渐变起始色
    lv_obj_set_style_bg_grad_color(log_bar,lv_color_hex(0x0575e6),LV_STATE_DEFAULT|LV_PART_INDICATOR);//渐变终止色
    lv_obj_set_style_bg_grad_dir(log_bar,LV_GRAD_DIR_HOR,LV_STATE_DEFAULT|LV_PART_INDICATOR);//开启渐变色
    lv_obj_set_style_bg_opa(log_bar,255,LV_STATE_DEFAULT|LV_PART_INDICATOR);
    /* 设置bar主体背景颜色和透明度为不可见 */
    lv_obj_set_style_bg_color(log_bar,lv_color_hex(0),LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(log_bar,0,LV_STATE_DEFAULT);
    /* 设置bar形状设置为长条形状 */
    lv_obj_set_size(log_bar,log_bar_w,5);
    lv_bar_set_range(log_bar,0,log_bar_w);//设置值的范围（范围要在设置值之前设置！）
    lv_obj_set_style_anim_time( log_bar, tim_log_bar,LV_STATE_DEFAULT );//动画也要在设置值之前就设置，否则看不到效果（时间单位为ms）
    lv_bar_set_mode(log_bar,LV_BAR_MODE_NORMAL);//从零值绘制到当前值（当前值可以小于0）
    lv_obj_update_layout(log_bar);//更新配置到部件
    lv_bar_set_value(log_bar,log_bar_w,LV_ANIM_ON);//将bar的值设为指定值，它会再tim_log_bar时间内完成设置

    /* 设置log字样标签，没插内存卡会导致标签位置不对 */
    lv_obj_t* log_label = lv_label_create(bg_img);
    lv_obj_align(log_label,LV_ALIGN_CENTER,0,10);//基于图片居中，且向下偏移10
    lv_obj_set_style_text_font(log_label,&lv_font_montserrat_24,LV_STATE_DEFAULT );//需要先在lv_conf.h中开启此字体对应宏
    lv_label_set_text(log_label,"LH&LXW");
    lv_obj_set_style_text_color(log_label,lv_color_hex(0xa8a078),LV_STATE_DEFAULT);//这里就不设置渐变色，因为字符的渐变色是以单个字符为对象的
    lv_obj_set_style_text_opa(log_label,0,LV_STATE_DEFAULT);//设置背景颜色的透明度

    /* log bar坐标改变动画 */
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1,log_bar);
    lv_anim_set_values(&a1,LCD_H/2-10,LCD_H/2+10);
    lv_anim_set_time(&a1,800);
    lv_anim_set_exec_cb(&a1,(lv_anim_exec_xcb_t)anim_x_cb1);
    lv_anim_set_path_cb(&a1,lv_anim_path_ease_out);
    lv_anim_set_delay(&a1, 700);//在值改变完成前1000-700=300ms前开始此动画

    /* log 标签坐标改变动画 */
    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2,log_label);
    lv_anim_set_values(&a2,20-10,5-10);
    lv_anim_set_time(&a2,1100);
    lv_anim_set_exec_cb(&a2,(lv_anim_exec_xcb_t)anim_x_cb2);
    lv_anim_set_path_cb(&a2,lv_anim_path_linear);
    lv_anim_set_delay(&a2, 700);//同步bar坐标变化

    /* log 标签透明度改变动画 */
    lv_anim_t a3;
    lv_anim_init(&a3);
    lv_anim_set_var(&a3,log_label);
    lv_anim_set_values(&a3,0,255);
    lv_anim_set_time(&a3,1500);
    lv_anim_set_exec_cb(&a3,(lv_anim_exec_xcb_t)anim_x_cb3);
    lv_anim_set_path_cb(&a3,lv_anim_path_linear);
    lv_anim_set_delay(&a3, 700);

    /* log 背景颜色透明度改变动画 */
    lv_anim_t a4;
    lv_anim_init(&a4);
    lv_anim_set_var(&a4,bg_img);
    lv_anim_set_values(&a4,0,255);
    lv_anim_set_time(&a4,1400);
    lv_anim_set_exec_cb(&a4,(lv_anim_exec_xcb_t)anim_x_cb4);
    lv_anim_set_path_cb(&a4,lv_anim_path_ease_in);

    /* 切换log anim页面 */
    lv_scr_load(LOG_SCR);

    /* 开启动画 */
    lv_anim_start(&a1);
    lv_anim_start(&a2);
    lv_anim_start(&a3);
    lv_anim_start(&a4);
    
    /* 等待动画结束 */
    /* 也可以用lv_anim_set_ready_cb函数实现 */
    for(uint16_t i=0;i<2000;i++){
        lv_task_handler();
        delay(1);
    }
    /* 丝滑过度到表情菜单，同时删除旧屏幕(这里不用此函数自带的删除，等执行完后手动删除) */
    lv_scr_load_anim(ym, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 573, 0, false);//上翻动画，切换到此页面

    /* 这里加延时是为了防止动画执行完成前就删除动画对象导致系统出错 */
    for(uint16_t i=0;i<573;i++){
        lv_task_handler();
        delay(1);
    }

    /* 删除启动log动画所有部件 */
    lv_obj_clean(LOG_SCR);
    lv_obj_del(LOG_SCR);
}
