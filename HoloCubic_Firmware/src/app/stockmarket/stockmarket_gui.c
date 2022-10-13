#include "stockmarket_gui.h"


#include "driver/lv_port_indev.h"
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
    if(NULL == default_style.map)
    {

    }
    lv_style_init(&default_style);
    lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
    lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

    lv_style_init(&numberBigRed_style);
    lv_style_set_text_opa(&numberBigRed_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&numberBigRed_style, LV_STATE_DEFAULT, lv_color_hex(0xff0000));
    lv_style_set_text_font(&numberBigRed_style, LV_STATE_DEFAULT, &lv_font_montserrat_48);

    lv_style_init(&numberBigGreen_style);
    lv_style_set_text_opa(&numberBigGreen_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&numberBigGreen_style, LV_STATE_DEFAULT, lv_color_hex(0x00ff00));
    lv_style_set_text_font(&numberBigGreen_style, LV_STATE_DEFAULT, &lv_font_montserrat_48);
    
    lv_style_init(&numberLittleRed_style);
    lv_style_set_text_opa(&numberLittleRed_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&numberLittleRed_style, LV_STATE_DEFAULT, lv_color_hex(0xff0000));
    lv_style_set_text_font(&numberLittleRed_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);

    
    lv_style_init(&numberLittleGreen_style);
    lv_style_set_text_opa(&numberLittleGreen_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&numberLittleGreen_style, LV_STATE_DEFAULT, lv_color_hex(0x00ff00));
    lv_style_set_text_font(&numberLittleGreen_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);
    
    lv_style_init(&chFont_style);
    lv_style_set_text_opa(&chFont_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&chFont_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_text_font(&chFont_style, LV_STATE_DEFAULT, &ch_font20);
    
    lv_style_init(&splitline_style);
    lv_style_set_text_opa(&splitline_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_text_color(&splitline_style, LV_STATE_DEFAULT, lv_color_hex(0x0000ff));
    lv_style_set_text_font(&splitline_style, LV_STATE_DEFAULT, &lv_font_montserrat_20);
}

void display_stockmarket_init(void)
{
    lv_obj_t *act_obj = lv_scr_act(); // 获取当前活动页
    if (act_obj == stockmarket_gui)
        return;

    stockmarket_obj_del();    // 清空对象
    lv_obj_clean(act_obj); // 清空此前页面

    stockmarket_gui = lv_obj_create(NULL, NULL);
    nowQuoLabel = lv_label_create(stockmarket_gui, NULL);
    ArrowImg = lv_img_create(stockmarket_gui, NULL);
    ChgValueLabel = lv_label_create(stockmarket_gui, NULL);
    ChgPercentLabel = lv_label_create(stockmarket_gui, NULL);
    NameLabel = lv_label_create(stockmarket_gui, NULL);
    uplineLabel = lv_label_create(stockmarket_gui, NULL);
    lineLabel2 = lv_label_create(stockmarket_gui, NULL);
    OpenQuo = lv_label_create(stockmarket_gui, NULL);
    MaxQuo = lv_label_create(stockmarket_gui, NULL);
    MinQuo = lv_label_create(stockmarket_gui, NULL);
    CloseQuo = lv_label_create(stockmarket_gui, NULL);
}

/*
 * 其他函数请根据需要添加
 */

void display_stockmarket(struct StockMarket stockInfo, lv_scr_load_anim_t anim_type)
{
    display_stockmarket_init();
    lv_obj_add_style(stockmarket_gui, LV_BTN_PART_MAIN, &default_style);

    //股票名称 NameLabel
    lv_obj_add_style(NameLabel, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_text_fmt(NameLabel,"股票代码:%s",stockInfo.code);
    
    if(stockInfo.updownflag == 0)   //股价下跌
    {
        //当前价
        lv_obj_add_style(nowQuoLabel, LV_LABEL_PART_MAIN, &numberBigGreen_style);
        lv_label_set_recolor(nowQuoLabel, true);
        lv_label_set_text_fmt(nowQuoLabel,"%.2f",stockInfo.NowQuo);
        //箭头
        lv_img_set_src(ArrowImg, &down);
        //涨跌幅
        lv_obj_add_style(ChgValueLabel, LV_LABEL_PART_MAIN, &numberLittleGreen_style);
        lv_label_set_text_fmt(ChgValueLabel,"%.2f",stockInfo.ChgValue);
        lv_obj_add_style(ChgPercentLabel, LV_LABEL_PART_MAIN, &numberLittleGreen_style);
        lv_label_set_text_fmt(ChgPercentLabel,"%.2f%%",stockInfo.ChgPercent);
    }
    else                                //股价上涨
    {        
        //当前价
        lv_obj_add_style(nowQuoLabel, LV_LABEL_PART_MAIN, &numberBigRed_style);
        lv_label_set_recolor(nowQuoLabel, true);
        lv_label_set_text_fmt(nowQuoLabel,"%.2f",stockInfo.NowQuo);
        //箭头
        lv_img_set_src(ArrowImg, &up);
        //涨跌幅
        lv_obj_add_style(ChgValueLabel, LV_LABEL_PART_MAIN, &numberLittleRed_style);
        lv_label_set_text_fmt(ChgValueLabel,"%.2f",stockInfo.ChgValue);
        lv_obj_add_style(ChgPercentLabel, LV_LABEL_PART_MAIN, &numberLittleRed_style);
        lv_label_set_text_fmt(ChgPercentLabel,"%.2f%%",stockInfo.ChgPercent);
    }

    //分隔符    
    lv_obj_add_style(uplineLabel, LV_LABEL_PART_MAIN, &splitline_style);
    lv_label_set_recolor(uplineLabel, true);
    lv_label_set_text_fmt(uplineLabel,"___________________________");
    lv_obj_add_style(lineLabel2, LV_LABEL_PART_MAIN, &splitline_style);
    lv_label_set_recolor(lineLabel2, true);
    lv_label_set_text_fmt(lineLabel2,"___________________________");

    //今开    最高
    lv_obj_add_style(OpenQuo, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(OpenQuo, true);
    lv_label_set_text_fmt(OpenQuo,"今   开:#ffa500 %0.2f#   最高:#ffa500 %0.2f#",stockInfo.OpenQuo,stockInfo.MaxQuo);
    //昨收    最低
    lv_obj_add_style(MaxQuo, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(MaxQuo, true);
    lv_label_set_text_fmt(MaxQuo,"昨   收:#ffa500 %0.2f#   最低:#ffa500 %0.2f#",stockInfo.CloseQuo,stockInfo.MinQuo);
    //成交量
    lv_obj_add_style(MinQuo, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(MinQuo, true);
    lv_label_set_text_fmt(MinQuo,"成交量:#ffa500 %0.2f#万手",stockInfo.tradvolume/1000000);
    //成交额
    lv_obj_add_style(CloseQuo, LV_LABEL_PART_MAIN, &chFont_style);
    lv_label_set_recolor(CloseQuo, true);
    lv_label_set_text_fmt(CloseQuo,"成交额:#ffa500 %0.2f#亿元",stockInfo.turnover/100000000);

    //绘制图形    
    lv_obj_align(NameLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_align(uplineLabel, NULL, LV_ALIGN_IN_LEFT_MID, 0, -70);
    lv_obj_align(nowQuoLabel, uplineLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(ArrowImg, nowQuoLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_align(ChgValueLabel, ArrowImg, LV_ALIGN_OUT_RIGHT_TOP, 10, -5);
    lv_obj_align(ChgPercentLabel, ChgValueLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_align(lineLabel2, nowQuoLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    lv_obj_align(OpenQuo, lineLabel2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(MaxQuo, OpenQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(MinQuo, MaxQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_align(CloseQuo, MinQuo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_scr_load(stockmarket_gui);
}

void stockmarket_obj_del(void)
{
    if (NULL != nowQuoLabel)
    {
        lv_obj_clean(nowQuoLabel);
        lv_obj_clean(ArrowImg);
        lv_obj_clean(ChgValueLabel);
        lv_obj_clean(ChgPercentLabel);
        lv_obj_clean(NameLabel);
        lv_obj_clean(uplineLabel);
        lv_obj_clean(lineLabel2);
        lv_obj_clean(OpenQuo);
        lv_obj_clean(MaxQuo);
        lv_obj_clean(MinQuo);
        lv_obj_clean(CloseQuo);

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
}

void stockmarket_gui_del(void)
{
    stockmarket_obj_del();
    if (NULL != stockmarket_gui)
    {
        lv_obj_clean(stockmarket_gui);
        stockmarket_gui = NULL;
    }

    // 手动清除样式，防止内存泄漏
    lv_style_reset(&default_style);
    lv_style_reset(&numberBigRed_style);
    lv_style_reset(&numberBigGreen_style);
    lv_style_reset(&numberLittleGreen_style);
    lv_style_reset(&numberLittleRed_style);
    lv_style_reset(&chFont_style);
    lv_style_reset(&splitline_style);
}