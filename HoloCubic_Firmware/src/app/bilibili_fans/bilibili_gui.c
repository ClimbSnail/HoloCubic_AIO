#include "bilibili_gui.h"

#include "lv_port_indev.h"
#include "lvgl.h"

// #include "img_ico.c"
// #include "bilibili_ico.c"
// #include "../../lib/lv_lib_split_jpg/lv_sjpg.h"

// LV_IMG_DECLARE( small_image_sjpg );
// LV_IMG_DECLARE( wallpaper_jpg );

LV_FONT_DECLARE(myFont);


// lv_obj_t *clock_text = NULL;
lv_obj_t *bilibili_gui = NULL;
lv_obj_t *follower_label = NULL;
lv_obj_t *img_gui = NULL;

lv_obj_t *label1 = NULL;  //用户ID
lv_obj_t *label2 = NULL;  //ID
lv_obj_t *label3 = NULL;  //粉丝
lv_obj_t *label4 = NULL;  //粉丝数
lv_obj_t *label5 = NULL;  //签名
lv_obj_t *label6 = NULL;  //个性签名
lv_obj_t *label7 = NULL;  //关注
lv_obj_t *label8 = NULL;  //关注数


static lv_style_t default_style;
static lv_style_t label1_style;        //用户ID样式
static lv_style_t label2_style;       //昵称样式
static lv_style_t label3_style;       //粉丝数样式
static lv_style_t label4_style;       //粉丝数目（number）值样式
static lv_style_t style_img0;         //头像样式


