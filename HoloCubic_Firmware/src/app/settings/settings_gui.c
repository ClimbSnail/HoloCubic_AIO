#include "settings_gui.h"

#include "driver/lv_port_indev.h"
#include "lvgl.h"

lv_obj_t *settings_scr;

lv_obj_t *title_label;
lv_obj_t *cur_ver_label;
lv_obj_t *new_ver_label;
lv_obj_t *qq_label;
lv_obj_t *author_label;

static lv_style_t default_style;
static lv_style_t title_style;
static lv_style_t label_style;
static lv_style_t info_style;

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(lv_font_montserrat_30);

void settings_gui_init(void)
{
    settings_scr = NULL;
    title_label = NULL;
    cur_ver_label = NULL;
    new_ver_label = NULL;
    qq_label = NULL;
    author_label = NULL;

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));
    // 本地的ip地址
    settings_scr = lv_obj_create(NULL, NULL);
    lv_obj_add_style(settings_scr, LV_BTN_PART_MAIN, &default_style);

    lv_style_init(&title_style);
    lv_style_set_text_opa(&title_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&title_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&title_style, LV_STATE_DEFAULT, &lv_font_montserrat_30);

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&label_style, LV_STATE_DEFAULT, &lv_font_montserrat_24);

    lv_style_init(&info_style);
    lv_style_set_text_opa(&info_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&info_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&info_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);
}

/*
 * 其他函数请根据需要添加
 */

void display_settings_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == settings_scr)
        return;
    if (NULL != title_label)
    {
        lv_obj_clean(title_label);
        lv_obj_clean(cur_ver_label);
        lv_obj_clean(new_ver_label);
        lv_obj_clean(qq_label);
        lv_obj_clean(author_label);
    }
    lv_obj_clean(act_obj); // 清空此前页面
    title_label = lv_label_create(settings_scr, NULL);
    cur_ver_label = lv_label_create(settings_scr, NULL);
    new_ver_label = lv_label_create(settings_scr, NULL);
    qq_label = lv_label_create(settings_scr, NULL);
    author_label = lv_label_create(settings_scr, NULL);
}

void display_settings(const char *cur_ver, const char *new_ver, lv_scr_load_anim_t anim_type)
{
    display_settings_init();

    lv_obj_add_style(title_label, LV_LABEL_PART_MAIN, &title_style);
    lv_label_set_recolor(title_label, true); //先得使能文本重绘色功能
    lv_label_set_text(title_label, "#00ff00 All\tin\tone#");
    lv_obj_align(title_label, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, -220);

    lv_obj_add_style(cur_ver_label, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(cur_ver_label, true); //先得使能文本重绘色功能
    lv_label_set_text_fmt(cur_ver_label, "Now Version:  #ff0000 %s#", cur_ver);
    lv_obj_align(cur_ver_label, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -160);

    lv_obj_add_style(qq_label, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(qq_label, true); //先得使能文本重绘色功能
    lv_label_set_text(qq_label, "AIO QQ: 755143193");
    lv_obj_align(qq_label, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, -120);

    lv_obj_add_style(new_ver_label, LV_LABEL_PART_MAIN, &info_style);
    // 由于使用了 LV_LABEL_LONG_SROLL_CIRC 所以 lv_obj_set_size 是不生效的
    // lv_obj_set_size(new_ver_label, 100, 100);
    // LV_LABEL_LONG_SROLL_CIRC 模式一旦设置 宽度恒定等于当前文本的长度，所以下面先设置以下长度
    lv_label_set_text(new_ver_label, "Please update your A");
    lv_label_set_recolor(new_ver_label, true); //先得使能文本重绘色功能
    lv_label_set_long_mode(new_ver_label, LV_LABEL_LONG_SROLL_CIRC);
    if (strcmp(cur_ver, &new_ver[2]) < 0)
    {
        lv_label_set_text_fmt(new_ver_label, "Please update your AIO to #ff0000 %s#", new_ver);
        lv_obj_align(new_ver_label, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, -80);
    }

    lv_obj_add_style(author_label, LV_LABEL_PART_MAIN, &info_style);
    lv_label_set_recolor(author_label, true); //先得使能文本重绘色功能
    lv_label_set_text(author_label, "@ClimbSnail");
    lv_obj_align(author_label, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, -30);

    lv_scr_load(settings_scr);
}

void settings_gui_del(void)
{
    if (NULL != cur_ver_label)
    {
        lv_obj_clean(cur_ver_label);
        cur_ver_label = NULL;
    }
    if (NULL != new_ver_label)
    {
        lv_obj_clean(new_ver_label);
        new_ver_label = NULL;
    }
    if (NULL != settings_scr)
    {
        lv_obj_clean(settings_scr);
        settings_scr = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&title_style);
    // lv_style_reset(&label_style);
    // lv_style_reset(&info_style);
}