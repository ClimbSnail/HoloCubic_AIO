#include "bilibili_gui.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"

lv_obj_t *bilibili_gui = NULL;
lv_obj_t *follower_label = NULL;

lv_obj_t *fans_label;
lv_obj_t *follow_label;

static lv_style_t default_style;
static lv_style_t fans_label_style;
static lv_style_t follow_label_style;

LV_FONT_DECLARE(lv_font_montserrat_40);
LV_FONT_DECLARE(font_unicode_kai_16);

void bilibili_gui_init(void)
{
    bilibili_gui = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    bilibili_gui = lv_obj_create(NULL, NULL);
    lv_obj_add_style(bilibili_gui, LV_BTN_PART_MAIN, &default_style);

    lv_style_init(&fans_label_style);
    lv_style_set_text_opa(&fans_label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&fans_label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&fans_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_40);

    lv_style_init(&follow_label_style);
    lv_style_set_text_opa(&follow_label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&follow_label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&follow_label_style, LV_STATE_DEFAULT, &lv_font_montserrat_40);
}

void display_bilibili_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == bilibili_gui)
        return;
    if (NULL != fans_label)
    {
        lv_obj_clean(fans_label);
        lv_obj_clean(follow_label);
    }
    lv_obj_clean(act_obj); // 清空此前页面
    fans_label = lv_label_create(bilibili_gui, NULL);
    follow_label = lv_label_create(bilibili_gui, NULL);
}

/*
 * 其他函数请根据需要添加
 */

void display_bilibili(const char *file_name, lv_scr_load_anim_t anim_type, unsigned int fans_num, unsigned int follow_num)
{
    display_bilibili_init();

    lv_obj_add_style(fans_label, LV_LABEL_PART_MAIN, &fans_label_style);
    lv_label_set_recolor(fans_label, true);
    lv_label_set_text_fmt(fans_label, "%d", fans_num);
    lv_obj_align(fans_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 60, -90);

    lv_obj_add_style(follow_label, LV_LABEL_PART_MAIN, &fans_label_style);
    lv_label_set_recolor(follow_label, true);
    lv_label_set_text_fmt(follow_label, "%d", follow_num);
    lv_obj_align(follow_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 60, -30);

    LV_IMG_DECLARE(bilibili_logo_ico);
    lv_obj_t *logo = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(logo, &bilibili_logo_ico);
    lv_obj_align(logo, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
    lv_obj_set_size(logo, 200, 62);

    LV_IMG_DECLARE(avatar_ico);
    lv_obj_t *avatar = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(avatar, &avatar_ico);
    lv_obj_align(avatar, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 20, -30);
    lv_obj_set_size(avatar, 100, 100);

    lv_scr_load(bilibili_gui);
}

void bilibili_gui_del(void)
{
    if (NULL != bilibili_gui)
    {
        lv_obj_clean(bilibili_gui);
        bilibili_gui = NULL;
    }
}