void bilibili_gui_init(const char* data_card_fans , const char* data_card_name ,
                    const char* data_card_face , const char* data_card_friends ,
                    const char* data_card_sign){
    //lv_obj_t * lable = lv_obj_create(NULL, NULL);

    // 初始化屏幕
    bilibili_gui = lv_obj_create(NULL, NULL);
    lv_obj_add_style(bilibili_gui, LV_BTN_PART_MAIN, &default_style);
    lv_scr_load(bilibili_gui);
    // 初始化默认样式
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));
    //“用户ID”样式
    lv_style_init(&label1_style);
    lv_style_set_text_opa(&label1_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    //lv_style_set_text_color(&label1_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);//字体颜色白色
    lv_style_set_text_color(&label1_style, LV_STATE_DEFAULT, lv_color_make(0x04, 0xff, 0x00));//字体颜色
    lv_style_set_text_font(&label1_style, LV_STATE_DEFAULT, &myFont);
    //用户昵称样式
    lv_style_init(&label2_style);
    lv_style_set_text_color(&label2_style, LV_STATE_DEFAULT, lv_color_make(0x3d, 0x9c, 0xf0));
	lv_style_set_text_font(&label2_style, LV_STATE_DEFAULT, &myFont);
	lv_style_set_text_letter_space(&label2_style, LV_STATE_DEFAULT, 2);
    //粉丝数样式
    lv_style_init(&label3_style);
    lv_style_set_text_opa(&label3_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label3_style, LV_STATE_DEFAULT, lv_color_make(0x04, 0xff, 0x00));//字体颜色
    lv_style_set_text_font(&label3_style, LV_STATE_DEFAULT, &myFont);
    //“number”样式
    lv_style_init(&label4_style);
    lv_style_set_text_color(&label4_style, LV_STATE_DEFAULT, lv_color_make(0xff, 0x00, 0x00));
	lv_style_set_text_font(&label4_style, LV_STATE_DEFAULT, &lv_font_montserrat_30);
	lv_style_set_text_letter_space(&label4_style, LV_STATE_DEFAULT, 2);
    //头像样式
    lv_style_init(&style_img0);
    lv_style_set_image_recolor(&style_img0, LV_STATE_DEFAULT, lv_color_make(0x52, 0x19, 0x19));
	lv_style_set_image_recolor_opa(&style_img0, LV_STATE_DEFAULT, 0);
	lv_style_set_image_opa(&style_img0, LV_STATE_DEFAULT, 232);

    // 创建标签
    label1 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &label1_style);
    //lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -10); // 居中对齐
	lv_obj_set_pos(label1, 0, 50);//位置
    lv_obj_set_size(label1, 60, 0);//
    // lv_label_set_text(label1, "hello world !");
    lv_label_set_text(label1, "用户ID：");
	lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);

    //创建用户昵称对象
    label2 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label2, LV_LABEL_PART_MAIN, &label2_style);
	lv_obj_set_pos(label2, 70, 50);
	lv_obj_set_size(label2, 200, 0);
    //lv_label_set_text(label2, "溜马小哥");
    lv_label_set_text_fmt(label2, "%s", data_card_name);//参数
	lv_label_set_long_mode(label2, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label2, LV_LABEL_ALIGN_CENTER);
    
    //创建粉丝数对象
    label3 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label3, LV_LABEL_PART_MAIN, &label3_style);
	lv_obj_set_pos(label3, 3, 105);
	lv_obj_set_size(label3, 60, 0);
    lv_label_set_text(label3, "粉丝数：");
	lv_label_set_long_mode(label3, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label3, LV_LABEL_ALIGN_CENTER);

    //创建“number(粉丝数)”对象
    label4 = lv_label_create(bilibili_gui, NULL);
    lv_label_set_text_fmt(label4, "%s", data_card_fans);//参数
	lv_label_set_long_mode(label4, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label4, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style(label4, LV_LABEL_PART_MAIN, &label4_style);
	lv_obj_set_pos(label4, 74, 100);
	lv_obj_set_size(label4, 70, 0);

    // 创建签名标签
    label5 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label5, LV_LABEL_PART_MAIN, &label1_style);
    //lv_obj_align(label5, NULL, LV_ALIGN_CENTER, 0, -10); // 居中对齐
	lv_obj_set_pos(label5, 3, 152);//位置
    lv_obj_set_size(label1, 70, 0);//
    // lv_label_set_text(label5, "hello world !");
    lv_label_set_text(label5, "个性签名：");
	lv_label_set_long_mode(label5, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label5, LV_LABEL_ALIGN_CENTER);

    //创建个性签名对象
    label6 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label6, LV_LABEL_PART_MAIN, &label2_style);
	lv_obj_set_pos(label6, 80, 152);
	lv_obj_set_size(label6, 100, 0);
    lv_label_set_text_fmt(label6, "%s", data_card_sign);
	lv_label_set_long_mode(label6, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label6, LV_LABEL_ALIGN_CENTER);
    
    //创建 关注 对象
    label7 = lv_label_create(bilibili_gui, NULL);
    lv_obj_add_style(label7, LV_LABEL_PART_MAIN, &label3_style);
	lv_obj_set_pos(label7, 3, 200);
	lv_obj_set_size(label7, 60, 0);
    lv_label_set_text(label7, "关注数：");
	lv_label_set_long_mode(label7, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label7, LV_LABEL_ALIGN_CENTER);

    //创建 关注数 对象
    label8 = lv_label_create(bilibili_gui, NULL);
    // lv_label_set_text_fmt(label8, "%s", data_card_fans);//参数
    lv_label_set_text_fmt(label8, "%s", data_card_friends);//参数
	lv_label_set_long_mode(label8, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label8, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style(label8, LV_LABEL_PART_MAIN, &label4_style);
	lv_obj_set_pos(label8, 72, 200);
	lv_obj_set_size(label8, 70, 0);


    //创建 头像
    //
	//创建一个IMG对象并加载SD卡中的jpg图片解码显示///
	//
	// lv_obj_t * objpg =  lv_img_create(bilibili_gui, NULL);				// 创建一个IMG对象 
	// lv_img_set_src(objpg, "S:/bilbili/a.png");					// 加载SD卡中的JPG图片
	// lv_obj_align(objpg, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);			// 重新设置对齐
    lv_obj_t *img0 = lv_img_create(bilibili_gui, NULL);
    lv_img_set_src(img0, "S:/bilbili/wall.bin");					// 加载SD卡中的JPG图片
    // lv_img_set_src(img0, data_card_face);
	lv_obj_align(img0, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);			// 重新设置对齐
    lv_obj_add_style(img0, LV_IMG_PART_MAIN, &style_img0);
    lv_obj_set_pos(img0, 148, 2);
    lv_obj_set_size(img0, 90, 90);
    lv_obj_set_click(img0, true);
    // //lv_img_set_src(img0,&app_bilibili);
    // lv_img_set_pivot(img0, 24,25);
    // lv_img_set_angle(img0, 0);
}


void bilibili_gui_start(lv_scr_load_anim_t anim_type)
{
    
}

/*
 * 其他函数请根据需要添加
 */

// void display_bilibili(const char *file_name, lv_scr_load_anim_t anim_type)
// {

// }

void bilibili_obj_del(void) 
{
    if (NULL != label1)
    {
        lv_obj_clean(label1);
        lv_obj_clean(label2);
        lv_obj_clean(label3);
        lv_obj_clean(label4);
        lv_obj_clean(label5);
        lv_obj_clean(label6);
        lv_obj_clean(label7);
        lv_obj_clean(label8);
        label1 = NULL;
        label2 = NULL;
        label3 = NULL;
        label4 = NULL;
        label5 = NULL;
        label6 = NULL;
        label7 = NULL;
        label8 = NULL;
    }

}

void bilibili_gui_del(void) 
{
    bilibili_obj_del();

    //lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (NULL != bilibili_gui)
    {
        lv_obj_clean(bilibili_gui);
        bilibili_gui = NULL;
    }

    if (NULL != img_gui)
    {
        lv_obj_clean(img_gui);
        img_gui = NULL;
    }
    
    //lv_obj_clean(act_obj); // 清空此前页面
}

// void bilibili_display_init()
// {
//     lv_obj_t *act_obj = lv_scr_act();
//     if (act_obj == bilibili_gui)
//     {
//         return;
//     }
//     lv_obj_clean(act_obj);
// }