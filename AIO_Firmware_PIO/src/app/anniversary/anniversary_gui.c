#include "anniversary_gui.h"
#include "time.h"

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(msyhbd_18); // TODO
LV_FONT_DECLARE(msyhbd_22);
LV_FONT_DECLARE(msyhbd_24);

static lv_style_t default_style;
static lv_style_t chFont_style;
static lv_style_t numberBig_style;
static lv_style_t smallch_style;
static lv_style_t bigch_style;
static lv_style_t btn_style;

static lv_obj_t *anniversary_gui = NULL;
static lv_obj_t *txtLabel = NULL; // 文本提示信息
static lv_obj_t *dayLabel = NULL; // 在一起天数
static lv_obj_t *btn = NULL, *btnLabel = NULL;
static lv_obj_t *targetDateLabel = NULL; //目标日期

static const char weekDayCh[7][4] = {"日", "一", "二", "三", "四", "五", "六"};

void anniversary_gui_init(void)
{
    // if(NULL == default_style.map)
    // {

    // }

    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &msyhbd_22);

    lv_style_init(&bigch_style);
    lv_style_set_text_opa(&bigch_style, LV_OPA_COVER);
    lv_style_set_text_color(&bigch_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&bigch_style, &msyhbd_24);

    lv_style_init(&smallch_style);
    lv_style_set_text_opa(&smallch_style, LV_OPA_COVER);
    lv_style_set_text_color(&smallch_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&smallch_style, &msyhbd_18);

    lv_style_init(&numberBig_style);
    lv_style_set_text_opa(&numberBig_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBig_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&numberBig_style, &lv_font_ibmplex_115);

    // lv_style_reset(&btn_style);
    lv_style_init(&btn_style);
    lv_style_set_border_width(&btn_style, 0);
}

/*
 * 其他函数请根据需要添加
 */

void display_anniversary(const char *file_name, lv_scr_load_anim_t anim_type, struct tm *target_date, int anniversary_day_count, const char *event_name)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == anniversary_gui)
        return;

    anniversary_gui_del(); // 清空对象
    lv_obj_clean(act_obj); // 清空此前页面

    anniversary_gui = lv_obj_create(NULL);
    lv_obj_add_style(anniversary_gui, &default_style, LV_STATE_DEFAULT);

    /* 创建文本消息标签 */
    txtLabel = lv_label_create(anniversary_gui);
    lv_obj_add_style(txtLabel, &bigch_style, LV_STATE_DEFAULT);
    lv_label_set_text(txtLabel, event_name);

    /* 创建天数标签 */
    dayLabel = lv_label_create(anniversary_gui);
    lv_obj_add_style(dayLabel, &numberBig_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(dayLabel, true);
    lv_label_set_text_fmt(dayLabel, "#ffa500 %02d#", (anniversary_day_count < 0) ? (-anniversary_day_count + 1) : anniversary_day_count);

    /* 创建天了/天后btn */
    btn = lv_btn_create(anniversary_gui);
    lv_obj_add_style(btn, &btn_style, LV_STATE_DEFAULT);
    lv_obj_set_size(btn, 46, 26);
    lv_obj_set_style_bg_color(btn, lv_color_make(0xFF, 0xA5, 0x00), LV_STATE_DEFAULT);

    /* 创建天了/天后btnLable */
    btnLabel = lv_label_create(btn);
    lv_obj_add_style(btnLabel, &smallch_style, LV_STATE_DEFAULT);
    lv_label_set_text(btnLabel, (anniversary_day_count < 0) ? "天了" : "天");

    /* 创建目标日标签 */
    targetDateLabel = lv_label_create(anniversary_gui);
    lv_obj_add_style(targetDateLabel, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(targetDateLabel, true);
    if (target_date->tm_year != 0)
        lv_label_set_text_fmt(targetDateLabel, "#FFFFFF %d年%d月%d日#", target_date->tm_year, target_date->tm_mon, target_date->tm_mday);
    else
        lv_label_set_text_fmt(targetDateLabel, "#FFFFFF %d月%d日#", target_date->tm_mon, target_date->tm_mday);

    /* 创建设置对齐方式 */
    // lv_obj_align(anniversary_gui, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(txtLabel, LV_ALIGN_CENTER, 0, -70);

    lv_obj_align(dayLabel, LV_ALIGN_CENTER, -10, 20);
    lv_obj_align_to(btn, dayLabel, LV_ALIGN_OUT_RIGHT_TOP, 5, 5);
    // lv_obj_align(btn, LV_ALIGN_CENTER, 50 , -20);
    lv_obj_align(btnLabel, LV_ALIGN_CENTER, 0, -1);

    lv_obj_align(targetDateLabel, LV_ALIGN_CENTER, 0, 70);

    lv_scr_load(anniversary_gui);
}

void anniversary_gui_set_txtlabel(char *txt)
{
    lv_label_set_text(txtLabel, txt);
}

void anniversary_gui_display_date(struct tm *target, int anniversary_day_count, const char *event_name)
{
    lv_label_set_text(txtLabel, event_name);
    lv_label_set_text(btnLabel, (anniversary_day_count < 0) ? "天了" : "天");
    lv_label_set_text_fmt(dayLabel, "#ffa500 %02d#", (anniversary_day_count < 0) ? (-anniversary_day_count + 1) : anniversary_day_count);
    lv_obj_align(dayLabel, LV_ALIGN_CENTER, -10, 20);
    if (target->tm_year != 0)
        lv_label_set_text_fmt(targetDateLabel, "#FFFFFF %d年%d月%d日#", target->tm_year, target->tm_mon, target->tm_mday);
    else
        lv_label_set_text_fmt(targetDateLabel, "#FFFFFF %d月%d日#", target->tm_mon, target->tm_mday);
    lv_obj_align(targetDateLabel, LV_ALIGN_CENTER, 0, 70);
}

void anniversary_gui_del(void)
{
    if (NULL != anniversary_gui)
    {
        lv_obj_clean(anniversary_gui);
        anniversary_gui = NULL;
        btnLabel = NULL;
        txtLabel = NULL;
        dayLabel = NULL;
        btn = NULL;
        targetDateLabel = NULL;
    }

    // 手动清除样式，防止内存泄漏
    // lv_style_reset(&default_style);
}