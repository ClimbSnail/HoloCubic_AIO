#include "stockmarket_gui.h"

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_ibmplex_115);
LV_FONT_DECLARE(ch_font20);

static lv_obj_t *stockmarket_gui = NULL;

static lv_obj_t *nowQuoLabel = NULL;
static lv_obj_t *ArrowImg = NULL;
static lv_obj_t *ChgValueLabel = NULL;
static lv_obj_t *ChgPercentLabel = NULL;
static lv_obj_t *NameLabel = NULL;
static lv_obj_t *uplineLabel = NULL;
static lv_obj_t *lineLabel2 = NULL;
static lv_obj_t *OpenQuo = NULL;
static lv_obj_t *MaxQuo = NULL;
static lv_obj_t *MinQuo = NULL;
static lv_obj_t *CloseQuo = NULL;

static lv_style_t default_style;
static lv_style_t numberBigRed_style;
static lv_style_t numberBigGreen_style;
static lv_style_t numberLittleGreen_style;
static lv_style_t numberLittleRed_style;
static lv_style_t chFont_style;
static lv_style_t splitline_style;

// static lv_img_decoder_dsc_t img_dc_dsc; // 图片解码器

LV_FONT_DECLARE(lv_font_montserrat_40);
LV_IMG_DECLARE(bilibili_logo_ico);

LV_IMG_DECLARE(imgbtn_green);
LV_IMG_DECLARE(imgbtn_blue);

void stockmarket_gui_init(void)
{
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, lv_color_hex(0x000000));

    lv_style_init(&numberBigRed_style);
    lv_style_set_text_opa(&numberBigRed_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBigRed_style, lv_color_hex(0xff0000));
    lv_style_set_text_font(&numberBigRed_style, &lv_font_montserrat_48);

    lv_style_init(&numberBigGreen_style);
    lv_style_set_text_opa(&numberBigGreen_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberBigGreen_style, lv_color_hex(0x00ff00));
    lv_style_set_text_font(&numberBigGreen_style, &lv_font_montserrat_48);

    lv_style_init(&numberLittleRed_style);
    lv_style_set_text_opa(&numberLittleRed_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberLittleRed_style, lv_color_hex(0xff0000));
    lv_style_set_text_font(&numberLittleRed_style, &lv_font_montserrat_20);

    lv_style_init(&numberLittleGreen_style);
    lv_style_set_text_opa(&numberLittleGreen_style, LV_OPA_COVER);
    lv_style_set_text_color(&numberLittleGreen_style, lv_color_hex(0x00ff00));
    lv_style_set_text_font(&numberLittleGreen_style, &lv_font_montserrat_20);

    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&chFont_style, &ch_font20);

    lv_style_init(&splitline_style);
    lv_style_set_text_opa(&splitline_style, LV_OPA_COVER);
    lv_style_set_text_color(&splitline_style, lv_color_hex(0x0000ff));
    lv_style_set_text_font(&splitline_style, &lv_font_montserrat_20);
}

