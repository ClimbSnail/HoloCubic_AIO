#include "settings_gui.h"

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
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&title_style);
    lv_style_set_text_opa(&title_style, LV_OPA_COVER);
    lv_style_set_text_color(&title_style, lv_color_white());
    lv_style_set_text_font(&title_style, &lv_font_montserrat_30);

    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_OPA_COVER);
    lv_style_set_text_color(&label_style, lv_color_white());
    lv_style_set_text_font(&label_style, &lv_font_montserrat_24);

    lv_style_init(&info_style);
    lv_style_set_text_opa(&info_style, LV_OPA_COVER);
    lv_style_set_text_color(&info_style, lv_color_white());
    lv_style_set_text_font(&info_style, &lv_font_montserrat_20);
}

/*
 * 其他函数请根据需要添加
 */

void display_settings_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == settings_scr)
        return;

    lv_obj_clean(act_obj); // 清空此前页面

    // 本地的ip地址
    settings_scr = lv_obj_create(NULL);
    lv_obj_add_style(settings_scr, &default_style, LV_STATE_DEFAULT);

    title_label = lv_label_create(settings_scr);
    lv_obj_add_style(title_label, &title_style, LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_recolor(title_label, true); //先得使能文本重绘色功能

    cur_ver_label = lv_label_create(settings_scr);
    lv_obj_add_style(cur_ver_label, &label_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(cur_ver_label, true); //先得使能文本重绘色功能
    lv_obj_align(cur_ver_label, LV_ALIGN_BOTTOM_LEFT, 5, -130);

    qq_label = lv_label_create(settings_scr);
    lv_obj_add_style(qq_label, &label_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(qq_label, true); //先得使能文本重绘色功能
    lv_obj_align(qq_label, LV_ALIGN_BOTTOM_MID, 0, -90);

    new_ver_label = lv_label_create(settings_scr);
    lv_obj_add_style(new_ver_label, &info_style, LV_STATE_DEFAULT);
    lv_obj_set_size(new_ver_label, 220, 20);
    // lvgl8之前版本，由于使用了 LV_LABEL_LONG_SCROLL_CIRCULAR 所以 lv_obj_set_size 是不生效的
    // lvgl8之前版本，模式一旦设置 LV_LABEL_LONG_SCROLL_CIRCULAR
    // 宽度恒定等于当前文本的长度，所以下面先设置以下长度
    // lv_label_set_text(new_ver_label, "Please update your A");
    lv_label_set_recolor(new_ver_label, true); //先得使能文本重绘色功能
    lv_label_set_long_mode(new_ver_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(new_ver_label, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text_fmt(new_ver_label, "#ff0000 Is the latest version!");

    author_label = lv_label_create(settings_scr);
    lv_obj_add_style(author_label, &info_style, LV_STATE_DEFAULT);
    lv_obj_align(author_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_recolor(author_label, true); //先得使能文本重绘色功能

    lv_scr_load(settings_scr);
}

void display_settings(const char *cur_ver, const char *new_ver, lv_scr_load_anim_t anim_type)
{
    display_settings_init();

    lv_label_set_text(title_label, "#00ff00 All\tin\tone#");

    lv_label_set_text_fmt(cur_ver_label, "Now Version:  #ff0000 %s#", cur_ver);

    lv_label_set_text(qq_label, "AIO QQ: 755143193");

    if (strcmp(cur_ver, &new_ver[2]) < 0)
    {
        lv_label_set_text_fmt(new_ver_label, "Please update your AIO to #ff0000 %s#", new_ver);
        lv_obj_align(new_ver_label, LV_ALIGN_CENTER, 0, 60);
    }

    lv_label_set_text(author_label, "@ClimbSnail");
}

void settings_gui_del(void)
{
    if (NULL != settings_scr)
    {
        lv_obj_clean(settings_scr);
        settings_scr = NULL;
        cur_ver_label = NULL;
        new_ver_label = NULL;
        qq_label = NULL;
        author_label = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
    // lv_style_reset(&title_style);
    // lv_style_reset(&label_style);
    // lv_style_reset(&info_style);
}