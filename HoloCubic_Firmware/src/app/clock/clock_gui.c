#include "lv_port_indev.h"
#include "clock_gui.h"
#include "lvgl.h"

LV_FONT_DECLARE(myFont)
lv_obj_t *clock_text = NULL;

// static lv_style_t clock_gui;
lv_obj_t *clock_gui = NULL;
static lv_style_t default_style;
static lv_style_t label_style;
// static lv_style_t label_style2;

void clock_gui_init()
{
    // 初始化屏幕
    clock_gui = lv_obj_create(NULL, NULL);
    lv_obj_add_style(clock_gui, LV_BTN_PART_MAIN, &default_style);
    lv_scr_load(clock_gui);
    // 初始化默认样式
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    //label标签样式
    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &lv_font_montserrat_40);


    // 创建标签
    lv_obj_t *label = lv_label_create(clock_gui, NULL);
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(label, "hello world !");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -10); // 居中对齐
    lv_obj_set_size(label, 150, 150);
    // lv_obj_set_pos(label, 67, 169);
	lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    
    
    //创建标签

}

void clock_gui_start(lv_scr_load_anim_t anim_type)
{
    display_init();
}

void clock_gui_exit()
{
    if (NULL != clock_gui)
    {
        lv_obj_clean(clock_gui);
        clock_gui = NULL;
    }
}

void display_init()
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == clock_gui)
    {
        return;
    }
    lv_obj_clean(act_obj); //清空此前页面
}