void display_stockmarket_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == stockmarket_gui)
        return;

    stockmarket_gui_del(); // 清空对象
    lv_obj_clean(act_obj); // 清空此前页面

    stockmarket_gui = lv_obj_create(NULL);
    lv_obj_add_style(stockmarket_gui, &default_style, LV_STATE_DEFAULT);

    nowQuoLabel = lv_label_create(stockmarket_gui);
    lv_label_set_recolor(nowQuoLabel, true);
    lv_label_set_text_fmt(nowQuoLabel, "0.00");

    ArrowImg = lv_img_create(stockmarket_gui);
    ChgValueLabel = lv_label_create(stockmarket_gui);

    ChgPercentLabel = lv_label_create(stockmarket_gui);

    NameLabel = lv_label_create(stockmarket_gui);
    lv_obj_add_style(NameLabel, &chFont_style, LV_STATE_DEFAULT);

    // Separator
    uplineLabel = lv_label_create(stockmarket_gui);
    lv_obj_add_style(uplineLabel, &splitline_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(uplineLabel, true);
    lv_label_set_text_fmt(uplineLabel, "___________________________");

    lineLabel2 = lv_label_create(stockmarket_gui);
    lv_obj_add_style(lineLabel2, &splitline_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(lineLabel2, true);
    lv_label_set_text_fmt(lineLabel2, "___________________________");

    OpenQuo = lv_label_create(stockmarket_gui);
    lv_obj_add_style(OpenQuo, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(OpenQuo, true);

    MaxQuo = lv_label_create(stockmarket_gui);
    lv_obj_add_style(MaxQuo, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(MaxQuo, true);

    MinQuo = lv_label_create(stockmarket_gui);
    lv_obj_add_style(MinQuo, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(MinQuo, true);

    CloseQuo = lv_label_create(stockmarket_gui);
    lv_obj_add_style(CloseQuo, &chFont_style, LV_STATE_DEFAULT);
    lv_label_set_recolor(CloseQuo, true);

    // Draw graphics
    lv_obj_align(NameLabel, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_align(uplineLabel, LV_ALIGN_LEFT_MID, 0, -90);
    lv_obj_align_to(nowQuoLabel, uplineLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align_to(ArrowImg, nowQuoLabel, LV_ALIGN_OUT_RIGHT_MID, 100, 0);
    lv_obj_align_to(ChgValueLabel, ArrowImg, LV_ALIGN_OUT_RIGHT_BOTTOM, 30, 10);
    lv_obj_align_to(ChgPercentLabel, ArrowImg, LV_ALIGN_OUT_RIGHT_TOP, 30, 20);
    lv_obj_align_to(lineLabel2, uplineLabel, LV_ALIGN_BOTTOM_MID, 0, 60);

    lv_obj_align_to(OpenQuo, lineLabel2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align_to(MaxQuo, OpenQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align_to(MinQuo, MaxQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align_to(CloseQuo, MinQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    lv_scr_load(stockmarket_gui);
}

/*
 * Add other functions as needed
 */

void display_stockmarket(struct StockMarket stockInfo, lv_scr_load_anim_t anim_type)
{
    display_stockmarket_init();

    // Stock name NameLabel
    lv_label_set_text_fmt(NameLabel, "Stock Code: %s", stockInfo.code);

    if (stockInfo.updownflag == 0) // Stock price down
    {
        // Current price
        lv_obj_add_style(nowQuoLabel, &numberBigGreen_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(nowQuoLabel, "%.2f", stockInfo.NowQuo);
        // Arrow
        lv_img_set_src(ArrowImg, &down);
        // Change value and percentage
        lv_obj_add_style(ChgValueLabel, &numberLittleGreen_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(ChgValueLabel, "%.2f", stockInfo.ChgValue);
        lv_obj_add_style(ChgPercentLabel, &numberLittleGreen_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(ChgPercentLabel, "%.2f%%", stockInfo.ChgPercent);
    }
    else // Stock price up
    {
        // Current price
        lv_obj_add_style(nowQuoLabel, &numberBigRed_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(nowQuoLabel, "%.2f", stockInfo.NowQuo);
        // Arrow
        lv_img_set_src(ArrowImg, &up);
        // Change value and percentage
        lv_obj_add_style(ChgValueLabel, &numberLittleRed_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(ChgValueLabel, "%.2f", stockInfo.ChgValue);
        lv_obj_add_style(ChgPercentLabel, &numberLittleRed_style, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(ChgPercentLabel, "%.2f%%", stockInfo.ChgPercent);
    }

    // Today's Open    High
    lv_label_set_text_fmt(OpenQuo, "Open:#ffa500 %0.2f#   High:#ffa500 %0.2f#",
                          stockInfo.OpenQuo, stockInfo.MaxQuo);
    // Yesterday's Close    Low
    lv_label_set_text_fmt(MaxQuo, "Close:#ffa500 %0.2f#   Low:#ffa500 %0.2f#",
                          stockInfo.CloseQuo, stockInfo.MinQuo);
    // Trading volume
    lv_label_set_text_fmt(MinQuo, "Volume:#ffa500 %0.2f#M shares",
                          stockInfo.tradvolume / 1000000);
    // Trading value
    lv_label_set_text_fmt(CloseQuo, "Value:#ffa500 %0.2f#B yuan",
                          stockInfo.turnover / 100000000);
}

void stockmarket_gui_del(void)
{
    if (NULL != stockmarket_gui)
    {
        lv_obj_clean(stockmarket_gui);
        stockmarket_gui = NULL;
        nowQuoLabel = NULL;
        ChgValueLabel = NULL;
        ChgPercentLabel = NULL;
        ArrowImg = NULL;
        NameLabel = NULL;
        uplineLabel = NULL;
        lineLabel2 = NULL;
        OpenQuo = NULL;
        MaxQuo = NULL;
        MinQuo = NULL;
        CloseQuo = NULL;
    }

    // Manually clear styles to prevent memory leaks
    // lv_style_reset(&default_style);
    // lv_style_reset(&numberBigRed_style);
    // lv_style_reset(&numberBigGreen_style);
    // lv_style_reset(&numberLittleGreen_style);
    // lv_style_reset(&numberLittleRed_style);
    // lv_style_reset(&chFont_style);
    // lv_style_reset(&splitline_style);
}