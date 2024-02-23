#include "tomato_gui.h"
#include <stdio.h>
#include "driver/lv_port_indev.h"
#include "lvgl.h"
#define Frosted_Glass 1

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(lv_font_ibmplex_64);
LV_FONT_DECLARE(lv_font_ibmplex_200);
LV_FONT_DECLARE(tomato_chFont_20);

#define PIC_FILENAME_MAX_LEN 100

static lv_style_t *tomato_scr = NULL;
static lv_obj_t *clockLabel_1 = NULL, *clockLabel_2 = NULL;
static lv_style_t default_style;
static lv_style_t name_style;
static lv_style_t time_style;
static lv_style_t chFont_style;
static lv_obj_t *chart, *txtLabel;
static lv_style_t numberSmall_style;
static lv_style_t numberBig_style;
static lv_style_t btn_style;
static lv_obj_t *btn = NULL, *btn1 = NULL, *btn2 = NULL, *btn3 = NULL;
int second = 0, minute = 0;
void tomato_gui_init(void) // style init
{
    if (tomato_scr == lv_scr_act())
        return;
    tomato_scr = lv_obj_create(NULL);

    lv_style_init(&default_style); // 背景
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));
    lv_obj_add_style(tomato_scr, &default_style, LV_STATE_DEFAULT);

    lv_style_init(&chFont_style); // 汉字
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &tomato_chFont_20);

    lv_style_init(&numberSmall_style); // 数字 小
    lv_style_set_text_opa(&numberSmall_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberSmall_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberSmall_style, &lv_font_ibmplex_64);

    lv_style_init(&numberBig_style); // 数字 大
    lv_style_set_text_opa(&numberBig_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBig_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberBig_style, &lv_font_ibmplex_200);
}

void UIInit()
{
    if (tomato_scr == lv_scr_act())
        return; // 如果已经是当前页，退出
    else
        lv_obj_clean(lv_scr_act()); // 否则，清除页面
    if (tomato_scr == NULL)
    {
        tomato_scr = lv_obj_create(NULL);
    }
#ifdef Frosted_Glass
    btn = lv_btn_create(tomato_scr);
    lv_obj_add_style(btn, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 228, 224);
    lv_obj_set_pos(btn, 6, 12);
    lv_obj_set_style_bg_color(btn, lv_color_make(0xff, 0x00, 0x00), LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(btn, LV_OPA_40, LV_STATE_DEFAULT);
    btn1 = lv_btn_create(tomato_scr);
    lv_obj_add_style(btn1, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_size(btn1, 136, 41);
    lv_obj_set_pos(btn1, 24, 165);
    lv_obj_set_style_bg_color(btn1, lv_color_make(0xff, 0xff, 0xff), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn1, LV_OPA_20, LV_STATE_DEFAULT);

    btn2 = lv_btn_create(tomato_scr);
    lv_obj_add_style(btn2, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_size(btn2, 138, 110);
    lv_obj_set_pos(btn2, 24, 37);
    lv_obj_set_style_bg_color(btn2, lv_color_make(0xff, 0xff, 0xff), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn2, LV_OPA_20, LV_STATE_DEFAULT);

    btn3 = lv_btn_create(tomato_scr);
    lv_obj_add_style(btn3, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_size(btn3, 52, 56);
    lv_obj_set_pos(btn3, 164, 87);
    lv_obj_set_style_bg_color(btn3, lv_color_make(0xff, 0xff, 0xff), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn3, LV_OPA_20, LV_STATE_DEFAULT);
 #endif   
    txtLabel = lv_label_create(tomato_scr);
    lv_obj_add_style(txtLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(txtLabel, true);
    lv_label_set_text(txtLabel, "专注中");
    lv_obj_set_pos(txtLabel, 31, 165);

    clockLabel_1 = lv_label_create(tomato_scr); // 分钟
    lv_obj_add_style(clockLabel_1, &numberBig_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_1, true);
    lv_label_set_text_fmt(clockLabel_1, "#ffa500 %02d# ", 25);
    lv_obj_set_pos(clockLabel_1, 20, 15);
    clockLabel_2 = lv_label_create(tomato_scr);
    lv_obj_add_style(clockLabel_2, &numberSmall_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(clockLabel_2, true);
    lv_label_set_text_fmt(clockLabel_2, "%02d", 00);
    lv_obj_set_pos(clockLabel_2, 165, 90);
    lv_scr_load(tomato_scr);
}

void display_tomato(struct TimeStr t, int mode)
{
    UIInit();

    if (t.second == 60) // 如果秒是60，显示为1分00秒
    {
        second = 0;
        minute = t.minute + 1;
    }
    else
    {
        second = t.second;
        minute = t.minute;
    }
    lv_label_set_text_fmt(clockLabel_1, "#ffa500 %02d#", minute); // #ffa500
    lv_label_set_text_fmt(clockLabel_2, "%02d", second);
    if (mode == 0 || mode == 1)
    {
        if (t.second >= 0 && t.second <= 59)
        {
            char labelText[15];
            int dots = (60 - t.second - 1) % 5 + 1;
            snprintf(labelText, sizeof(labelText), "专注中%.*s", dots, ".....");
            lv_label_set_text(txtLabel, labelText);
        }
    }
    else
    {
        if (t.second >= 0 && t.second <= 59)
        {
            char labelText[15];
            int dots = (60 - t.second - 1) % 5 + 1;
            snprintf(labelText, sizeof(labelText), "休息中%.*s", dots, ".....");
            lv_label_set_text(txtLabel, labelText);
        }
    }
    if (second == 0 && minute == 0)
    {
        lv_label_set_text_fmt(txtLabel, "#F5F5F5 %s#", "时间到 !");
    }
}

void tomato_gui_del(void)
{
    if (NULL != tomato_scr)
    {
        lv_obj_clean(tomato_scr);
        tomato_scr = NULL;
    }
    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}
